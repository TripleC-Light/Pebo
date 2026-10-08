import unittest

from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from pebo_bridge_core import BridgeError, list_project_files, project_status, read_project_file


class PeboBridgeCoreTests(unittest.TestCase):
    def test_project_status(self):
        status = project_status()
        self.assertEqual(status["project"], "Pebo")
        self.assertIn("docs", status)
        self.assertIn("Firmware", status["allowed_roots"])

    def test_list_docs(self):
        files = list_project_files("docs")
        paths = {entry["relative_path"].replace("\\", "/") for entry in files}
        self.assertIn("docs/hardware.md", paths)

    def test_read_agents(self):
        result = read_project_file("AGENTS.md")
        self.assertEqual(result["relative_path"], "AGENTS.md")
        self.assertIn("Pebo project instructions", result["content"])

    def test_reject_path_traversal(self):
        with self.assertRaises(BridgeError):
            read_project_file("../AGENTS.md")

    def test_reject_absolute_path(self):
        with self.assertRaises(BridgeError):
            read_project_file("C:/Users/Edward/.codex/auth.json")

    def test_reject_credential_name(self):
        with self.assertRaises(BridgeError):
            read_project_file(".codex/auth.json")


if __name__ == "__main__":
    unittest.main()
