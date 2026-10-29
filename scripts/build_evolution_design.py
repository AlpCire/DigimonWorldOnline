#!/usr/bin/env python3
"""Generate a sourced, model-gated evolution graph from the model inventory."""
import argparse
import csv
import hashlib
import json
from pathlib import Path
import sys
import tempfile
from datetime import datetime, timezone

from scan_model_output import stable_key

STAGES = {
    "pagumon": ("Baby II", 2, 5),
    "impmon": ("Rookie", 3, 10),
    "wizardmon": ("Champion", 4, 25),
    "devimon": ("Champion", 4, 25),
    "bakemon": ("Champion", 4, 25),
    "witchmon": ("Champion", 4, 25),
    "myotismon": ("Ultimate / Perfect", 5, 45),
    "phantomon": ("Ultimate / Perfect", 5, 45),
    "ladydevimon": ("Ultimate / Perfect", 5, 45),
    "beelzemon": ("Mega", 6, 65),
    "venommyotismon": ("Mega", 6, 65),
    "piedmon": ("Mega", 6, 65),
    "lilithmon": ("Mega", 6, 65),
    "malomyotismon": ("Mega", 6, 65),
    "beelzemonbm": ("Special mode", 7, 80),
    "patamon": ("Rookie", 3, 10),
    "angemon": ("Champion", 4, 25),
}
DESIGN_EDGES = [
    ("Pagumon", "Impmon", 10, None),
    ("Impmon", "Wizardmon", 25, None),
    ("Impmon", "Devimon", 25, None),
    ("Impmon", "Bakemon", 25, None),
    ("Impmon", "Witchmon", 25, None),
    ("Wizardmon", "Beelzemon", 70, "Baalmon model absent; direct skip specified by project design"),
    ("Beelzemon", "Beelzemon Blast Mode", 85, "Special-mode unlock threshold specified by project design"),
    ("Devimon", "Myotismon", 45, None),
    ("Myotismon", "VenomMyotismon", 65, None),
    ("VenomMyotismon", "MaloMyotismon", 80, None),
    ("Bakemon", "Phantomon", 45, None),
    ("Phantomon", "Piedmon", 65, None),
    ("Witchmon", "LadyDevimon", 45, None),
    ("LadyDevimon", "Lilithmon", 65, None),
    ("Patamon", "Angemon", 22, "Existing enabled laboratory evolution rule"),
]
DISPLAY = {
    "beelzemonbm": "Beelzemon Blast Mode",
    "beelzemon": "Beelzemon",
    "ladydevimon": "LadyDevimon",
    "venommyotismon": "VenomMyotismon",
    "malomyotismon": "MaloMyotismon",
}


def read_csv(path: Path) -> list[dict]:
    if not path.exists():
        return []
    with path.open(encoding="utf-8-sig", newline="") as stream:
        return list(csv.DictReader(stream, delimiter=";"))


def make_id(source: str, target: str) -> str:
    return "evo-" + hashlib.sha256((source + "->" + target).encode()).hexdigest()[:16]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--models", type=Path,
                        default=Path(r"G:\World of Digimon\04_modelos\catalogo\model_catalog.json"))
    parser.add_argument("--catalog-dir", type=Path,
                        default=Path(r"G:\World of Digimon\04_modelos\catalogo"))
    args = parser.parse_args()
    models = json.loads(args.models.read_text(encoding="utf-8"))
    by_name = {item["key"]: item for item in models["models"]}
    alias_to_key = {}
    for item in models["models"]:
        for alias in item.get("aliases", []):
            alias_to_key[stable_key(alias)] = item["key"]
    species_rows = read_csv(args.catalog_dir / "digimon_species.csv")
    species_ids = {stable_key(row["name"]): int(row["id"]) for row in species_rows}
    edge_csv = args.catalog_dir / "digimon_evolution_edges.csv"
    rules = read_csv(edge_csv if edge_csv.exists() else args.catalog_dir / "digimon_evolution_rules.csv")
    live_rules = {(int(row["source_species"]), int(row["target_species"])): int(row["required_level"])
                  for row in rules if int(row["enabled"])}
    old_path = args.catalog_dir / "evolution_graph.json"
    old = json.loads(old_path.read_text(encoding="utf-8")) if old_path.exists() else {}
    old_edges = {edge["id"]: edge for edge in old.get("edges", [])}
    nodes = []
    for key, item in by_name.items():
        stage = STAGES.get(key)
        database_id = species_ids.get(key)
        db_enabled = bool(database_id and any(
            int(row["id"]) == database_id and int(row["enabled"]) == 1
            for row in species_rows))
        nodes.append({"model_id": item["id"], "name": DISPLAY.get(key, item["name"].capitalize()),
                      "stage_default": ({"name": stage[0], "rank": stage[1], "level": stage[2]}
                                        if stage else None),
                      "source_model_available": item.get("conversion_source_ready", False),
                      "wow_assets_available": item.get("wow_asset_bundle_ready", False),
                      "database_species_id": database_id, "database_species_enabled": db_enabled,
                      "status": "pending_review" if not stage else "model_checked"})
    edges = []
    for source_name, target_name, level, note in DESIGN_EDGES:
        source_key, target_key = stable_key(source_name), stable_key(target_name)
        source_key = alias_to_key.get(source_key, source_key)
        target_key = alias_to_key.get(target_key, target_key)
        source, target = by_name.get(source_key), by_name.get(target_key)
        if source is None or target is None:
            continue
        source_db, target_db = species_ids.get(source_key), species_ids.get(target_key)
        source_enabled = bool(source_db and any(int(row["id"]) == source_db and int(row["enabled"]) == 1
                                                for row in species_rows))
        target_enabled = bool(target_db and any(int(row["id"]) == target_db and int(row["enabled"]) == 1
                                                for row in species_rows))
        runtime_level = live_rules.get((source_db, target_db)) if source_db and target_db else None
        old_edge = old_edges.get(make_id(source["id"], target["id"]), {})
        edge = {"id": make_id(source["id"], target["id"]),
                "source_model_id": source["id"], "target_model_id": target["id"],
                "source": DISPLAY.get(source_key, source["name"].capitalize()),
                "target": DISPLAY.get(target_key, target["name"].capitalize()),
                "classification": "MMO_CUSTOM",
                "classification_source": "User-provided WORLD DIGIMON MMO design prompt, 2026-10-16",
                "confidence": {"design_score": 1.0, "canonical_score": 0.0,
                               "basis": "explicit_user_design_prompt; no independent canon verification"},
                "minimum_level": level,
                "current_engine_level_cap": 100,
                "reachable_under_current_level_cap": level <= 100,
                "special_note": note,
                "model_asset_bundles_available": bool(source.get("wow_asset_bundle_ready") and
                                                       target.get("wow_asset_bundle_ready")),
                "database_species_enabled": bool(source_enabled and target_enabled),
                "database_rule_enabled": bool(runtime_level == level and source_enabled and target_enabled),
                "player_usable": bool(source.get("wow_asset_bundle_ready") and target.get("wow_asset_bundle_ready")
                                      and source_enabled and target_enabled and runtime_level == level),
                "unconfigured_requirements": ["affinities", "quests", "catalysts", "discovered_form"],
                "status": ("blocked_level_cap" if level > 100 else
                           "active_runtime_edge" if runtime_level == level else
                           "pending_game_assets" if not (source.get("wow_asset_bundle_ready") and
                                                         target.get("wow_asset_bundle_ready")) else
                           "design_only_not_connected_to_runtime")}
        for field in ("review", "affinity_requirements", "quest_requirements", "catalyst_requirements"):
            if field in old_edge:
                edge[field] = old_edge[field]
        edges.append(edge)
    outgoing, incoming = {}, {}
    for edge in edges:
        outgoing.setdefault(edge["source_model_id"], []).append(edge["id"])
        incoming.setdefault(edge["target_model_id"], []).append(edge["id"])
    absent = [name for name in ("Keemon", "Yaamon", "Baalmon")
              if stable_key(name) not in by_name and stable_key(name) not in alias_to_key]
    graph = {"schema_version": 1,
             "generated_at": datetime.now(timezone.utc).isoformat(timespec="seconds"),
             "stage_defaults": [
                 {"name": "Baby I", "rank": 1, "default_level": 1},
                 {"name": "Baby II", "rank": 2, "default_level": 5},
                 {"name": "Rookie", "rank": 3, "default_level": 10},
                 {"name": "Champion", "rank": 4, "default_level": 25},
                 {"name": "Ultimate / Perfect", "rank": 5, "default_level": 45},
                 {"name": "Mega", "rank": 6, "default_level": 65},
                 {"name": "Special mode", "rank": 7, "default_level": 80}],
             "nodes": sorted(nodes, key=lambda node: node["model_id"]),
             "edges": sorted(edges, key=lambda edge: edge["id"]),
             "adjacency": {"outgoing": outgoing, "incoming": incoming},
             "excluded_models": [{"name": name, "status": "not_available_in_source_output"}
                                 for name in absent],
             "policy": {"unverified_official_edges_added": 0,
                        "player_choices_require_models_runtime_rule_and_requirements": True,
                        "this_file_is_design_data_not_a_live_runtime_evolution_graph": True}}
    with tempfile.NamedTemporaryFile(mode="w", encoding="utf-8", newline="\n", dir=args.catalog_dir,
                                     prefix=old_path.name + ".", suffix=".tmp", delete=False) as stream:
        json.dump(graph, stream, ensure_ascii=False, indent=2)
        stream.write("\n")
        temp = Path(stream.name)
    temp.replace(old_path)
    print(f"evolution graph generated: {len(nodes)} models, {len(edges)} user-designed edges, "
          f"runtime-usable={sum(edge['player_usable'] for edge in edges)}, excluded={len(absent)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
