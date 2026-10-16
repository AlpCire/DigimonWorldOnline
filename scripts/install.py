#!/usr/bin/env python3
"""Audit/apply module migrations to a local AzerothCore laboratory. No client assets."""
import argparse
import hashlib
import json
import os
import re
from pathlib import Path

import pymysql
from pymysql.constants import CLIENT

ROOT = Path(__file__).resolve().parents[1]
EXPECTED = {900100: 'DigimonWelcome', 900101: 'DigimonWelcome',
            900102: 'DigimonCompanionAI', 900103: 'DigimonWildAI',
            900104: 'DigimonCompanionAI', 900105: 'DigimonCompanionAI',
            900106: 'DigimonCompanionAI', 900120: 'DigimonLabFollowerAI'}


def identifier(value):
    if not re.fullmatch(r'[A-Za-z0-9_]+', value):
        raise ValueError('Invalid SQL identifier')
    return '`' + value + '`'


def connect(database):
    password = os.environ.get('DIGIMON_DB_PASSWORD')
    if not password:
        raise RuntimeError('DIGIMON_DB_PASSWORD must be set; it is never logged.')
    return pymysql.connect(host=os.environ.get('DIGIMON_DB_HOST', 'ac-database'),
                           port=int(os.environ.get('DIGIMON_DB_PORT', '3306')),
                           user=os.environ.get('DIGIMON_DB_USER', 'root'), password=password,
                           database=database, charset='utf8mb4', autocommit=False,
                           cursorclass=pymysql.cursors.DictCursor,
                           client_flag=CLIENT.MULTI_STATEMENTS)


def insert_row(cursor, table, row):
    columns = ','.join(identifier(k) for k in row)
    placeholders = ','.join(['%s'] * len(row))
    cursor.execute(f'INSERT IGNORE INTO {identifier(table)} ({columns}) VALUES ({placeholders})', tuple(row.values()))


def audit(cursor):
    placeholders = ','.join(['%s'] * len(EXPECTED))
    cursor.execute(f'SELECT entry,ScriptName FROM creature_template WHERE entry IN ({placeholders})', tuple(EXPECTED))
    for row in cursor.fetchall():
        if row['ScriptName'] != EXPECTED[row['entry']]:
            raise RuntimeError(f"Creature ID collision: {row['entry']}. No existing template was changed.")
    cursor.execute('SELECT entry,ScriptName FROM item_template WHERE entry=900110')
    row = cursor.fetchone()
    if row and row['ScriptName'] != 'DigimonDevice':
        raise RuntimeError('Item ID collision: 900110. Choose IDs before installing.')
    cursor.execute('SELECT guid,id,Comment FROM creature WHERE guid BETWEEN 9000000 AND 9000002')
    spawn_entries = {9000000: 900100, 9000001: 900101, 9000002: 900103}
    for row in cursor.fetchall():
        if (not (row['Comment'] or '').startswith('mod-digimon-tamer:')
                or row['id'] != spawn_entries[row['guid']]):
            raise RuntimeError(f"Spawn GUID collision: {row['guid']}")
    cursor.execute('SELECT entry,name FROM creature_template WHERE entry IN (69,197,1128,2031,107)')
    references = cursor.fetchall()
    if len(references) != 5:
        raise RuntimeError('Expected stock wolf/NPC/bear/cat/raptor template is missing.')
    return references


def clone_template(cursor, source, destination, name, script, **changes):
    cursor.execute('SELECT * FROM creature_template WHERE entry=%s', (source,))
    row = cursor.fetchone()
    row.update(entry=destination, name=name, subname='WORLD DIGIMON MMO', ScriptName=script,
               AIName='', npcflag=0, lootid=0, skinloot=0, pickpocketloot=0,
               mingold=0, maxgold=0, difficulty_entry_1=0, difficulty_entry_2=0, difficulty_entry_3=0,
               KillCredit1=0, KillCredit2=0, VerifiedBuild=0)
    row.update(changes)
    insert_row(cursor, 'creature_template', row)
    cursor.execute('SELECT * FROM creature_template_model WHERE CreatureID=%s ORDER BY Idx LIMIT 1', (source,))
    model = cursor.fetchone()
    if not model:
        raise RuntimeError(f'No model for source creature {source}')
    model.update(CreatureID=destination, Idx=0, Probability=1, VerifiedBuild=0)
    insert_row(cursor, 'creature_template_model', model)


def apply_world(cursor):
    clone_template(cursor, 197, 900100, 'Guía del Mundo Digital', 'DigimonWelcome', npcflag=1, faction=35)
    clone_template(cursor, 197, 900101, 'Entrenador de técnicas', 'DigimonWelcome', npcflag=1, faction=35)
    clone_template(cursor, 69, 900102, 'Lobo digital', 'DigimonCompanionAI', faction=35, minlevel=1,
                   maxlevel=1, RegenHealth=0, ExperienceModifier=0, BaseAttackTime=1500)
    clone_template(cursor, 69, 900103, 'Lobo de datos salvaje', 'DigimonWildAI', faction=14, minlevel=1,
                   maxlevel=2, HealthModifier=3, ExperienceModifier=1)
    # Upgrade only this module's original laboratory value; preserve custom tuning.
    cursor.execute("UPDATE creature_template SET HealthModifier=3 WHERE entry=900103 AND ScriptName='DigimonWildAI' AND HealthModifier=1")
    for source, entry, name in ((1128, 900104, 'Oso digital'), (2031, 900105, 'Felino digital'),
                                (107, 900106, 'Rapaz digital')):
        clone_template(cursor, source, entry, name, 'DigimonCompanionAI', faction=35, minlevel=1,
                       maxlevel=1, RegenHealth=0, ExperienceModifier=0, BaseAttackTime=1500)
    clone_template(cursor, 69, 900120, 'Seguidor de ensayo', 'DigimonLabFollowerAI', faction=35,
                   minlevel=1, maxlevel=1, RegenHealth=0, ExperienceModifier=0, unit_flags=2)
    cursor.execute('SELECT * FROM item_template WHERE entry=6948')
    item = cursor.fetchone()
    item.update(entry=900110, name='Dispositivo Sagrado', description='Dirige y consulta tu colección digital.',
                ScriptName='DigimonDevice', maxcount=1, stackable=1, BuyPrice=0, SellPrice=0,
                spellid_1=18282, spelltrigger_1=0, spellcharges_1=0,
                spellcooldown_1=0, spellcategory_1=0, spellcategorycooldown_1=0)
    insert_row(cursor, 'item_template', item)
    # Upgrade only this module's inherited Hearthstone use. 18282 is a local
    # instant self-target dummy; DigimonDevice handles the use before casting.
    # Preserve deliberate administrator changes and never edit stock item 6948.
    cursor.execute("UPDATE item_template SET spellid_1=18282,spelltrigger_1=0,spellcharges_1=0,"
                   "spellcooldown_1=0,spellcategory_1=0,spellcategorycooldown_1=0 "
                   "WHERE entry=900110 AND ScriptName='DigimonDevice' AND spellid_1=8690")
    cursor.execute('SELECT map,position_x,position_y,position_z,orientation FROM playercreateinfo WHERE race=1 AND class=1')
    start = cursor.fetchone()
    if not start:
        raise RuntimeError('Human starting position is missing')
    for guid, entry, dx, dy in ((9000000, 900100, 3, 0), (9000001, 900101, -3, 0)):
        insert_row(cursor, 'creature', dict(guid=guid, id=entry, map=start['map'],
                   position_x=start['position_x'] + dx, position_y=start['position_y'] + dy,
                   position_z=start['position_z'], orientation=start['orientation'],
                   spawntimesecs=30, curhealth=100, Comment='mod-digimon-tamer: alpha03 laboratory'))
    # Use an existing stock wolf location with known ground height, at least
    # sixty metres from login. A hostile capture fixture must not camp new players.
    cursor.execute('SELECT map,position_x,position_y,position_z,orientation FROM creature '
                   'WHERE map=%s AND id IN (69,299) AND POW(position_x-%s,2)+POW(position_y-%s,2) '
                   'BETWEEN 3600 AND 40000 ORDER BY POW(position_x-%s,2)+POW(position_y-%s,2) LIMIT 1',
                   (start['map'], start['position_x'], start['position_y'], start['position_x'], start['position_y']))
    wild = cursor.fetchone()
    if not wild:
        raise RuntimeError('No verified stock wolf position 60–200 metres from the human start.')
    insert_row(cursor, 'creature', dict(guid=9000002, id=900103, **wild,
               spawntimesecs=30, curhealth=100, Comment='mod-digimon-tamer: alpha03 laboratory'))
    cursor.execute("UPDATE creature SET position_x=%s,position_y=%s,position_z=%s,orientation=%s "
                   "WHERE guid=9000002 AND id=900103 AND Comment='mod-digimon-tamer: alpha03 laboratory' "
                   "AND map=%s AND ABS(position_x-%s)<0.1 AND ABS(position_y-%s)<0.1",
                   (wild['position_x'], wild['position_y'], wild['position_z'], wild['orientation'],
                    start['map'], start['position_x']+12, start['position_y']))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--apply', action='store_true', help='Apply migrations after preflight; otherwise read only.')
    args = parser.parse_args()
    world_name = os.environ.get('DIGIMON_WORLD_DB', 'acore_world')
    chars_name = os.environ.get('DIGIMON_CHAR_DB', 'acore_characters')
    identifier(world_name)
    identifier(chars_name)
    world = connect(world_name)
    try:
        with world.cursor() as cursor:
            references = audit(cursor)
            if args.apply:
                apply_world(cursor)
        if args.apply:
            world.commit()
        else:
            world.rollback()
    except Exception:
        world.rollback()
        raise
    finally:
        world.close()
    migrations = sorted((ROOT / 'modules/mod-digimon-tamer/sql/characters').glob('*.sql'))
    if args.apply:
        chars = connect(chars_name)
        try:
            with chars.cursor() as cursor:
                for migration in migrations:
                    version = int(migration.name.split('_', 1)[0])
                    if version > 1:
                        cursor.execute('SELECT version FROM digimon_schema_version WHERE version=%s', (version,))
                        if cursor.fetchone():
                            continue
                    cursor.execute(migration.read_text(encoding='utf-8'))
                    while cursor.nextset():
                        pass
            chars.commit()
        finally:
            chars.close()
    print(json.dumps({'preflight': 'passed', 'applied': args.apply,
                      'references': references,
                      'migration_sha256': {m.name: hashlib.sha256(m.read_bytes()).hexdigest() for m in migrations},
                      'spawn_area': 'Northshire, human starting point',
                      'note': 'Client assets are neither copied nor distributed.'}, ensure_ascii=False))


if __name__ == '__main__':
    main()
