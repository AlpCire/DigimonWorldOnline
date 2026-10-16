#!/usr/bin/env python3
"""Export/validate/apply module configuration CSVs. Never imports client assets."""
import argparse
import csv
from datetime import datetime, timezone
import json
import os
from pathlib import Path
import struct

import pymysql

TABLES = {
    'digimon_species': ('id', 'creature_entry', 'name', 'starter_enabled', 'capture_entry', 'stage', 'enabled'),
    'digimon_aliases': ('name', 'species_id'),
    'digimon_techniques': ('id', 'name', 'spell_id', 'required_stage', 'price', 'enabled', 'special'),
    'digimon_species_techniques': ('species_id', 'technique_id'),
    'digimon_evolution_rules': ('source_species', 'target_species', 'required_level', 'enabled'),
}


def key(name):
    return ''.join(name.lower().split())


def read_csvs(folder):
    data = {}
    for table, columns in TABLES.items():
        with (folder / (table + '.csv')).open(encoding='utf-8-sig', newline='') as handle:
            reader = csv.DictReader(handle, delimiter=';')
            if reader.fieldnames != list(columns):
                raise ValueError('Unexpected columns: ' + table)
            data[table] = []
            for row in reader:
                if None in row or any(v is None for v in row.values()):
                    raise ValueError('Incomplete CSV row: ' + table)
                for column, value in row.items():
                    if column == 'name':
                        value = value.strip()
                        if not value or len(value) > 80 or len(value.encode('utf-8')) > 160 or any(ord(c) < 32 or c == '|' for c in value):
                            raise ValueError('Invalid name: ' + table)
                        row[column] = value
                    elif column == 'capture_entry' and not value.strip():
                        row[column] = None
                    else:
                        if not value.strip().isascii() or not value.strip().isdigit():
                            raise ValueError('Expected unsigned integer: ' + table + '.' + column)
                        row[column] = int(value)
                        if row[column] > (1000000000 if column == 'price' else 4294967295):
                            raise ValueError('Integer outside supported range')
                data[table].append(row)
    return data


def unique(rows, columns):
    values = [tuple(row[c] for c in columns) for row in rows]
    if len(set(values)) != len(values):
        raise ValueError('Duplicate keys: ' + str(columns))


def validate(data, cursor, dbc):
    species = {r['id']: r for r in data['digimon_species']}
    techniques = {r['id']: r for r in data['digimon_techniques']}
    for table, rows in data.items():
        unique(rows, TABLES[table][:2] if table == 'digimon_species_techniques' else TABLES[table][:1])
        for row in rows:
            for flag in ('enabled', 'starter_enabled', 'special'):
                if flag in row and row[flag] not in (0, 1):
                    raise ValueError('Invalid flag: ' + table + '.' + flag)
    unique(data['digimon_species'], ('creature_entry',))
    if len({key(r['name']) for r in species.values()}) != len(species):
        raise ValueError('Ambiguous species names')
    for row in species.values():
        if not row['id'] or not row['creature_entry'] or row['stage'] not in range(1, 5):
            raise ValueError('Invalid species')
        if row['enabled']:
            cursor.execute('SELECT ScriptName FROM acore_world.creature_template WHERE entry=%s', row['creature_entry'])
            template = cursor.fetchone()
            if not template or template['ScriptName'] != 'DigimonCompanionAI':
                raise ValueError('Install the companion template before enabling ' + row['name'])
    known_names = {key(r['name']): r['id'] for r in species.values()}
    for row in data['digimon_aliases']:
        alias = key(row['name'])
        if not alias or row['species_id'] not in species or known_names.get(alias, row['species_id']) != row['species_id']:
            raise ValueError('Invalid/ambiguous alias')
        known_names[alias] = row['species_id']
    spells = dbc.read_bytes()
    magic, count, fields, size, strings = struct.unpack_from('<4s4I', spells)
    if magic != b'WDBC' or size < 4 or len(spells) != 20 + count * size + strings:
        raise ValueError('Invalid Spell.dbc')
    spell_ids = {struct.unpack_from('<I', spells, 20 + i * size)[0] for i in range(count)}
    for row in techniques.values():
        if not row['id'] or row['required_stage'] not in range(1, 5) or (row['enabled'] and row['spell_id'] not in spell_ids):
            raise ValueError('Invalid technique/spell: ' + row['name'])
    for row in data['digimon_species_techniques']:
        if row['species_id'] not in species or row['technique_id'] not in techniques:
            raise ValueError('Technique assignment references a missing entry')
    for row in data['digimon_evolution_rules']:
        source, target = species.get(row['source_species']), species.get(row['target_species'])
        if not source or not target or source is target or row['required_level'] not in range(2, 81):
            raise ValueError('Invalid evolution')
        if target['stage'] <= source['stage']:
            raise ValueError('Evolution must increase stage; cycles are not permitted')
        if row['enabled'] and not (source['enabled'] and target['enabled']):
            raise ValueError('Install/enable both species before enabling evolution')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('mode', choices=('export', 'validate', 'apply'))
    parser.add_argument('directory', type=Path)
    parser.add_argument('--spell-dbc', type=Path, default=Path('/client/dbc/Spell.dbc'))
    args = parser.parse_args()
    db = pymysql.connect(host=os.environ.get('DIGIMON_DB_HOST', 'ac-database'), user='root',
                         password=os.environ['DIGIMON_DB_PASSWORD'], database='acore_characters',
                         charset='utf8mb4', cursorclass=pymysql.cursors.DictCursor)
    try:
        with db.cursor() as cursor:
            before = {}
            for table, columns in TABLES.items():
                cursor.execute('SELECT ' + ','.join(columns) + ' FROM ' + table)
                before[table] = cursor.fetchall()
            if args.mode == 'export':
                args.directory.mkdir(parents=True, exist_ok=True)
                for table, columns in TABLES.items():
                    path = args.directory / (table + '.csv')
                    # Never overwrite the user's edits when exporting again.
                    with path.open('x', encoding='utf-8-sig', newline='') as handle:
                        writer = csv.DictWriter(handle, fieldnames=columns, delimiter=';')
                        writer.writeheader()
                        writer.writerows(before[table])
            else:
                data = read_csvs(args.directory)
                validate(data, cursor, args.spell_dbc)
                if args.mode == 'apply':
                    cursor.execute('SELECT COUNT(*) AS total FROM characters WHERE online=1')
                    if cursor.fetchone()['total']:
                        raise ValueError('Players are connected; stop the world first')
                    # No catalog row may disappear: collections retain their references.
                    for table in ('digimon_species', 'digimon_techniques'):
                        if not {r['id'] for r in before[table]} <= {r['id'] for r in data[table]}:
                            raise ValueError('Keep existing IDs; disable instead of removing')
                    backup = args.directory / ('backup-' + datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ') + '.json')
                    with backup.open('x', encoding='utf-8') as handle:
                        json.dump(before, handle, ensure_ascii=False, indent=2)
                    for table in ('digimon_aliases', 'digimon_species_techniques', 'digimon_evolution_rules'):
                        cursor.execute('DELETE FROM ' + table)
                    for table, columns in TABLES.items():
                        marks = ','.join(['%s'] * len(columns))
                        updates = ','.join(c + '=VALUES(' + c + ')' for c in columns[1:])
                        sql = 'INSERT INTO ' + table + ' (' + ','.join(columns) + ') VALUES (' + marks + ') ON DUPLICATE KEY UPDATE ' + updates
                        for row in data[table]:
                            cursor.execute(sql, tuple(row[c] for c in columns))
                    db.commit()
            print('PASS: catalog ' + args.mode + '; player progression was not edited')
    finally:
        db.close()


if __name__ == '__main__':
    main()
