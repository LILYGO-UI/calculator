#!/usr/bin/env python3
"""Prepare the calculator sysroot through the selected AppKit's public tool."""

import argparse
import os
from pathlib import Path
import subprocess
import sys


def main(argv=None):
    project = Path(__file__).resolve().parents[1]
    sdk = Path(os.environ.get("LILYGO_UI_APPKIT_DIR") or project / "third_party/cm0-appkit").expanduser()
    if not sdk.is_absolute():
        sdk = project / sdk
    script = sdk.resolve() / "tools/prepare-riscv64-sysroot.py"
    if not script.is_file() or script.resolve() == Path(__file__).resolve():
        print(
            f"The selected AppKit has no usable Debian sysroot preparation tool: {script}\n"
            "Update the pinned third_party/cm0-appkit SDK, or set "
            "LILYGO_UI_APPKIT_DIR to a compatible source SDK.", file=sys.stderr)
        return 2

    arguments = list(sys.argv[1:] if argv is None else argv)
    # Preserve this entry point's old cache location. All other options,
    # including package extensions and metadata repair, belong to the SDK.
    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument("--output")
    options, _ = parser.parse_known_args(arguments)
    if options.output is None:
        arguments += ["--output", str(project / ".cache/k230-debian/sysroot")]
    return subprocess.call([sys.executable, str(script), *arguments])


if __name__ == "__main__":
    sys.exit(main())
