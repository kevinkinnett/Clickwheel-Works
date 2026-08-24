import hashlib
import importlib.util
import json
from pathlib import Path
import shutil
import tempfile
import unittest

from PIL import Image


POCKETSTEP_ROOT = Path(__file__).resolve().parents[1]
SCRIPT = POCKETSTEP_ROOT / "tools" / "asset_compiler.py"
FIXTURES = Path(__file__).resolve().parent / "fixtures" / "assets"
SPEC = importlib.util.spec_from_file_location("pocketstep_asset_compiler", SCRIPT)
assert SPEC is not None and SPEC.loader is not None
COMPILER = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(COMPILER)


class AssetCompilerTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.directory = Path(self.temporary.name)
        for source in FIXTURES.glob("*.json"):
            shutil.copy2(source, self.directory / source.name)
        pixels = Image.new("RGBA", (3, 3), (255, 255, 255, 255))
        pixels.putpixel((0, 0), (255, 0, 0, 255))
        pixels.putpixel((1, 0), (0, 255, 0, 255))
        pixels.putpixel((0, 1), (0, 0, 255, 255))
        pixels.putpixel((1, 1), (255, 255, 255, 255))
        pixels.save(self.directory / "source.png")
        strip = Image.new("RGBA", (2, 2), (12, 34, 56, 255))
        strip.putpixel((0, 0), (255, 255, 255, 0))
        strip.save(self.directory / "strip.png")

    def tearDown(self):
        self.temporary.cleanup()

    def compile(self, name):
        return COMPILER.compile_manifest(self.directory / name)

    def test_valid_transform_and_determinism(self):
        output = self.compile("valid.json")
        first = output.read_bytes()
        first_hash = hashlib.sha256(first).hexdigest()
        second = self.compile("valid.json").read_bytes()
        self.assertEqual(first, second)
        self.assertEqual(first_hash, hashlib.sha256(second).hexdigest())
        text = first.decode("ascii")
        self.assertIn("#define FIXTURE_WIDTH 4", text)
        self.assertIn("Fixture Author | CC0 | local test fixture", text)
        self.assertIn("fixture_sheet_day[8]", text)
        self.assertIn("fixture_sheet_night[8]", text)
        self.assertIn("0x00f8", text)
        self.assertIn("0x1ff8", text)

    def test_failures_preserve_existing_output(self):
        for manifest in (
            "invalid-color.json", "invalid-crop.json",
            "invalid-assembly.json", "invalid-operation.json",
            "duplicate.json", "missing-source.json",
        ):
            output = self.directory / "generated.h"
            output.write_text("known-good", encoding="ascii")
            with self.assertRaises(COMPILER.AssetError, msg=manifest):
                self.compile(manifest)
            self.assertEqual(output.read_text(encoding="ascii"), "known-good")

    def test_duplicate_variant_output(self):
        manifest_path = self.directory / "duplicate-variant.json"
        manifest = json.loads((self.directory / "valid.json").read_text())
        manifest["assets"].append({
            "name": "fixture_sheet_day",
            "operations": [{"op": "source", "source": "pixels"}],
        })
        manifest_path.write_text(json.dumps(manifest), encoding="utf-8")
        with self.assertRaises(COMPILER.AssetError):
            self.compile("duplicate-variant.json")


if __name__ == "__main__":
    unittest.main()
