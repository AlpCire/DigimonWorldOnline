from pathlib import Path
import re
r=Path(__file__).resolve().parents[2]
cpp=(r/"modules/mod-digimon-tamer/src/DigimonTalentTree.h").read_text(encoding="utf-8")
lua=(r/"client/WorldDigimon/Talents.lua").read_text(encoding="utf-8")
c={int(x[0]):tuple(map(int,x[1:])) for x in re.findall(r'\{(\d+), (\d+), (\d+), (\d+), (\d+), (\d+), (\d+), (\d+)\}',cpp)}
l={int(x[0]):tuple(map(int,x[1:])) for x in re.findall(r'^\s*\[(\d+)\]=\{(\d+),(\d+),(\d+),(\d+),(\d+),(\d+),(\d+),',lua,re.M)}
assert len(c)==len(l)==27,(len(c),len(l))
assert all(c[id][:2]==l[id][:2] and c[id][2]+1==l[id][2] and c[id][3:]==l[id][3:] for id in c),[(id,c.get(id),l.get(id)) for id in c if c[id][2]+1!=l[id][2]]
assert 'Talents.lua' in (r/"client/WorldDigimon/WorldDigimon.toc").read_text()
print("PASS: 27 client nodes match server gates and positions")
