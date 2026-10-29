#!/usr/bin/env python3
"""Incrementally catalogue model-converter output without changing source assets."""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import re
import struct
import sys
import tempfile
from datetime import datetime, timezone

VERSION = 1
ALIASES = {
    "beelzemon-bm": ("beelzemon blast mode", "blast mode beelzemon"),
    "malomyotismon": ("malo myotismon", "malomyotismon"),
    "venommyotismon": ("venom myotismon",),
    "ladydevimon": ("lady devimon",),
    "beelzemon": ("beelze-mon",),
}
MANUAL_MODEL_REVIEW = {
    "pagumon", "impmon", "wizardmon", "devimon", "bakemon", "witchmon",
    "myotismon", "phantomon", "ladydevimon", "beelzemon", "venommyotismon",
    "piedmon", "lilithmon", "beelzemonbm", "malomyotismon",
}
MANUAL_ABSENT = {"keemon", "yaamon", "baalmon"}
GENERATED_FIELDS = {
    "id", "key", "name", "aliases", "relative_directory", "files", "selected_glb",
    "alternate_glb_files", "possible_boss_npc_variant", "conversion_source_ready", "wow_asset_bundle_ready",
    "wow_asset_bundle", "wow_game_ready", "wow_assets", "review", "glb", "seen_at", "first_seen_at", "availability",
}


def stable_key(name: str) -> str:
    return re.sub(r"[^a-z0-9]+", "", name.casefold())


def internal_id(name: str) -> str:
    digest = hashlib.sha256(("world-digimon-model:v1:" + stable_key(name)).encode()).hexdigest()
    return "wdm-" + digest[:16]


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def parse_glb(path: Path, folder: Path) -> tuple[dict, list[str]]:
    raw = path.read_bytes()
    if len(raw) < 20:
        raise ValueError("truncated GLB header")
    magic, version, total = struct.unpack_from("<4sII", raw)
    if magic != b"glTF" or version != 2 or total != len(raw):
        raise ValueError("invalid GLB 2.0 header or byte length")
    offset = 12
    document = None
    while offset < total:
        if offset + 8 > total:
            raise ValueError("truncated GLB chunk header")
        size, kind = struct.unpack_from("<I4s", raw, offset)
        offset += 8
        if offset + size > total:
            raise ValueError("GLB chunk exceeds file length")
        if kind == b"JSON":
            if document is not None:
                raise ValueError("multiple JSON chunks")
            document = json.loads(raw[offset:offset + size].decode("utf-8").rstrip(" \t\r\n\0"))
        offset += size
    if document is None or offset != total:
        raise ValueError("missing JSON chunk")
    meshes = document.get("meshes", [])
    primitives = [primitive for mesh in meshes for primitive in mesh.get("primitives", [])]
    vertex_accessors = [primitive.get("attributes", {}).get("POSITION") for primitive in primitives]
    accessor_count = {item.get("count", 0) for item in document.get("accessors", [])}
    geometry = bool(primitives) and all(index is not None and
                  0 <= index < len(document.get("accessors", [])) and
                  document["accessors"][index].get("count", 0) > 0 and
                  "bufferView" in document["accessors"][index] for index in vertex_accessors)
    geometry = geometry and all(
        0 <= document["accessors"][index]["bufferView"] < len(document.get("bufferViews", []))
        for index in vertex_accessors
    ) if geometry else False
    if not accessor_count and geometry:
        geometry = False
    image_sources = document.get("images", [])
    texture_paths = []
    unresolved = []
    for image in image_sources:
        uri = image.get("uri")
        if not uri:
            if "bufferView" not in image:
                unresolved.append("embedded image without bufferView")
            continue
        if uri.startswith("data:"):
            continue
        texture = (path.parent / uri.replace("/", "\\")).resolve()
        try:
            texture.relative_to(folder.resolve())
        except ValueError:
            unresolved.append(uri)
            continue
        if not texture.is_file():
            unresolved.append(uri)
            continue
        head = texture.read_bytes()[:8]
        if not (head.startswith(b"\x89PNG\r\n\x1a\n") or head[:4] == b"BLP2"):
            unresolved.append(uri + " (unsupported image signature)")
            continue
        texture_paths.append(texture.relative_to(folder).as_posix())
    materials = document.get("materials", [])
    uses_texture = any(m.get("pbrMetallicRoughness", {}).get("baseColorTexture") for m in materials)
    textured = bool(image_sources) and bool(texture_paths or any(not i.get("uri") for i in image_sources))
    diagnostics = []
    if not geometry:
        diagnostics.append("no valid mesh POSITION accessors")
    if uses_texture and not textured:
        diagnostics.append("base-colour textures not embedded or resolved")
    if unresolved:
        diagnostics.append("unresolved images: " + ", ".join(unresolved))
    return ({"valid_glb": True, "mesh_count": len(meshes), "primitive_count": len(primitives),
             "image_count": len(image_sources), "texture_paths": sorted(set(texture_paths)),
             "has_geometry": geometry, "has_materials": bool(materials),
             "has_base_color_textures": uses_texture}, diagnostics)


def inventory_folder(folder: Path, root: Path, asset_root: Path | None = None) -> dict:
    slug = folder.name
    norm = stable_key(slug)
    paths = []
    for file in sorted(folder.rglob("*")):
        if not file.is_file() or file.is_symlink():
            continue
        relative = file.relative_to(root).as_posix()
        paths.append({"path": relative, "bytes": file.stat().st_size,
                      "sha256": sha256(file)})
    glbs = sorted((p for p in folder.rglob("*.glb") if p.is_file() and not p.is_symlink()),
                  key=lambda p: (p.stem.casefold() != slug.casefold(),
                                 "uncompressed" in p.stem.casefold(), p.as_posix().casefold()))
    chosen = None
    diagnostics = []
    model = {"valid_glb": False, "mesh_count": 0, "primitive_count": 0,
             "image_count": 0, "texture_paths": [], "has_geometry": False,
             "has_materials": False, "has_base_color_textures": False}
    if glbs:
        chosen = glbs[0]
        try:
            model, diagnostics = parse_glb(chosen, folder)
        except (OSError, ValueError, UnicodeError, json.JSONDecodeError, struct.error) as error:
            diagnostics = ["GLB inspection failed: " + str(error)]
    local_wow_assets = []
    for artifact in folder.rglob("*.m2"):
        if artifact.is_symlink() or artifact.read_bytes()[:4] != b"MD20":
            continue
        stem = artifact.stem.casefold()
        skins = sorted(p for p in artifact.parent.glob("*.skin")
                       if p.stem.casefold() == stem or p.stem.casefold().startswith(stem))
        textures = sorted(artifact.parent.glob("*.blp"))
        if skins and textures and all(texture.read_bytes()[:4] == b"BLP2" for texture in textures):
            local_wow_assets.append({"model": artifact.relative_to(root).as_posix(),
                                     "skins": [p.relative_to(root).as_posix() for p in skins],
                                     "textures": [p.relative_to(root).as_posix() for p in textures]})
    installed_assets = []
    if asset_root and asset_root.is_dir():
        wanted = norm
        for artifact in asset_root.rglob("*.m2"):
            parent_key = stable_key(artifact.parent.name)
            if parent_key != wanted:
                continue
            siblings = list(artifact.parent.glob("*.skin"))
            textures = list(artifact.parent.glob("*.blp"))
            if not siblings or not textures or artifact.read_bytes()[:4] != b"MD20":
                continue
            installed_assets.append({"model": artifact.relative_to(asset_root).as_posix(),
                                     "skins": sorted(p.relative_to(asset_root).as_posix() for p in siblings),
                                     "textures": sorted(p.relative_to(asset_root).as_posix() for p in textures)})
    installed_assets.sort(key=lambda item: item["model"].casefold())
    source_ready = bool(chosen and model["valid_glb"] and model["has_geometry"] and
                        (not model["has_base_color_textures"] or not diagnostics))
    review = norm in MANUAL_MODEL_REVIEW
    alternates = [p.relative_to(root).as_posix() for p in glbs if p != chosen]
    variant_text = " ".join([slug, *(Path(p["path"]).stem for p in paths)]).casefold()
    variant_signals = [term for term in ("boss", "npc", "blastmode", "raid", "mode", "bm")
                       if re.search(r"(?:^|[^a-z])" + term + r"(?:$|[^a-z])", variant_text)]
    return {
        "id": internal_id(norm), "key": norm, "name": slug,
        "aliases": list(ALIASES.get(slug.casefold(), ())),
        "relative_directory": folder.relative_to(root).as_posix(),
        "files": paths, "selected_glb": chosen.relative_to(root).as_posix() if chosen else None,
        "alternate_glb_files": alternates,
        "possible_boss_npc_variant": {"possible": bool(variant_signals), "signals": variant_signals},
        "conversion_source_ready": source_ready,
        "availability": "present",
        "wow_asset_bundle_ready": bool(local_wow_assets or installed_assets),
        "wow_asset_bundle": {"model": bool(local_wow_assets), "skin": bool(local_wow_assets),
                       "texture": bool(local_wow_assets), "local_artifacts": local_wow_assets,
                       "verified_asset_root_bundles": installed_assets},
        "review": {"manual_family_confirmed_by_user": review,
                   "status": "asset_bundle_available" if (local_wow_assets or installed_assets) else
                            "pending_conversion" if source_ready else "incomplete_source",
                   "confidence": "manual model list from prompt; format checks are automatic" if review
                                 else "automatic file inspection only",
                   "diagnostics": diagnostics},
        "glb": model,
        "seen_at": datetime.now(timezone.utc).isoformat(timespec="seconds"),
    }


def merge_inventory(root: Path, prior: dict, asset_root: Path | None = None) -> tuple[dict, list[str]]:
    found = {}
    for folder in sorted(root.iterdir(), key=lambda item: item.name.casefold()):
        if not folder.is_dir() or folder.is_symlink():
            continue
        key = stable_key(folder.name)
        if not key:
            continue
        if key in found:
            raise ValueError("ambiguous normalized model folders: " + folder.name)
        current = inventory_folder(folder, root, asset_root)
        if key in prior:
            old = prior[key]
            current["id"] = old["id"]
            current.update({field: value for field, value in old.items()
                            if field not in GENERATED_FIELDS and field not in {"first_seen_at"}})
            current["first_seen_at"] = old.get("first_seen_at", old.get("seen_at"))
        else:
            current["first_seen_at"] = current["seen_at"]
        found[key] = current
    missing = []
    for key, old in prior.items():
        if key not in found:
            retained = dict(old)
            retained["availability"] = "missing_from_source"
            found[key] = retained
            missing.append(key)
    spellings = {}
    for key, item in found.items():
        for label in [item["name"], *item.get("aliases", [])]:
            normalized = stable_key(label)
            if normalized in spellings and spellings[normalized] != key:
                raise ValueError("ambiguous model name or alias: " + label)
            spellings[normalized] = key
    return found, missing


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path,
                        default=Path(r"G:\DigiModelConverter-source\DigiModelConverter\output"))
    parser.add_argument("--catalog", type=Path,
                        default=Path(r"G:\World of Digimon\04_modelos\catalogo\model_catalog.json"))
    parser.add_argument("--asset-root", type=Path,
                        default=Path(r"G:\World of Digimon\04_modelos\trabajo"))
    args = parser.parse_args()
    root = args.source.resolve(strict=True)
    if not root.is_dir():
        parser.error("source must be a directory")
    prior = {}
    if args.catalog.exists():
        existing = json.loads(args.catalog.read_text(encoding="utf-8"))
        if existing.get("schema_version") != VERSION:
            parser.error("catalog schema_version mismatch; migrate it before merging")
        prior = {item["key"]: item for item in existing.get("models", [])}
    try:
        found, missing = merge_inventory(root, prior, args.asset_root if args.asset_root.exists() else None)
    except ValueError as error:
        parser.error(str(error))
    catalog = {"schema_version": VERSION,
               "source_label": "DigiModelConverter/output",
               "generated_at": datetime.now(timezone.utc).isoformat(timespec="seconds"),
               "policy": {"original_assets_modified": False, "name_is_not_game_readiness": True,
                          "database_ids_assigned": False,
                          "unavailable_names_from_user_prompt": sorted(MANUAL_ABSENT)},
               "models": sorted(found.values(), key=lambda item: item["key"])}
    ids = [item["id"] for item in catalog["models"]]
    keys = [item["key"] for item in catalog["models"]]
    if len(ids) != len(set(ids)) or len(keys) != len(set(keys)):
        parser.error("catalog contains colliding stable model IDs or normalized names")
    args.catalog.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(mode="w", encoding="utf-8", newline="\n", dir=args.catalog.parent,
                                     prefix=args.catalog.name + ".", suffix=".tmp", delete=False) as stream:
        json.dump(catalog, stream, ensure_ascii=False, indent=2)
        stream.write("\n")
        temp = Path(stream.name)
    temp.replace(args.catalog)
    ready = sum(item.get("conversion_source_ready", False) for item in found.values())
    bundle = sum(item.get("wow_asset_bundle_ready", False) for item in found.values())
    print(f"catalogue updated incrementally: {len(found)} records; source-ready={ready}; "
          f"asset-bundle-ready={bundle}; missing={len(missing)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
