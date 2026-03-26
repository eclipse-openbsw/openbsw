import importlib.util
import subprocess
import tempfile
import unittest
from pathlib import Path
from unittest import mock


class TestResolveClangTidyTranslationUnits(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        repo_root = Path(__file__).resolve().parents[2]
        module_path = repo_root / ".ci/resolve-clang-tidy-translation-units.py"
        spec = importlib.util.spec_from_file_location(
            "resolve_clang_tidy_translation_units", module_path
        )
        module = importlib.util.module_from_spec(spec)
        assert spec.loader is not None
        spec.loader.exec_module(module)
        cls.module = module

    def test_read_changed_files_classifies_sources_and_headers(self):
        with tempfile.TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            changed = repo_root / "changed.txt"
            changed.write_text("src/main.cpp\ninclude/api.hpp\ndoc/readme.md\n\n")

            sources, headers = self.module.read_changed_files(changed, repo_root)

            self.assertEqual(sources, {(repo_root / "src/main.cpp").resolve(strict=False)})
            self.assertEqual(headers, {(repo_root / "include/api.hpp").resolve(strict=False)})

    def test_build_output_to_source_map_tracks_output_variants(self):
        with tempfile.TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            build_dir = repo_root / "build"
            build_dir.mkdir(parents=True)

            source = (repo_root / "src/main.cpp").resolve(strict=False)
            output = "build/obj/main.cpp.o"
            compile_commands = build_dir / "compile_commands.json"
            compile_commands.write_text(
                "["
                '{"file": "' + str(source) + '", "output": "' + output + '"}'
                "]"
            )

            mapping = self.module.build_output_to_source_map(build_dir, repo_root)

            self.assertEqual(mapping[output], source)
            self.assertEqual(
                mapping[str((repo_root / output).resolve(strict=False))], source
            )
            self.assertEqual(mapping["obj/main.cpp.o"], source)

    def test_build_output_to_source_map_fails_without_compile_commands(self):
        with tempfile.TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            build_dir = repo_root / "build"
            build_dir.mkdir(parents=True)

            with self.assertRaisesRegex(RuntimeError, "Missing .*compile_commands.json"):
                self.module.build_output_to_source_map(build_dir, repo_root)

    def test_resolve_translation_units_expands_header_dependencies(self):
        changed_sources = {Path("/repo/src/direct.cpp")}
        changed_header = Path("/repo/include/common.hpp")

        output_to_source = {
            "obj/direct.o": Path("/repo/src/direct.cpp"),
            "obj/a.o": Path("/repo/src/a.cpp"),
            "obj/b.o": Path("/repo/src/b.cpp"),
        }
        deps_by_target = {
            "obj/direct.o": {Path("/repo/src/direct.cpp")},
            "obj/a.o": {changed_header},
            "obj/b.o": {Path("/repo/include/unrelated.hpp")},
        }

        resolved = self.module.resolve_translation_units(
            changed_sources,
            {changed_header},
            output_to_source,
            deps_by_target,
        )

        self.assertEqual(
            resolved,
            {Path("/repo/src/direct.cpp"), Path("/repo/src/a.cpp")},
        )

    def test_resolve_translation_units_drops_unbuilt_changed_sources(self):
        resolved = self.module.resolve_translation_units(
            {Path("/repo/src/direct.cpp")},
            set(),
            {"obj/a.o": Path("/repo/src/a.cpp")},
            {"obj/a.o": {Path("/repo/src/a.cpp")}},
        )

        self.assertEqual(resolved, set())

    def test_parse_ninja_deps_parses_targets_and_dependencies(self):
        with tempfile.TemporaryDirectory() as tmp:
            build_dir = Path(tmp) / "build"
            dep1 = str((Path(tmp) / "include/a.hpp").resolve(strict=False))
            dep2 = str((Path(tmp) / "src/a.cpp").resolve(strict=False))
            dep3 = str((Path(tmp) / "include/b.hpp").resolve(strict=False))

            ninja_stdout = (
                "obj/a.o: #deps 2, deps mtime 123456\n"
                f"  {dep1}\n"
                f"  {dep2}\n"
                "obj/b.o: #deps 1, deps mtime 123456\n"
                f"  {dep3}\n"
            )

            completed = subprocess.CompletedProcess(
                args=[],
                returncode=0,
                stdout=ninja_stdout,
            )

            with mock.patch.object(self.module.subprocess, "run", return_value=completed) as run_mock:
                deps = self.module.parse_ninja_deps(build_dir)

            run_mock.assert_called_once_with(
                ["ninja", "-C", str(build_dir), "-t", "deps"],
                check=True,
                capture_output=True,
                text=True,
            )

            self.assertEqual(
                deps,
                {
                    "obj/a.o": {
                        Path(dep1).resolve(strict=False),
                        Path(dep2).resolve(strict=False),
                    },
                    "obj/b.o": {Path(dep3).resolve(strict=False)},
                },
            )

    def test_parse_ninja_deps_resolves_relative_dependencies_from_build_directory(self):
        with tempfile.TemporaryDirectory() as tmp:
            build_dir = Path(tmp) / "build"
            ninja_stdout = "obj/a.o: #deps 1, deps mtime 123456\n  ../include/a.hpp\n"
            completed = subprocess.CompletedProcess(args=[], returncode=0, stdout=ninja_stdout)

            with mock.patch.object(self.module.subprocess, "run", return_value=completed):
                deps = self.module.parse_ninja_deps(build_dir)

            self.assertEqual(
                deps,
                {
                    "obj/a.o": {
                        (build_dir / "../include/a.hpp").resolve(strict=False),
                    }
                },
            )

    def test_write_sources_uses_repo_relative_sorted_paths(self):
        with tempfile.TemporaryDirectory() as tmp:
            repo_root = Path(tmp).resolve(strict=False)
            output_file = repo_root / "out.txt"
            sources = {
                repo_root / "src/b.cpp",
                repo_root / "src/a.cpp",
                Path("/outside/repo/ignored.cpp"),
            }

            self.module.write_sources(output_file, repo_root, sources)

            self.assertEqual(output_file.read_text(), "src/a.cpp\nsrc/b.cpp\n")

    def test_write_sources_creates_parent_directory(self):
        with tempfile.TemporaryDirectory() as tmp:
            repo_root = Path(tmp).resolve(strict=False)
            output_file = repo_root / "nested" / "clang-tidy" / "out.txt"

            self.module.write_sources(output_file, repo_root, {repo_root / "src/a.cpp"})

            self.assertEqual(output_file.read_text(), "src/a.cpp\n")

    def test_main_returns_early_when_no_relevant_changes(self):
        with tempfile.TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            changed = repo_root / "changed.txt"
            changed.write_text("README.md\n")
            output_file = repo_root / "out.txt"

            ret = self.module.main(
                [
                    "--build-directory",
                    str(repo_root / "build"),
                    "--changed-files",
                    str(changed),
                    "--output-file",
                    str(output_file),
                ]
            )

            self.assertEqual(ret, 0)
            self.assertEqual(output_file.read_text(), "")

    def test_main_fails_for_header_changes_without_compile_commands(self):
        with tempfile.TemporaryDirectory() as tmp:
            repo_root = Path(self.module.get_repo_root())
            changed = Path(tmp) / "changed.txt"
            output_file = Path(tmp) / "out.txt"
            build_dir = Path(tmp) / "build"
            build_dir.mkdir(parents=True)

            header_path = repo_root / "include" / "api.hpp"
            changed.write_text(f"{header_path.relative_to(repo_root)}\n")

            ret = self.module.main(
                [
                    "--build-directory",
                    str(build_dir),
                    "--changed-files",
                    str(changed),
                    "--output-file",
                    str(output_file),
                ]
            )

            self.assertEqual(ret, 1)


if __name__ == "__main__":
    unittest.main()
