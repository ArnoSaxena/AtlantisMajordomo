#!/usr/bin/env python3
"""
Build script for AtlantisMajordomo

Usage:
    python build.py --help                          # Show all options
    python build.py                                 # Build release
    python build.py --build-type debug              # Build debug
    python build.py --clean                         # Clean build directory
    python build.py --clean-all                     # Remove all artifacts (build, venv, cache)
    python build.py --clean --build-type release    # Clean and build release
    python build.py --run                           # Build and run
"""

import argparse
import os
import re
import shutil
import subprocess
import sys
import platform as platform_module

#
# Global Vars
#

src_working_dir = os.path.dirname(os.path.realpath(__file__))
BUILD_DIR = os.path.join(src_working_dir, "build")
EXECUTABLE_NAME = "AtlantisMajordomo.exe"
FALLBACK_CMAKE_GENERATOR = "Visual Studio 17 2022"


def get_default_cmake_generator():
    """
    Read default CMake generator from user folder file.

    Hardcoded file name: cmake_build_generator.txt
    Falls back to Visual Studio 17 2022 when file is missing/invalid.
    """
    generator_file = os.path.join(os.path.expanduser("~"), "cmake_build_generator.txt")

    try:
        with open(generator_file, "r", encoding="utf-8") as file:
            generator = file.read().strip()
            if generator:
                return generator
    except OSError:
        pass

    return FALLBACK_CMAKE_GENERATOR


DEFAULT_CMAKE_GENERATOR = get_default_cmake_generator()

#
# Argparse
#

parser = argparse.ArgumentParser(
    description='Build the Atlantis Majordomo project',
    epilog='Typical usage: python build.py --build-type release --run'
)

parser.add_argument(
    '--build-type',
    '-t',
    action='store',
    default='release',
    required=False,
    type=str,
    choices=['debug', 'release'],
    help='Build type (debug or release)'
)

parser.add_argument(
    '--clean',
    '-c',
    action='store_true',
    required=False,
    help='Clean build artifacts before building'
)

parser.add_argument(
    '--run',
    '-r',
    action='store_true',
    required=False,
    help='Run the executable after building'
)

parser.add_argument(
    '--generator',
    '-g',
    action='store',
    default=DEFAULT_CMAKE_GENERATOR,
    required=False,
    type=str,
    help='CMake generator to use (default: from ~/cmake_build_generator.txt or Visual Studio 17 2022)'
)

parser.add_argument(
    '--verbose',
    '-v',
    action='store_true',
    required=False,
    default=False,
    help='Verbose output'
)

parser.add_argument(
    '--clean-all',
    action='store_true',
    required=False,
    help='Remove all build artifacts and virtual environment (full cleanup)'
)

args = parser.parse_args()


def describe_windows_return_code(return_code):
    """
    Convert Windows process return codes into more informative diagnostics.
    """
    unsigned_code = return_code & 0xFFFFFFFF

    known_codes = {
        0xC0000005: "Access violation (invalid memory read/write)",
        0xC000001D: "Illegal instruction",
        0xC0000094: "Integer divide by zero",
        0xC00000FD: "Stack overflow",
        0xC0000409: "Stack buffer overrun / fast-fail",
        0xC0000135: "Missing dependency (DLL not found)",
    }

    description = known_codes.get(unsigned_code)
    hex_code = f"0x{unsigned_code:08X}"

    if description:
        return f"{hex_code} ({description})"

    return hex_code

#
# Functions
#


def execute_cmd(cmd, description=""):
    """
    Execute a shell command and handle errors

    Args:
        cmd: Command to execute
        description: Description of what the command does
    """
    if description:
        print(f"\n{'='*60}")
        print(f"[*] {description}")
        print(f"{'='*60}")

    if args.verbose:
        print(f"[CMD] {cmd}\n")

    result = subprocess.run(cmd, shell=True)
    if result.returncode != 0:
        details = f"return code {result.returncode}"
        if platform_module.system() == "Windows":
            details += f" [{describe_windows_return_code(result.returncode)}]"

        raise RuntimeError(f"Command failed with {details}: {cmd}")


def clean():
    """
    Clean build directory
    """
    if os.path.exists(BUILD_DIR):
        print(f"[*] Removing build directory: {BUILD_DIR}")
        shutil.rmtree(BUILD_DIR, ignore_errors=True)
        print("[+] Build directory cleaned")
    else:
        print("[-] Build directory does not exist, nothing to clean")


def clean_all():
    """
    Comprehensive cleanup - remove build directory,
    virtual environment, and cache files
    """
    targets = [
        (BUILD_DIR, "build directory"),
        (os.path.join(src_working_dir, ".venv"), "virtual environment"),
        (os.path.join(src_working_dir, ".cmake"), "CMake cache"),
        (os.path.join(src_working_dir, "__pycache__"), "Python cache"),
    ]

    cleaned_any = False
    for target_path, description in targets:
        if os.path.exists(target_path):
            print(f"[*] Removing {description}: {target_path}")
            if os.path.isdir(target_path):
                shutil.rmtree(target_path, ignore_errors=True)
            else:
                os.remove(target_path)
            print(f"[+] {description} cleaned")
            cleaned_any = True

    if not cleaned_any:
        print("[-] No artifacts found to clean")


def bump_about_version():
    """
    Increment MainWindow::kAboutVersion patch number in MainWindow.hpp.
    """
    header_path = os.path.join(src_working_dir, "include", "GUI", "MainWindow.hpp")
    if not os.path.isfile(header_path):
        raise RuntimeError(f"Version header not found: {header_path}")

    with open(header_path, "r", encoding="utf-8") as file:
        content = file.read()

    version_pattern = re.compile(
        r'(static constexpr wchar_t kAboutVersion\[\] = L")([0-9]+)\.([0-9]+)\.([0-9]+)(";)',
        re.MULTILINE
    )
    match = version_pattern.search(content)
    if match is None:
        raise RuntimeError("Could not locate MainWindow::kAboutVersion in MainWindow.hpp")

    major = int(match.group(2))
    minor = int(match.group(3))
    patch = int(match.group(4)) + 1
    new_version = f"{major}.{minor}.{patch}"

    updated_content = version_pattern.sub(
        rf'\g<1>{new_version}\g<5>',
        content,
        count=1
    )

    with open(header_path, "w", encoding="utf-8", newline="") as file:
        file.write(updated_content)

    print(f"[*] Updated About version to {new_version}")


def configure():
    """
    Configure CMake project
    """
    os.makedirs(BUILD_DIR, exist_ok=True)

    # Visual Studio generators are multi-configuration and don't use CMAKE_BUILD_TYPE
    is_vs_generator = "Visual Studio" in args.generator

    if is_vs_generator:
        cmake_cmd = f'cmake -G "{args.generator}" -S "{src_working_dir}" -B "{BUILD_DIR}"'
    else:
        cmake_cmd = f'cmake -G "{args.generator}" -DCMAKE_BUILD_TYPE={args.build_type.capitalize()} -S "{src_working_dir}" -B "{BUILD_DIR}"'

    execute_cmd(cmake_cmd, f"Configuring CMake ({args.generator})")


def build():
    """
    Build the project
    """
    cmake_cmd = f'cmake --build "{BUILD_DIR}" --config {args.build_type.capitalize()}'
    execute_cmd(cmake_cmd, f"Building AtlantisMajordomo ({args.build_type})")


def find_executable():
    """
    Find the built executable

    Returns:
        Path to the executable or None if not found
    """
    possible_paths = [
        os.path.join(BUILD_DIR, args.build_type.capitalize(), EXECUTABLE_NAME),
        os.path.join(BUILD_DIR, EXECUTABLE_NAME),
        os.path.join(BUILD_DIR, args.build_type.capitalize()),
    ]

    for path in possible_paths:
        if os.path.isfile(path):
            return path
        # Check if it's a directory, look for exe inside
        if os.path.isdir(path):
            exe_path = os.path.join(path, EXECUTABLE_NAME)
            if os.path.isfile(exe_path):
                return exe_path

    return None


def run():
    """
    Run the compiled executable
    """
    exe_path = find_executable()

    if exe_path is None:
        print("[-] ERROR: Could not find executable!")
        print(f"[*] Searched in: {BUILD_DIR}")
        sys.exit(1)

    print(f"[+] Found executable: {exe_path}")
    execute_cmd(f'"{exe_path}"', "Running AtlantisMajordomo")


def main():
    """
    Main entry point
    """
    try:
        # Verify we're on Windows
        if platform_module.system() != "Windows":
            print("[-] ERROR: This script is designed for Windows only")
            sys.exit(1)

        # Handle --clean-all first
        if args.clean_all:
            clean_all()
            sys.exit(0)

        # Check if CMake is available
        try:
            subprocess.run(["cmake", "--version"], capture_output=True, check=True)
        except (subprocess.CalledProcessError, FileNotFoundError):
            print("[-] ERROR: CMake is not installed or not in PATH")
            print("[*] Please install CMake from https://cmake.org/download/")
            sys.exit(1)

        print(f"[*] Working directory: {src_working_dir}")
        print(f"[*] Build type: {args.build_type}")
        print(f"[*] CMake generator: {args.generator}")

        # Clean if requested
        if args.clean:
            clean()

        bump_about_version()

        # Configure and build
        configure()
        build()

        print("\n[+] Build completed successfully!")

        # Run if requested
        if args.run:
            run()

    except RuntimeError as e:
        print(f"\n[-] ERROR: {e}")
        sys.exit(1)
    except KeyboardInterrupt:
        print("\n[-] Build cancelled by user")
        sys.exit(1)
    except Exception as e:
        print(f"\n[-] ERROR: Unexpected error: {e}")
        sys.exit(1)


if __name__ == '__main__':
    main()
