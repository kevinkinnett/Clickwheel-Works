"""Compatibility entry point for the Story Clock asset manifest."""

from pathlib import Path
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[1]


def main() -> int:
    return subprocess.call([
        sys.executable,
        str(ROOT / "scripts" / "compile-pocketstep-assets.py"),
        str(ROOT / "assets" / "storyclock" / "assets.json"),
    ])


if __name__ == "__main__":
    raise SystemExit(main())
