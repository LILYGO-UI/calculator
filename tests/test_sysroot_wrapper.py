"""Check SDK delegation without downloading or extracting packages."""

import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest


WRAPPER = Path(__file__).resolve().parents[1] / "tools/prepare-riscv64-sysroot.py"


class SysrootWrapperTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix="calculator-sysroot-wrapper-")
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name).resolve()
        self.project = self.root / "calculator"
        self.wrapper = self.project / "tools/prepare-riscv64-sysroot.py"
        self.wrapper.parent.mkdir(parents=True)
        shutil.copy2(WRAPPER, self.wrapper)
        self.vendor = self.make_sdk(self.project / "third_party/cm0-appkit")
        self.environment = dict(os.environ)
        self.environment.pop("LILYGO_UI_APPKIT_DIR", None)

    def make_sdk(self, sdk):
        script = sdk / "tools/prepare-riscv64-sysroot.py"
        script.parent.mkdir(parents=True)
        script.write_text(
            "import json, os, sys\n"
            "print(json.dumps({'script': __file__, 'args': sys.argv[1:]}))\n"
            "sys.exit(int(os.environ.get('TEST_SDK_EXIT', '0')))\n")
        return script

    def run_wrapper(self, *arguments):
        return subprocess.run([sys.executable, "-B", str(self.wrapper), *arguments],
                              cwd=self.root, env=self.environment,
                              capture_output=True, text=True, timeout=10)

    def test_default_sdk_preserves_calculator_output_and_forwards_options(self):
        result = self.run_wrapper("--packages", "libextra-dev", "--jobs", "2")
        self.assertEqual(result.returncode, 0, result.stderr)
        data = json.loads(result.stdout)
        self.assertEqual(data["script"], str(self.vendor))
        self.assertEqual(data["args"], ["--packages", "libextra-dev", "--jobs", "2",
                                      "--output", str(self.project / ".cache/k230-debian/sysroot")])

    def test_selected_sdk_respects_explicit_output_and_exit_status(self):
        selected = self.make_sdk(self.root / "selected SDK")
        self.environment["LILYGO_UI_APPKIT_DIR"] = str(selected.parents[1])
        self.environment["TEST_SDK_EXIT"] = "7"
        for arguments in (("--output", "relative output"), ("--output=relative output",)):
            with self.subTest(arguments=arguments):
                result = self.run_wrapper(*arguments)
                self.assertEqual(result.returncode, 7, result.stderr)
                data = json.loads(result.stdout)
                self.assertEqual(data["script"], str(selected))
                self.assertEqual(data["args"], list(arguments))

    def test_missing_vendor_has_actionable_error_without_sibling_fallback(self):
        self.vendor.unlink()
        self.make_sdk(self.root / "cm0-appkit")
        result = self.run_wrapper()
        self.assertEqual(result.returncode, 2)
        self.assertIn("selected AppKit", result.stderr)
        self.assertIn("LILYGO_UI_APPKIT_DIR", result.stderr)
        self.assertEqual(result.stdout, "")

    def test_rejects_recursive_sdk_selection(self):
        self.environment["LILYGO_UI_APPKIT_DIR"] = str(self.project)
        result = self.run_wrapper()
        self.assertEqual(result.returncode, 2)
        self.assertIn("no usable", result.stderr)


if __name__ == "__main__":
    unittest.main()
