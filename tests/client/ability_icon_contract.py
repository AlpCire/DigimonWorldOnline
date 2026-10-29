from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
ADDON = ROOT / "client" / "WorldDigimon"
icons = (ADDON / "AbilityIcons.lua").read_text(encoding="utf-8")
combat = (ADDON / "Combat.lua").read_text(encoding="utf-8")
panels = (ADDON / "Panels.lua").read_text(encoding="utf-8")
toc = (ADDON / "WorldDigimon.toc").read_text(encoding="utf-8")
entries = {int(k): v for k, v in re.findall(r"\[(\d+)\] = \"(physical|fire|shadow|nature|arcane)\"", icons)}
assert len(entries) == 70 and set(entries) == set(range(1000, 1070))
for family in set(entries.values()):
    asset = ADDON / "Textures" / "Abilities" / f"{family}.tga"
    assert asset.is_file() and asset.stat().st_size > 1000, f"missing/empty texture: {family}"
assert "AbilityIcons.lua" in toc and toc.index("AbilityIcons.lua") < toc.index("Combat.lua")
assert "WorldDigimonAbilityIcons[slot.technique]" in combat
assert "WorldDigimonAbilityIcons[t.id]" in panels
assert "custom..\".tga\"" in combat and "custom..\".tga\"" in panels
assert "GetSpellInfo(slot.spell)" in combat and "GetSpellInfo(t.spell)" in panels
print(f"OK: {len(entries)} technique mappings, {len(set(entries.values()))} icon families, bar and skills panel use custom texture paths")
