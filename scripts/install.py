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
            900106: 'DigimonCompanionAI', 900120: 'DigimonLabFollowerAI', 900121: 'DigimonPreviewAI',
            900163: 'DigimonQuestNPC', 900164: 'DigimonQuestNPC', 900165: 'DigimonQuestNPC'}


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


def split_sql_statements(sql):
    """Split SQL at semicolons outside quoted strings and identifiers."""
    statements, start, quote, i = [], 0, None, 0
    while i < len(sql):
        char = sql[i]
        if quote:
            if char == quote:
                if quote == "'" and i + 1 < len(sql) and sql[i + 1] == "'":
                    i += 2
                    continue
                if i == 0 or sql[i - 1] != "\\":
                    quote = None
        elif char in "'\"`":
            quote = char
        elif char == ';':
            statement = sql[start:i].strip()
            if statement:
                statements.append(statement)
            start = i + 1
        i += 1
    tail = sql[start:].strip()
    if tail:
        statements.append(tail)
    return statements


def apply_migration(cursor, migration):
    sql = migration.read_text(encoding='utf-8')
    if migration.name == '023_roles_talents.sql':
        cursor.execute('SHOW COLUMNS FROM digimon_collection')
        columns = {row['Field'] for row in cursor.fetchall()}
        alter, separator, rest = sql.partition(';')
        if not separator or 'ALTER TABLE `digimon_collection`' not in alter:
            raise RuntimeError('Unexpected version 23 migration layout')
        if 'combat_role' not in columns:
            cursor.execute(alter)
        rest = '\n'.join(line for line in rest.splitlines()
                         if not line.lstrip().startswith('--'))
        for statement in split_sql_statements(rest):
            cursor.execute(statement)
        return
    if migration.name == '022_digialmacen_party_capture.sql':
        cursor.execute('SHOW COLUMNS FROM digimon_tamer')
        columns = {row['Field'] for row in cursor.fetchall()}
        if 'storage_boxes' not in columns:
            cursor.execute('ALTER TABLE digimon_tamer ADD COLUMN storage_boxes SMALLINT UNSIGNED NOT NULL DEFAULT 2 AFTER collection_capacity')
        table_sql, separator, rest = sql.partition(';')
        if not separator:
            raise RuntimeError('Unexpected version 22 migration layout')
        rest = '\n'.join(line for line in rest.splitlines()
                         if not line.lstrip().startswith('--'))
        for statement in split_sql_statements(rest):
            cursor.execute(statement)
        return
    if migration.name == '010_impmon_attack_catalog.sql':
        # Ignore SQL header comments before finding the ALTER terminator;
        # comments may contain semicolons and must not split the migration.
        sql = '\n'.join(line for line in sql.splitlines()
                          if not line.lstrip().startswith('--'))
        alter, separator, rest = sql.partition(';')
        if not separator or 'ALTER TABLE `digimon_techniques`' not in alter:
            raise RuntimeError('Unexpected version 10 migration layout')
        cursor.execute('SHOW COLUMNS FROM digimon_techniques')
        columns = {row['Field'] for row in cursor.fetchall()}
        additions = [
            ('localized_name', "VARCHAR(80) NOT NULL DEFAULT '' AFTER `name`"),
            ('category', "ENUM('OFFICIAL','GAME_REFERENCE','MMO_CUSTOM') NOT NULL DEFAULT 'GAME_REFERENCE' AFTER `special`"),
            ('min_level', 'TINYINT UNSIGNED NOT NULL DEFAULT 1 AFTER `category`'),
            ('cooldown_override_ms', 'INT UNSIGNED NOT NULL DEFAULT 0 AFTER `min_level`'),
            ('damage_type', "ENUM('PHYSICAL','FIRE','SHADOW','NATURE','ARCANE') NOT NULL DEFAULT 'PHYSICAL' AFTER `cooldown_override_ms`"),
            ('scaling_model', "VARCHAR(48) NOT NULL DEFAULT 'SPELL_NATIVE_LEVEL_AND_STATS' AFTER `damage_type`"),
            ('effect_note', "VARCHAR(255) NOT NULL DEFAULT '' AFTER `scaling_model`"),
            ('control_limited', 'TINYINT UNSIGNED NOT NULL DEFAULT 0 AFTER `effect_note`'),
        ]
        for name, definition in additions:
            if name not in columns:
                cursor.execute(f'ALTER TABLE digimon_techniques ADD COLUMN `{name}` {definition}')
        for statement in split_sql_statements(rest):
            cursor.execute(statement)
    else:
        cursor.execute(sql)
        while cursor.nextset():
            pass
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
    cursor.execute('SELECT entry,ScriptName FROM item_template WHERE entry IN (900110,900210,900211,900212,900213,900230,900231,900232,900233)')
    for row in cursor.fetchall():
        if row['entry'] == 900110 and row['ScriptName'] != 'DigimonDevice':
            raise RuntimeError('Item ID collision: 900110. Choose IDs before installing.')
        if row['entry'] in (900230, 900231, 900232, 900233) and row['ScriptName'] != 'DigimonConsumable':
            raise RuntimeError(f"Item ID collision: {row['entry']}. Existing row was not changed.")
    consumable_names = {900230: 'Galleta de Datos', 900231: 'Ración de Filo',
                        900232: 'Estofado de Núcleo', 900233: 'Poción de Impulso Digital'}
    cursor.execute('SELECT entry,name,ScriptName FROM item_template WHERE entry BETWEEN 900230 AND 900233')
    for row in cursor.fetchall():
        if row['name'] != consumable_names[row['entry']] or row['ScriptName'] != 'DigimonConsumable':
            raise RuntimeError(f"Consumable ID collision: {row['entry']}. Existing row was not changed.")
    cursor.execute('SELECT entry,name,ScriptName FROM item_template WHERE entry=900240')
    expansion = cursor.fetchone()
    if expansion and (expansion['name'] != 'Núcleo de expansión' or expansion['ScriptName']):
        raise RuntimeError('Item ID collision: 900240. Existing row was not changed.')
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
    cursor.execute('SELECT entry FROM item_template WHERE entry IN (4536,2459)')
    stock_items = {row['entry'] for row in cursor.fetchall()}
    if stock_items != {4536, 2459}:
        raise RuntimeError('Expected client-known food and Swiftness Potion item templates are missing.')
    for table, query, params, expected_names in (
        ('creature', 'SELECT entry,name FROM creature_template WHERE entry BETWEEN 900150 AND 900162', (),
         {900150: 'Pagumon desincronizado', 900151: 'Pagumon nodal', 900160: 'Patamon',
          900161: 'Impmon', 900162: 'Wizardmon'}),
        ('item', 'SELECT entry,name FROM item_template WHERE entry BETWEEN 900220 AND 900222', (),
         {900220: 'Fragmento de Datos de Pagumon', 900221: 'Núcleo corrupto', 900222: 'Informe cifrado de Wizardmon'}),
    ):
        cursor.execute(query, params)
        for row in cursor.fetchall():
            if row['entry'] not in expected_names or row['name'] != expected_names[row['entry']]:
                raise RuntimeError(f"{table.title()} ID collision: {row['entry']}. Existing row was not changed.")
    cursor.execute('SELECT ID,LogTitle FROM quest_template WHERE ID BETWEEN 900601 AND 900605')
    quest_names = {900601: 'Señales entre los pinos', 900602: 'Datos de Pagumon',
                   900603: 'Rumores en Villadorada', 900604: 'El núcleo de la distorsión',
                   900605: 'Informe para el Guía'}
    for row in cursor.fetchall():
        if row['LogTitle'] != quest_names[row['ID']]:
            raise RuntimeError(f"Quest ID collision: {row['ID']}. Existing row was not changed.")
    cursor.execute('SELECT ID,LogTitle FROM quest_template WHERE ID BETWEEN 900701 AND 900704')
    arrival_names = {900701: 'Despertar en el Mundo Digital', 900702: 'Recuerdos fragmentados',
                     900703: 'Una voz en Villadorada', 900704: 'El primer vínculo'}
    for row in cursor.fetchall():
        if row['LogTitle'] != arrival_names[row['ID']]:
            raise RuntimeError(f"Quest ID collision: {row['ID']}. Existing row was not changed.")
    cursor.execute('SELECT ID,LogTitle FROM quest_template WHERE ID BETWEEN 900801 AND 900804')
    pagumon_names = {900801: 'Una señal propia', 900802: 'Datos dispersos',
                     900803: 'La grieta nodal', 900804: 'Un vínculo elegido'}
    for row in cursor.fetchall():
        if row['LogTitle'] != pagumon_names[row['ID']]:
            raise RuntimeError(f"Quest ID collision: {row['ID']}. Existing row was not changed.")
    cursor.execute('SELECT guid,id,Comment FROM creature WHERE guid BETWEEN 9000010 AND 9000016')
    spawn_entries = {9000010: 900160, 9000011: 900161, 9000012: 900162,
                     9000013: 900150, 9000014: 900150, 9000015: 900150, 9000016: 900151}
    for row in cursor.fetchall():
        if not (row['Comment'] or '').startswith('mod-digimon-tamer: human-story') or row['id'] != spawn_entries[row['guid']]:
            raise RuntimeError(f"Spawn GUID collision: {row['guid']}")
    cursor.execute('SELECT guid,id,Comment FROM creature WHERE guid BETWEEN 9000017 AND 9000019')
    arrival_spawns = {9000017: 900163, 9000018: 900164, 9000019: 900165}
    for row in cursor.fetchall():
        if not (row['Comment'] or '').startswith('mod-digimon-tamer: arrival-story') or row['id'] != arrival_spawns[row['guid']]:
            raise RuntimeError(f"Spawn GUID collision: {row['guid']}")
    cursor.execute('SELECT entry,name,AIName,ScriptName FROM creature_template '
                   'WHERE entry IN (900170,900171,900174)')
    forest_templates = {
        900170: ('Patamon desfasado', '', 'DigimonWildAI'),
        900171: ('Pagumon desfasado', '', 'DigimonWildAI'),
        900174: ('Eco sombrío de Devimon', 'SmartAI', ''),
    }
    for row in cursor.fetchall():
        actual = (row['name'], row['AIName'], row['ScriptName'])
        if actual != forest_templates[row['entry']]:
            raise RuntimeError(f"Forest creature ID collision: {row['entry']}")
    cursor.execute('SELECT ID,LogTitle FROM quest_template WHERE ID BETWEEN 901001 AND 901005')
    forest_quests = {901001: 'Una señal en el Vivero', 901002: 'Fragmentos entre las piedras',
                     901003: 'Alas fuera de fase', 901004: 'El guardián del Eco',
                     901005: 'El bosque recuerda'}
    for row in cursor.fetchall():
        if row['LogTitle'] != forest_quests[row['ID']]:
            raise RuntimeError(f"Forest quest ID collision: {row['ID']}")
    cursor.execute('SELECT guid,id,map,Comment FROM creature WHERE guid BETWEEN 9000030 AND 9000039')
    forest_spawns = {9000030: 900100, 9000031: 900160, 9000032: 900161,
                     9000033: 900162, 9000034: 900170, 9000035: 900170,
                     9000036: 900171, 9000037: 900171, 9000038: 900171,
                     9000039: 900174}
    for row in cursor.fetchall():
        if (row['id'] != forest_spawns[row['guid']] or row['map'] != 1001 or
                not (row['Comment'] or '').startswith('mod-digimon-tamer: emerald-first-adventure')):
            raise RuntimeError(f"Forest spawn GUID collision: {row['guid']}")
    cursor.execute('SELECT entry,name FROM creature_template WHERE entry IN (900172,900173,900175)')
    second_creatures = {900172: 'Impmon alterado', 900173: 'Reflejo de Wizardmon',
                        900175: 'Archivista de Villaraíz'}
    for row in cursor.fetchall():
        if row['name'] != second_creatures[row['entry']]:
            raise RuntimeError(f"Second chapter creature ID collision: {row['entry']}")
    cursor.execute('SELECT entry,name FROM item_template WHERE entry IN (900250,900251)')
    second_items = {900250: 'Rastro de datos alterados', 900251: 'Glifo del Eco'}
    for row in cursor.fetchall():
        if row['name'] != second_items[row['entry']]:
            raise RuntimeError(f"Second chapter item ID collision: {row['entry']}")
    cursor.execute('SELECT ID,LogTitle FROM quest_template WHERE ID BETWEEN 901006 AND 901010')
    second_quests = {901006: 'Otra voz bajo el árbol', 901007: 'Datos que no encajan',
                     901008: 'Los glifos del Círculo', 901009: 'El archivo de Villaraíz',
                     901010: 'Raíces de una memoria'}
    for row in cursor.fetchall():
        if row['LogTitle'] != second_quests[row['ID']]:
            raise RuntimeError(f"Second chapter quest ID collision: {row['ID']}")
    cursor.execute('SELECT guid,id,map,Comment FROM creature WHERE guid BETWEEN 9000043 AND 9000048')
    second_spawns = {9000043: 900175, 9000044: 900172, 9000045: 900172,
                     9000046: 900172, 9000047: 900173, 9000048: 900173}
    for row in cursor.fetchall():
        if (row['id'] != second_spawns[row['guid']] or row['map'] != 1001 or
                not (row['Comment'] or '').startswith('mod-digimon-tamer: emerald-second-chapter')):
            raise RuntimeError(f"Second chapter spawn GUID collision: {row['guid']}")
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
    clone_template(cursor, 197, 900100, 'Guía del Mundo Digital', 'DigimonWelcome', npcflag=3, faction=35)
    # Existing alpha installs created the guide with gossip only; add the quest-giver flag safely.
    cursor.execute("UPDATE creature_template SET npcflag=(npcflag | 2) "
                   "WHERE entry=900100 AND ScriptName='DigimonWelcome'")
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
    clone_template(cursor, 69, 900121, 'Digimon de prueba temporal', 'DigimonPreviewAI', faction=14,
                   minlevel=1, maxlevel=1, RegenHealth=0, ExperienceModifier=0)
    cursor.execute('SELECT * FROM item_template WHERE entry=6948')
    item = cursor.fetchone()
    item.update(entry=900110, name='Dispositivo Sagrado', description='Dirige y consulta tu colección digital.',
                ScriptName='DigimonDevice', maxcount=1, stackable=1, BuyPrice=0, SellPrice=0,
                spellid_1=18282, spelltrigger_1=0, spellcharges_1=0,
                spellcooldown_1=0, spellcategory_1=0, spellcategorycooldown_1=0)
    insert_row(cursor, 'item_template', item)
    for source, entry, name, description in (
            (23436, 900210, 'Nucleo Arcano', 'Catalizador de una rama de magia.'),
            (23436, 900211, 'Nucleo Oscuro', 'Catalizador de una rama de oscuridad.'),
            (23436, 900212, 'Nucleo Espiritual', 'Catalizador de una rama de espiritu.'),
            (23436, 900213, 'Nucleo de Brujeria', 'Catalizador de una rama de brujeria.')):
        cursor.execute('SELECT * FROM item_template WHERE entry=%s', (source,))
        catalyst = cursor.fetchone()
        if not catalyst:
            raise RuntimeError(f'Catalyst source item {source} is missing')
        catalyst.update(entry=entry, name=name, description=description, ScriptName='',
                        maxcount=0, stackable=20, BuyPrice=0, SellPrice=0,
                        spellid_1=0, spelltrigger_1=0, spellcharges_1=0,
                        spellcooldown_1=0, spellcategory_1=0, spellcategorycooldown_1=0)
        insert_row(cursor, 'item_template', catalyst)
    # Small quest-chain inventory objects, reusing client-known crystal icons.
    for source, entry, name, description in (
            (22449, 900220, 'Fragmento de Datos de Pagumon', 'Datos dañados recuperados de un Pagumon desincronizado.'),
            (22450, 900221, 'Núcleo corrupto', 'El núcleo que mantiene activa la distorsión digital.'),
            (22448, 900222, 'Informe cifrado de Wizardmon', 'Una nota con la lectura del núcleo y la ruta de la señal.')):
        cursor.execute('SELECT * FROM item_template WHERE entry=%s', (source,))
        item = cursor.fetchone()
        if not item:
            raise RuntimeError(f'Quest item source {source} is missing')
        item.update(entry=entry, name=name, description=description, ScriptName='', maxcount=20,
                    stackable=20, BuyPrice=0, SellPrice=0, spellid_1=0, spelltrigger_1=0,
                    spellcharges_1=0, spellcooldown_1=0, spellcategory_1=0, spellcategorycooldown_1=0)
        insert_row(cursor, 'item_template', item)
    # Companion food and care items reuse client-known food icons. Each is
    # non-stackable so the item GUID ledger can recover safely from a crash.
    for entry, name, description in (
            (900230, 'Galleta de Datos', 'Aumenta la felicidad de tu Digimon en 15.'),
            (900231, 'Ración de Filo', 'Aumenta la felicidad en 5 y el daño del Digimon un 15% durante 10 minutos.'),
            (900232, 'Estofado de Núcleo', 'Aumenta la felicidad en 5 y reduce el daño recibido un 15% durante 10 minutos.')):
        cursor.execute('SELECT * FROM item_template WHERE entry=4536')
        item = cursor.fetchone()
        if not item:
            raise RuntimeError('Stock food source item 4536 is missing')
        item.update(entry=entry, name=name, description=description, ScriptName='DigimonConsumable',
                    maxcount=0, stackable=1, BuyPrice=0, SellPrice=0,
                    spellid_1=0, spelltrigger_1=0, spellcharges_1=0,
                    spellcooldown_1=0, spellcategory_1=0, spellcategorycooldown_1=0)
        insert_row(cursor, 'item_template', item)
    cursor.execute('SELECT * FROM item_template WHERE entry=2459')
    speed = cursor.fetchone()
    if not speed:
        raise RuntimeError('Stock Swiftness Potion source item 2459 is missing')
    speed.update(entry=900233, name='Poción de Impulso Digital',
                 description='Aumenta la velocidad de movimiento durante 15 segundos.',
                 ScriptName='DigimonConsumable', maxcount=0, stackable=1, BuyPrice=0, SellPrice=0,
                 spellid_1=2379, spelltrigger_1=0, spellcharges_1=1,
                 spellcooldown_1=0, spellcategory_1=0, spellcategorycooldown_1=0)
    insert_row(cursor, 'item_template', speed)
    cursor.execute('SELECT * FROM item_template WHERE entry=22449')
    core = cursor.fetchone()
    if not core:
        raise RuntimeError('Digital expansion core source item 22449 is missing')
    core.update(entry=900240, name='Núcleo de expansión',
                description='Un estabilizador de datos necesario para ampliar el Digialmacén.',
                ScriptName='', maxcount=20, stackable=20, BuyPrice=0, SellPrice=0,
                spellid_1=0, spelltrigger_1=0, spellcharges_1=0,
                spellcooldown_1=0, spellcategory_1=0, spellcategorycooldown_1=0)
    insert_row(cursor, 'item_template', core)
    for source, entry, name, description in (
            (22449, 900250, 'Rastro de datos alterados',
             'Un fragmento perdido por un Impmon alterado.'),
            (22450, 900251, 'Glifo del Eco',
             'Una marca que los reflejos de Wizardmon protegen.')):
        cursor.execute('SELECT * FROM item_template WHERE entry=%s', (source,))
        quest_item = cursor.fetchone()
        if not quest_item:
            raise RuntimeError(f'Second chapter source item {source} is missing')
        quest_item.update(entry=entry, name=name, description=description,
                          ScriptName='', maxcount=20, stackable=20,
                          BuyPrice=0, SellPrice=0, spellid_1=0,
                          spelltrigger_1=0, spellcharges_1=0,
                          spellcooldown_1=0, spellcategory_1=0,
                          spellcategorycooldown_1=0)
        insert_row(cursor, 'item_template', quest_item)
    for entry, name, price in ((900230, 'Galleta de Datos', 300),
                               (900231, 'Ración de Filo', 1000),
                               (900232, 'Estofado de Núcleo', 1000),
                               (900233, 'Poción de Impulso Digital', 2500)):
        cursor.execute('UPDATE item_template SET BuyPrice=%s '
                       'WHERE entry=%s AND name=%s AND BuyPrice=0',
                       (price, entry, name))
    # Quest characters use the installed custom Digimon models but remain ordinary,
    # non-capturable NPCs with native quest gossip.
    for source, entry, name in ((900130, 900160, 'Patamon'), (900132, 900161, 'Impmon'),
                                (900134, 900162, 'Wizardmon')):
        clone_template(cursor, source, entry, name, 'DigimonQuestNPC', npcflag=3, faction=35,
                       minlevel=1, maxlevel=1, lootid=0, unit_flags=2)
    for source, entry, name in ((197, 900163, 'Cronista del Umbral'),
                                (197, 900164, 'Exploradora de Elwynn'),
                                (197, 900165, 'Mensajero de Villadorada')):
        clone_template(cursor, source, entry, name, 'DigimonQuestNPC', npcflag=3, faction=35,
                       minlevel=1, maxlevel=1, lootid=0, unit_flags=2)
    for source, entry, name, level, lootid in (
            (900133, 900150, 'Pagumon desincronizado', 3, 900150),
            (900133, 900151, 'Pagumon nodal', 5, 900151)):
        clone_template(cursor, source, entry, name, '', npcflag=0, faction=14,
                       minlevel=level, maxlevel=level, lootid=lootid, HealthModifier=0.8,
                       DamageModifier=0.6, ExperienceModifier=1, unit_flags=0)
    # Northshire introduction, Goldshire contacts and four low-level encounters.
    spawns = (
        (9000010, 900160, 0, -8898.8, -164.6, 82.6, 1.4),
        (9000011, 900161, 0, -9458.5, 88.5, 58.4, 3.1),
        (9000012, 900162, 0, -9467.0, 88.0, 58.4, 0.0),
        (9000013, 900150, 0, -8968.0, -52.8, 91.8, 0.0),
        (9000014, 900150, 0, -8976.5, -64.6, 90.5, 0.0),
        (9000015, 900150, 0, -8948.5, -83.9, 88.3, 2.0),
        (9000016, 900151, 0, -9397.0, 55.0, 59.8, 2.0),
    )
    for guid, entry, map_id, x, y, z, orientation in spawns:
        insert_row(cursor, 'creature', dict(guid=guid, id=entry, map=map_id,
                   position_x=x, position_y=y, position_z=z, orientation=orientation,
                   spawntimesecs=120, curhealth=100, Comment='mod-digimon-tamer: human-story quest chain'))
    # Story contacts are placed on the protected human road, away from hostile spawns.
    for guid, entry, map_id, x, y, z, orientation in (
            (9000017, 900163, 0, -8890.8, -166.6, 82.6, 1.4),
            (9000018, 900164, 0, -9425.0, 110.0, 58.8, 3.1),
            (9000019, 900165, 0, -9480.0, 125.0, 58.2, 0.0)):
        insert_row(cursor, 'creature', dict(guid=guid, id=entry, map=map_id,
                   position_x=x, position_y=y, position_z=z, orientation=orientation,
                   spawntimesecs=120, curhealth=100, Comment='mod-digimon-tamer: arrival-story'))
    cursor.execute("UPDATE creature SET position_x=-9425.0,position_y=110.0,position_z=58.8,orientation=3.1 "
                   "WHERE guid=9000018 AND id=900164 AND Comment='mod-digimon-tamer: arrival-story'")
    cursor.execute("UPDATE creature SET position_x=-9480.0,position_y=125.0,position_z=58.2,orientation=0 "
                   "WHERE guid=9000019 AND id=900165 AND Comment='mod-digimon-tamer: arrival-story'")
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
                for migration in sorted((ROOT / 'modules/mod-digimon-tamer/sql/world').glob('*.sql')):
                    apply_migration(cursor, migration)
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
                    cursor.execute('SELECT version FROM digimon_schema_version WHERE version=%s', (version,))
                    if cursor.fetchone():
                        continue
                    apply_migration(cursor, migration)
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
