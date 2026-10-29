"""Execute module SQL and integrity checks in a newly created disposable schema."""
import os
from pathlib import Path
import uuid

import pymysql
from pymysql.constants import CLIENT

root = Path(__file__).resolve().parents[2]
name = 'digimon_test_' + uuid.uuid4().hex[:16]
connection = pymysql.connect(host=os.environ.get('DIGIMON_DB_HOST', 'ac-database'),
                             port=int(os.environ.get('DIGIMON_DB_PORT', '3306')),
                             user=os.environ.get('DIGIMON_DB_USER', 'root'),
                             password=os.environ['DIGIMON_DB_PASSWORD'],
                             autocommit=True, client_flag=CLIENT.MULTI_STATEMENTS)
checks = 0


def reject(cursor, statement, expected):
    global checks
    try:
        cursor.execute(statement)
    except pymysql.MySQLError as error:
        if error.args[0] not in expected:
            raise
        checks += 1
    else:
        raise AssertionError('Invalid persistent state was accepted: ' + statement)


created = False
try:
    with connection.cursor() as cursor:
        cursor.execute(f'CREATE DATABASE `{name}` CHARACTER SET utf8mb4')
        created = True
        connection.select_db(name)
        cursor.execute('CREATE TABLE characters (guid INT UNSIGNED PRIMARY KEY) ENGINE=InnoDB')
        cursor.execute('INSERT INTO characters VALUES (1),(2)')
        migrations = sorted((root / 'modules/mod-digimon-tamer/sql/characters').glob('*.sql'))
        for migration in migrations:
            if migration.name.startswith('006_'):
                # An upgrade must retain old players and backfill their principal.
                cursor.execute('INSERT INTO digimon_tamer (owner_guid) VALUES (1),(2)')
                cursor.execute('INSERT INTO digimon_collection (id,owner_guid,species_id,is_starter,starter_owner) '
                               'VALUES (101,1,1,1,1),(102,2,1,1,2)')
                cursor.execute('INSERT INTO digimon_active VALUES (1,101),(2,102)')
            cursor.execute(migration.read_text(encoding='utf-8'))
            while cursor.nextset():
                pass
        # The graph seed migration is idempotent and can be replayed without duplicate edges.
        graph_migration = next(migration for migration in migrations if migration.name.startswith('008_'))
        cursor.execute(graph_migration.read_text(encoding='utf-8'))
        while cursor.nextset():
            pass
        cursor.execute('SELECT owner_guid,slot,companion_id FROM digimon_team ORDER BY owner_guid')
        assert cursor.fetchall() == ((1, 0, 101), (2, 0, 102))
        checks += 1
        cursor.execute('INSERT INTO digimon_collection (id,owner_guid,species_id) VALUES (103,2,1),(104,1,1)')
        cursor.execute('SELECT combat_role FROM digimon_collection WHERE id=101')
        assert cursor.fetchone() == ('DPS',)
        cursor.execute("UPDATE digimon_collection SET combat_role='TANK' WHERE id=101")
        cursor.execute('INSERT INTO digimon_talent_points (companion_id,talent_id) VALUES (101,1),(101,3)')
        cursor.execute('SELECT talent_id FROM digimon_talent_points WHERE companion_id=101 ORDER BY id')
        assert cursor.fetchall() == ((1,), (3,))
        reject(cursor, 'INSERT INTO digimon_talent_points (companion_id,talent_id) VALUES (101,4)', {3819})
        reject(cursor, 'INSERT INTO digimon_talent_points (companion_id,talent_id) VALUES (999999,1)', {1452})
        cursor.execute('SELECT spell_id FROM digimon_techniques WHERE id=2002')
        assert cursor.fetchone() == (2061,)
        checks += 4
        reject(cursor, 'INSERT INTO digimon_collection (owner_guid,species_id,is_starter,starter_owner) '
                       'VALUES (1,1,1,1)', {1062})
        reject(cursor, 'INSERT INTO digimon_collection (owner_guid,species_id,is_starter) VALUES (1,1,1)', {3819})
        reject(cursor, 'UPDATE digimon_active SET companion_id=103 WHERE owner_guid=1', {1452})
        reject(cursor, 'INSERT INTO digimon_team VALUES (1,1,103)', {1452})
        reject(cursor, 'INSERT INTO digimon_team VALUES (1,3,104)', {3819})
        cursor.execute("INSERT INTO digimon_collection (owner_guid,species_id,capture_key) VALUES (1,1,'unique-life')")
        reject(cursor, "INSERT INTO digimon_collection (owner_guid,species_id,capture_key) VALUES (2,1,'unique-life')", {1062})
        reject(cursor, 'INSERT INTO digimon_equipped VALUES (101,0,1)', {1452})
        reject(cursor, 'INSERT INTO digimon_evolution_rules VALUES (1,1,22,1)', {3819})
        reject(cursor, 'INSERT INTO digimon_evolution_rules VALUES (1,101,0,1)', {3819})
        reject(cursor, "INSERT INTO digimon_aliases VALUES ('missing',999999)", {1452})
        reject(cursor, 'INSERT INTO digimon_species_techniques VALUES (1,999999)', {1452})
        cursor.execute("SELECT target_species,required_level,enabled FROM digimon_evolution_rules WHERE source_species=101")
        assert cursor.fetchone() == (102, 22, 0)
        checks += 1
        cursor.execute("SELECT target_species,required_level,enabled,classification,confidence "
                       "FROM digimon_evolution_edges WHERE source_species=101")
        assert cursor.fetchone() == (102, 22, 0, 'MMO_CUSTOM', 0.0)
        checks += 1
        cursor.execute("INSERT INTO digimon_species (id,creature_entry,name,stage,enabled) "
                       "VALUES (118,900200,'Test Branch',2,0),(119,900201,'Test Branch 2',2,0)")
        cursor.execute("INSERT INTO digimon_evolution_edges "
                       "(source_species,target_species,required_level,enabled,classification,source_note,confidence) "
                       "VALUES (101,118,22,0,'MMO_CUSTOM','schema branch test',0)")
        cursor.execute("SELECT COUNT(*) FROM digimon_evolution_edges WHERE source_species=101")
        assert cursor.fetchone()[0] == 2
        checks += 1
        reject(cursor, "INSERT INTO digimon_evolution_edges "
                       "(source_species,target_species,required_level) VALUES (101,118,22)", {1062})
        cursor.execute("INSERT INTO digimon_evolution_edges "
                       "(source_species,target_species,required_level,enabled) VALUES (101,119,85,1)")
        cursor.execute("INSERT INTO digimon_evolution_edges "
                       "(source_species,target_species,required_level,enabled) VALUES (118,119,100,1)")
        reject(cursor, "INSERT INTO digimon_evolution_edges "
                       "(source_species,target_species,required_level) VALUES (119,101,101)", {3819})
        cursor.execute("INSERT INTO digimon_collection (owner_guid,species_id,level) VALUES (1,1,100)")
        reject(cursor, "INSERT INTO digimon_collection (owner_guid,species_id,level) VALUES (2,1,101)", {3819})
        cursor.execute("INSERT INTO digimon_companion_forms (companion_id,species_id) VALUES (101,101)")
        cursor.execute("INSERT INTO digimon_evolution_history (companion_id,from_species,to_species,level,action) "
                       "VALUES (101,101,102,100,'EVOLVE')")
        cursor.execute('SELECT points,last_care_at FROM digimon_bond WHERE companion_id=101')
        assert cursor.fetchone() is None
        cursor.execute('INSERT INTO digimon_bond(companion_id,points) VALUES (101,0)')
        cursor.execute('UPDATE digimon_bond SET points=LEAST(points+37,100) WHERE companion_id=101')
        cursor.execute('SELECT points FROM digimon_bond WHERE companion_id=101')
        assert cursor.fetchone() == (37,)
        checks += 1
        reject(cursor, 'INSERT INTO digimon_bond(companion_id,points) VALUES (101,101)', {3819})
        reject(cursor, 'INSERT INTO digimon_bond(companion_id,points) VALUES (999999,1)', {1452})
        cursor.execute("INSERT INTO digimon_techniques (id,name,spell_id,required_stage,price,enabled,special) "
                       "VALUES (101,'Nombre Digital',17253,1,20,1,0)")
        cursor.execute('SELECT COUNT(*) FROM digimon_techniques WHERE spell_id=17253')
        assert cursor.fetchone()[0] == 2
        checks += 1
        cursor.execute('SELECT COUNT(*) FROM digimon_techniques WHERE id BETWEEN 1000 AND 1069')
        assert cursor.fetchone()[0] == 70
        checks += 1
        cursor.execute("SELECT min_level,cooldown_override_ms,category,localized_name FROM digimon_techniques WHERE name='Night of Fire'")
        assert cursor.fetchone() == (10, 5000, 'GAME_REFERENCE', 'Noche de fuego')
        checks += 1
        cursor.execute('SELECT COUNT(*) FROM digimon_techniques WHERE control_limited=1')
        assert cursor.fetchone()[0] == 7
        checks += 1
        cursor.execute('SELECT COUNT(*) FROM digimon_species WHERE id BETWEEN 104 AND 117 AND enabled=0')
        assert cursor.fetchone()[0] >= 14
        checks += 1
        cursor.execute('SELECT COUNT(*) FROM digimon_evolution_edges WHERE source_species BETWEEN 103 AND 117 AND enabled=0')
        assert cursor.fetchone()[0] == 9
        checks += 1
        cursor.execute('SELECT COUNT(*) FROM digimon_schema_version')
        assert cursor.fetchone()[0] == len(migrations)
        checks += 1
        print(f'PASS: fresh SQL, upgrade backfill and {checks} schema integrity checks; disposable schema only')
finally:
    if created:
        with connection.cursor() as cursor:
            cursor.execute(f'DROP DATABASE `{name}`')
    connection.close()
