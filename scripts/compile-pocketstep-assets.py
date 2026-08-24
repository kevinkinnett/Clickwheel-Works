"""Run the reusable PocketStep asset compiler from Clickwheel Works."""

from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "pocketstep" / "tools"))

from asset_compiler import main  # noqa: E402


if __name__ == "__main__":
    raise SystemExit(main())
