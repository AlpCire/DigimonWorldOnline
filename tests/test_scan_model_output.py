import importlib.util
import json
from pathlib import Path
import struct
import tempfile
import unittest

SCRIPT = Path(__file__).resolve().parents[1] / "scripts" / "scan_model_output.py"
SPEC = importlib.util.spec_from_file_location("scan_model_output", SCRIPT)
SCAN = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(SCAN)


def make_glb(path: Path) -> None:
    image = path.parent / "texture.png"
    image.write_bytes(b"\x89PNG\r\n\x1a\n" + b"fixture")
    document = {
        "asset": {"version": "2.0"},
        "buffers": [{"byteLength": 12}],
        "bufferViews": [{"buffer": 0, "byteOffset": 0, "byteLength": 12}],
        "accessors": [{"bufferView": 0, "componentType": 5126, "count": 1, "type": "VEC3"}],
        "images": [{"uri": "texture.png"}],
        "textures": [{"source": 0}],
        "materials": [{"pbrMetallicRoughness": {"baseColorTexture": {"index": 0}}}],
        "meshes": [{"primitives": [{"attributes": {"POSITION": 0}, "material": 0}]}],
    }
    payload = json.dumps(document, separators=(",", ":")).encode()
    payload += b" " * ((-len(payload)) % 4)
    binary = b"\0" * 12
    total = 12 + 8 + len(payload) + 8 + len(binary)
    path.write_bytes(struct.pack("<4sII", b"glTF", 2, total) +
                     struct.pack("<I4s", len(payload), b"JSON") + payload +
                     struct.pack("<I4s", len(binary), b"BIN\0") + binary)


class CatalogScannerTests(unittest.TestCase):
    def test_readiness_needs_geometry_images_and_wow_files(self):
        with tempfile.TemporaryDirectory(dir=Path(__file__).parent) as temp:
            root = Path(temp)
            folder = root / "Example-Mon"
            folder.mkdir()
            make_glb(folder / "Example-Mon.glb")
            result = SCAN.inventory_folder(folder, root)
            self.assertTrue(result["conversion_source_ready"])
            self.assertFalse(result["wow_asset_bundle_ready"])
            (folder / "example.m2").write_bytes(b"MD20" + b"\0" * 20)
            (folder / "example00.skin").write_bytes(b"skin")
            (folder / "texture.blp").write_bytes(b"BLP2")
            self.assertTrue(SCAN.inventory_folder(folder, root)["wow_asset_bundle_ready"])
            (folder / "Example-Mon.glb").write_bytes(b"not a glb")
            self.assertFalse(SCAN.inventory_folder(folder, root)["conversion_source_ready"])

    def test_installed_artifacts_are_linked_by_name_and_valid_m2_signature(self):
        with tempfile.TemporaryDirectory(dir=Path(__file__).parent) as temp:
            root = Path(temp) / "source"
            installed = Path(temp) / "installed"
            folder = root / "Patamon"
            artifacts = installed / "patch" / "Creature" / "WorldDigimon" / "Patamon"
            folder.mkdir(parents=True)
            artifacts.mkdir(parents=True)
            make_glb(folder / "Patamon.glb")
            (artifacts / "Patamon.m2").write_bytes(b"MD20" + b"\0" * 20)
            (artifacts / "Patamon00.skin").write_bytes(b"SKIN")
            (artifacts / "texture.blp").write_bytes(b"BLP2")
            result = SCAN.inventory_folder(folder, root, installed)
            self.assertTrue(result["wow_asset_bundle_ready"])
            self.assertEqual("patch/Creature/WorldDigimon/Patamon/Patamon.m2",
                             result["wow_asset_bundle"]["verified_asset_root_bundles"][0]["model"])

    def test_incremental_rescan_preserves_ids_annotations_and_missing_records(self):
        with tempfile.TemporaryDirectory(dir=Path(__file__).parent) as temp:
            root = Path(temp)
            source = root / "source"
            source.mkdir()
            folder = source / "Impmon"
            folder.mkdir()
            make_glb(folder / "Impmon.glb")
            first = SCAN.inventory_folder(folder, source)
            first["manual_stage"] = {"value": 10, "source": "user review"}
            first["future_manual_annotation"] = "preserve me"
            old_id = first["id"]
            merged, missing = SCAN.merge_inventory(source, {"impmon": first})
            self.assertFalse(missing)
            self.assertEqual(old_id, merged["impmon"]["id"])
            self.assertEqual("user review", merged["impmon"]["manual_stage"]["source"])
            self.assertEqual("preserve me", merged["impmon"]["future_manual_annotation"])
            (source / "Impmon").rename(source / "Impmon-away")
            merged, missing = SCAN.merge_inventory(source, {"impmon": first})
            self.assertEqual(["impmon"], missing)
            self.assertEqual("missing_from_source", merged["impmon"]["availability"])
            self.assertEqual(SCAN.internal_id("Impmon"), SCAN.internal_id("impmon"))
            self.assertEqual(SCAN.stable_key("Beelzemon Blast Mode"), "beelzemonblastmode")


if __name__ == "__main__":
    unittest.main()
