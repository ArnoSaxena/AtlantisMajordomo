from pathlib import Path
import runpy
import sys
import unittest
from unittest.mock import Mock, mock_open, patch


WORKSPACE = Path(__file__).resolve().parents[1]


def load_workflow(generator, *arguments):
    with patch.object(sys, "argv", ["build_multi.py", *arguments]), \
            patch("builtins.open", mock_open(read_data=generator + "\n")), \
            patch("os.path.expanduser", return_value=str(WORKSPACE)), \
            patch("platform.system", return_value="Windows"):
        return runpy.run_path(str(WORKSPACE / "build_multi.py"))


class VisualStudioWorkflowTests(unittest.TestCase):
    def setUp(self):
        self.workflow = load_workflow("Visual Studio 18 2026", "--test")
        self.args = self.workflow["args"]
        self.globals = self.workflow["configure"].__globals__
        self.build_dir = str(WORKSPACE / "build")
        self.execute = Mock()
        self.clean = Mock()
        globals_patch = patch.dict(self.globals, execute_cmd=self.execute)
        globals_patch.start()
        self.addCleanup(globals_patch.stop)

    def configure(self, stale=False):
        with patch.dict(self.globals,
                        is_stale_cmake_cache=Mock(return_value=stale),
                        clean=self.clean), \
                patch("os.makedirs"):
            self.workflow["configure"]()

    def cache_is_stale(self, generator, source=WORKSPACE):
        cache = (
            f"CMAKE_HOME_DIRECTORY:INTERNAL={source}\n"
            f"CMAKE_GENERATOR:INTERNAL={generator}\n"
        )
        with patch("os.path.isfile", return_value=True), \
                patch("builtins.open", mock_open(read_data=cache)):
            return self.workflow["is_stale_cmake_cache"]()

    def test_reading_visual_studio_17_from_configuration(self):
        workflow = load_workflow("Visual Studio 17 2022")
        self.assertEqual(workflow["args"].generator, "Visual Studio 17 2022")

    def test_reading_visual_studio_18_from_configuration(self):
        workflow = load_workflow("Visual Studio 18 2026")
        self.assertEqual(workflow["args"].generator, "Visual Studio 18 2026")

    def test_command_line_overrides_configured_version(self):
        workflow = load_workflow(
            "Visual Studio 17 2022", "-g", "Visual Studio 18 2026"
        )
        self.assertEqual(workflow["args"].generator, "Visual Studio 18 2026")

    def test_configure_passes_visual_studio_17_to_cmake(self):
        self.args.generator = "Visual Studio 17 2022"
        self.configure()
        self.execute.assert_called_once_with(
            ["cmake", "-G", "Visual Studio 17 2022", "-S", str(WORKSPACE),
             "-B", self.build_dir, "-DATLANTIS_BUILD_TESTS=ON"],
            "Configuring CMake (Visual Studio 17 2022)",
        )

    def test_configure_passes_visual_studio_18_to_cmake(self):
        self.configure()
        self.execute.assert_called_once_with(
            ["cmake", "-G", "Visual Studio 18 2026", "-S", str(WORKSPACE),
             "-B", self.build_dir, "-DATLANTIS_BUILD_TESTS=ON"],
            "Configuring CMake (Visual Studio 18 2026)",
        )

    def test_configure_disables_tests_when_not_requested(self):
        self.args.test = False
        self.configure()
        self.execute.assert_called_once_with(
            ["cmake", "-G", "Visual Studio 18 2026", "-S", str(WORKSPACE),
             "-B", self.build_dir, "-DATLANTIS_BUILD_TESTS=OFF"],
            "Configuring CMake (Visual Studio 18 2026)",
        )

    def test_build_selects_debug_configuration(self):
        self.args.build_type = "debug"
        self.workflow["build"]()
        self.execute.assert_called_once_with(
            ["cmake", "--build", self.build_dir, "--config", "Debug"],
            "Building AtlantisMajordomo (debug)",
        )

    def test_build_selects_release_configuration(self):
        self.workflow["build"]()
        self.execute.assert_called_once_with(
            ["cmake", "--build", self.build_dir, "--config", "Release"],
            "Building AtlantisMajordomo (release)",
        )

    def test_catch2_selects_debug_executable(self):
        self.args.build_type = "debug"
        executable = str(WORKSPACE / "build" / "Debug" / "AtlantisMajordomoTests.exe")
        with patch("os.path.isfile", side_effect=lambda path: path == executable), \
                patch("os.path.isdir", return_value=False):
            self.workflow["run_tests"]()
        self.execute.assert_called_once_with(
            [executable, "--durations", "yes"],
            "Running Catch2 tests",
        )

    def test_catch2_selects_release_executable(self):
        executable = str(WORKSPACE / "build" / "Release" / "AtlantisMajordomoTests.exe")
        with patch("os.path.isfile", side_effect=lambda path: path == executable), \
                patch("os.path.isdir", return_value=False):
            self.workflow["run_tests"]()
        self.execute.assert_called_once_with(
            [executable, "--durations", "yes"],
            "Running Catch2 tests",
        )

    def test_catch2_finds_single_configuration_output(self):
        executable = str(WORKSPACE / "build" / "AtlantisMajordomoTests.exe")
        with patch("os.path.isfile", side_effect=lambda path: path == executable), \
                patch("os.path.isdir", return_value=False):
            self.workflow["run_tests"]()
        self.execute.assert_called_once_with(
            [executable, "--durations", "yes"], "Running Catch2 tests",
        )

    def test_catch2_uses_the_linux_executable_name(self):
        executable = str(WORKSPACE / "build" / "AtlantisMajordomoTests")
        with patch.dict(self.globals, IS_WINDOWS=False), \
                patch("os.path.isfile", side_effect=lambda path: path == executable), \
                patch("os.path.isdir", return_value=False):
            self.workflow["run_tests"]()
        self.execute.assert_called_once_with(
            [executable, "--durations", "yes"], "Running Catch2 tests",
        )

    def test_missing_catch2_executable_reports_error_without_starting_command(self):
        with patch("os.path.isfile", return_value=False), \
                patch("os.path.isdir", return_value=False):
            with self.assertRaisesRegex(RuntimeError, "Could not find Catch2 test executable"):
                self.workflow["run_tests"]()
        self.execute.assert_not_called()

    def test_changed_generator_marks_cache_stale(self):
        self.assertTrue(self.cache_is_stale("Visual Studio 17 2022"))

    def test_matching_generator_keeps_cache_valid(self):
        self.assertFalse(self.cache_is_stale("Visual Studio 18 2026"))

    def test_changed_source_marks_cache_stale(self):
        self.assertTrue(self.cache_is_stale("Visual Studio 18 2026", WORKSPACE / "other"))

    def test_configure_cleans_a_stale_cache(self):
        self.configure(stale=True)
        self.clean.assert_called_once_with()

    def test_configure_does_not_clean_a_matching_cache(self):
        self.configure()
        self.clean.assert_not_called()


class CommandExecutionTests(unittest.TestCase):
    def setUp(self):
        self.workflow = load_workflow("Visual Studio 18 2026")
        self.globals = self.workflow["execute_cmd"].__globals__
        self.process = Mock(pid=12345)
        self.process.wait.return_value = 0
        popen_patch = patch("subprocess.Popen", return_value=self.process)
        self.popen = popen_patch.start()
        self.addCleanup(popen_patch.stop)

    def test_windows_commands_start_in_a_separate_process_group(self):
        self.workflow["execute_cmd"](["example"])
        self.popen.assert_called_once_with(
            ["example"], shell=False,
            creationflags=self.globals["subprocess"].CREATE_NEW_PROCESS_GROUP,
        )

    def test_posix_commands_start_in_a_separate_session(self):
        with patch.dict(self.globals, IS_WINDOWS=False):
            self.workflow["execute_cmd"](["example"])
        self.popen.assert_called_once_with(
            ["example"], shell=False, start_new_session=True,
        )

    def test_nonzero_command_exit_is_reported(self):
        self.process.wait.return_value = 7
        with self.assertRaisesRegex(RuntimeError, "return code 7"):
            self.workflow["execute_cmd"](["example"])

    def test_windows_crash_diagnostics_are_preserved(self):
        self.process.wait.return_value = 0xC0000005
        with self.assertRaisesRegex(RuntimeError, "Access violation"):
            self.workflow["execute_cmd"](["example"])

    def test_successful_commands_do_not_stop_the_process_tree(self):
        stop_tree = Mock()
        with patch.dict(self.globals, stop_command_tree=stop_tree):
            self.workflow["execute_cmd"](["example"])
        stop_tree.assert_not_called()

    def test_cancellation_stops_the_owned_process_tree_and_propagates(self):
        self.process.wait.side_effect = KeyboardInterrupt
        stop_tree = Mock()
        with patch.dict(self.globals, stop_command_tree=stop_tree):
            with self.assertRaises(KeyboardInterrupt):
                self.workflow["execute_cmd"](["example"])
        stop_tree.assert_called_once_with(self.process)

    def test_cleanup_failure_is_reported_explicitly(self):
        self.process.wait.side_effect = KeyboardInterrupt
        stop_tree = Mock(side_effect=RuntimeError("cleanup failed"))
        with patch.dict(self.globals, stop_command_tree=stop_tree):
            with self.assertRaisesRegex(RuntimeError, "cleanup failed"):
                self.workflow["execute_cmd"](["example"])

    def test_windows_cleanup_targets_the_owned_pid_and_waits_for_exit(self):
        with patch("subprocess.run", return_value=Mock(returncode=0)) as run:
            self.workflow["stop_command_tree"](self.process)
        command = run.call_args.args[0]
        self.assertIn("Stop-CommandTree 12345", command[-1])
        self.process.wait.assert_called_once_with()

    def test_windows_tree_cleanup_failure_is_reported(self):
        with patch("subprocess.run", return_value=Mock(returncode=1)):
            with self.assertRaisesRegex(RuntimeError, "Could not stop command process tree"):
                self.workflow["stop_command_tree"](self.process)

    def test_posix_cleanup_kills_the_owned_process_group(self):
        with patch.dict(self.globals, IS_WINDOWS=False, signal=Mock(SIGKILL=9)), \
                patch("os.killpg", create=True) as kill_group:
            self.workflow["stop_command_tree"](self.process)
        kill_group.assert_called_once_with(12345, 9)

    def test_posix_cleanup_accepts_an_already_exited_process_group(self):
        with patch.dict(self.globals, IS_WINDOWS=False, signal=Mock(SIGKILL=9)), \
                patch("os.killpg", side_effect=ProcessLookupError, create=True):
            self.workflow["stop_command_tree"](self.process)
        self.process.wait.assert_called_once_with()

    def test_posix_cleanup_surfaces_permission_failures(self):
        with patch.dict(self.globals, IS_WINDOWS=False, signal=Mock(SIGKILL=9)), \
                patch("os.killpg", side_effect=PermissionError, create=True):
            with self.assertRaises(PermissionError):
                self.workflow["stop_command_tree"](self.process)


if __name__ == "__main__":
    unittest.main()
