#!/usr/bin/env python3
"""Run the pinned core's own style checkers against module files."""
import argparse
import ast
from pathlib import Path
import subprocess
import sys

if hasattr(sys.stdout, 'reconfigure'):
    sys.stdout.reconfigure(encoding='utf-8')

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('core', type=Path)
args = parser.parse_args()
module = Path(__file__).resolve().parents[1] / 'modules/mod-digimon-tamer'
checkers = args.core.resolve() / 'apps/codestyle'
cpp = subprocess.run([sys.executable, str(checkers / 'codestyle-cpp.py')], cwd=module)
# Load the upstream definitions without its hardcoded discovery/main invocation.
# Keep every check intact; only supply explicit module paths instead of fetching
# origin/master for core base/archive files. No checker source file is changed.
path = checkers / 'codestyle-sql.py'
tree = ast.parse(path.read_text(encoding='utf-8'), filename=str(path))
tree.body = [node for node in tree.body if not (
    isinstance(node, ast.Expr) and isinstance(node.value, ast.Call)
    and isinstance(node.value.func, ast.Name) and node.value.func.id == 'parsing_file')]
namespace = {'__file__': str(path), '__name__': 'digimon_upstream_sql_checker'}
exec(compile(tree, str(path), 'exec'), namespace)
files = [str(p) for p in sorted((module / 'sql/characters').glob('*.sql'))]
namespace['get_changed_files'] = lambda: files
try:
    namespace['parsing_file'](files)
    sql_code = 0
except SystemExit as result:
    sql_code = int(result.code or 0)
sys.exit(cpp.returncode or sql_code)
