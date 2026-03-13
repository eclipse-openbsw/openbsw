import importlib.util
import subprocess
import tempfile
import unittest
from pathlib import Path
from unittest import mock


class TestDetermineClangTidyScope(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        repo_root = Path(__file__).resolve().parents[2]
        module_path = repo_root / ".ci/determine-clang-tidy-scope.py"
        spec = importlib.util.spec_from_file_location(
            "determine_clang_tidy_scope", module_path
        )
        module = importlib.util.module_from_spec(spec)
        assert spec.loader is not None
        spec.loader.exec_module(module)
        cls.module = module

    def test_load_event_fails_for_missing_file(self):
        with self.assertRaisesRegex(RuntimeError, "GitHub event payload file not found"):
            self.module.load_event(Path("/does/not/exist.json"))

    def test_determine_refs_rejects_unsupported_event(self):
        with self.assertRaisesRegex(RuntimeError, "Unsupported GitHub event"):
            self.module.determine_refs("workflow_dispatch", "head", {})

    def test_determine_refs_requires_pull_request_base_sha(self):
        with self.assertRaisesRegex(RuntimeError, "Unable to determine base SHA"):
            self.module.determine_refs("pull_request", "head", {})

    def test_run_git_command_wraps_missing_git(self):
        with mock.patch.object(self.module.subprocess, "run", side_effect=FileNotFoundError):
            with self.assertRaisesRegex(RuntimeError, "'git' executable is not available"):
                self.module.run_git_command(["status"])

    def test_list_changed_sources_uses_diff_filter_and_suffix_filter(self):
        outputs = {
            ("merge-base", "head", "base"): "merge-base-sha\n",
            ("diff", "--name-only", "--diff-filter=ACMR", "merge-base-sha...head"):
                "src/main.cpp\nsrc/removed.cpp\ndoc/readme.md\ninclude/api.hpp\n",
        }

        def fake_run_git_command(args):
            return outputs[tuple(args)]

        with mock.patch.object(self.module, "run_git_command", side_effect=fake_run_git_command):
            changed = self.module.list_changed_sources("base", "head")

        self.assertEqual(changed, ["include/api.hpp", "src/main.cpp", "src/removed.cpp"])

    def test_list_changed_sources_falls_back_when_merge_base_fails(self):
        def fake_run_git_command(args):
            if args == ["merge-base", "head", "base"]:
                raise RuntimeError("Git command failed: git merge-base head base: missing commit")
            if args == ["diff", "--name-only", "--diff-filter=ACMR", "base..head"]:
                return "src/main.cpp\n"
            raise AssertionError(f"Unexpected git args: {args}")

        with mock.patch.object(self.module, "run_git_command", side_effect=fake_run_git_command):
            changed = self.module.list_changed_sources("base", "head")

        self.assertEqual(changed, ["src/main.cpp"])

    def test_list_changed_sources_raises_for_non_merge_base_git_failure(self):
        def fake_run_git_command(args):
            if args == ["merge-base", "head", "base"]:
                return "merge-base-sha\n"
            raise RuntimeError("Git command failed: git diff --name-only --diff-filter=ACMR merge-base-sha...head: bad revision")

        with mock.patch.object(self.module, "run_git_command", side_effect=fake_run_git_command):
            with self.assertRaisesRegex(RuntimeError, "bad revision"):
                self.module.list_changed_sources("base", "head")

    def test_write_changed_files_creates_parent_directory(self):
        with tempfile.TemporaryDirectory() as tmp:
            output_file = Path(tmp) / "nested" / "scope" / "files.txt"

            self.module.write_changed_files(output_file, ["src/main.cpp"])

            self.assertEqual(output_file.read_text(), "src/main.cpp\n")

    def test_main_main_branch_push_sets_full_mode(self):
        with tempfile.TemporaryDirectory() as tmp:
            output_file = Path(tmp) / "nested" / "files.txt"
            github_output = Path(tmp) / "nested" / "github-output.txt"
            event_path = Path(tmp) / "event.json"
            event_path.write_text("{}")

            ret = self.module.main(
                [
                    "--event-name",
                    "push",
                    "--ref-name",
                    "main",
                    "--sha",
                    "head",
                    "--event-path",
                    str(event_path),
                    "--output-file",
                    str(output_file),
                    "--github-output",
                    str(github_output),
                ]
            )

            self.assertEqual(ret, 0)
            self.assertEqual(output_file.read_text(), "")
            self.assertEqual(github_output.read_text(), "mode=full\nhas_changes=true\n")

    def test_main_returns_error_for_invalid_event_payload(self):
        with tempfile.TemporaryDirectory() as tmp:
            output_file = Path(tmp) / "files.txt"
            github_output = Path(tmp) / "github-output.txt"
            event_path = Path(tmp) / "event.json"
            event_path.write_text("not json")

            ret = self.module.main(
                [
                    "--event-name",
                    "pull_request",
                    "--ref-name",
                    "feature",
                    "--sha",
                    "head",
                    "--event-path",
                    str(event_path),
                    "--output-file",
                    str(output_file),
                    "--github-output",
                    str(github_output),
                ]
            )

            self.assertEqual(ret, 1)


if __name__ == "__main__":
    unittest.main()
