#!/usr/bin/env python3
import subprocess
import sys
import shutil
import os
import glob

def run(cmd):
    print(f"[+] Running: {' '.join(cmd)}")
    result = subprocess.run(cmd)
    if result.returncode != 0:
        print("[!] Command failed")
        sys.exit(result.returncode)

def check_cmd(cmd):
    return shutil.which(cmd) is not None

def detect_package_manager():
    if check_cmd("apt"):
        return "apt"
    elif check_cmd("dnf"):
        return "dnf"
    elif check_cmd("pacman"):
        return "pacman"
    else:
        return None

def install_apt():
    run(["sudo", "apt", "update"])

    packages = [
        "build-essential",
        "cmake",
        "ninja-build",

        # Qt6 for your CMakeLists.txt: find_package(Qt6 REQUIRED COMPONENTS Widgets)
        "qt6-base-dev",
        "qt6-base-dev-tools",
        "qt6-widgets-dev",
    ]

    # Keep your original optional tool packages
    packages += [
        "qt6-tools-dev",
        "qt6-tools-dev-tools",
    ]

    run(["sudo", "apt", "install", "-y"] + packages)

def install_dnf():
    packages = [
        "gcc",
        "gcc-c++",
        "cmake",
        "ninja-build",

        # Qt6 Widgets dev for: find_package(Qt6 REQUIRED COMPONENTS Widgets)
        "qt6-qtbase-devel",
        "qt6-qtwidgets-devel",

        # (your original script had qt6-qttools-devel; keep it)
        "qt6-qttools-devel",
    ]

    run(["sudo", "dnf", "install", "-y"] + packages)

def install_pacman():
    packages = [
        "base-devel",
        "cmake",
        "ninja",

        # Qt6 Widgets dev for: find_package(Qt6 REQUIRED COMPONENTS Widgets)
        "qt6-base",
        "qt6-widgets",

        # (your original script had qt6-tools; keep it)
        "qt6-tools",
    ]

    run(["sudo", "pacman", "-Syu", "--noconfirm"] + packages)

def setup_generator_file():
    path = os.path.expanduser("~/cmake_build_generator.txt")
    print(f"[+] Writing generator preference: {path}")
    with open(path, "w") as f:
        f.write("Ninja")

def find_qt6config_dir():
    # Try to find a directory that contains Qt6Config.cmake
    candidates = [
        "/usr/lib/x86_64-linux-gnu/cmake/Qt6",
        "/usr/lib/cmake/Qt6",
        "/usr/lib64/cmake/Qt6",
        "/usr/share/cmake/Qt6",
    ]

    for base in candidates:
        cfg = os.path.join(base, "Qt6Config.cmake")
        if os.path.exists(cfg):
            return base

    # As a fallback, search under /usr and /opt (bounded)
    for prefix in ["/usr", "/opt"]:
        for d in glob.glob(os.path.join(prefix, "**", "cmake", "Qt6"), recursive=True):
            cfg = os.path.join(d, "Qt6Config.cmake")
            if os.path.exists(cfg):
                return d

    return None

def verify():
    print("\n[+] Verifying installation...")

    tools = ["cmake", "ninja", "g++"]
    for t in tools:
        if check_cmd(t):
            print(f"  ✔ {t} found")
        else:
            print(f"  ✖ {t} NOT found")

    print("\n[+] Checking Qt installation...")
    qt_paths = [
        "/usr/lib/qt6",
        "/usr/lib64/qt6",
        "/usr/include/qt6"
    ]

    found = any(os.path.exists(p) for p in qt_paths)
    if found:
        print("  ✔ Qt6 seems installed")
    else:
        print("  ⚠ Qt6 not clearly detected (may still work via CMake)")

    print("\n[+] Checking for Qt6Config.cmake and Widgets support...")
    qt6_dir = find_qt6config_dir()
    if qt6_dir:
        cfg = os.path.join(qt6_dir, "Qt6Config.cmake")
        print(f"  ✔ Found: {cfg}")

        # Heuristic confirmation for Widgets component config
        widgets_markers = [
            os.path.join(qt6_dir, "WidgetsConfig.cmake"),
            os.path.join(qt6_dir, "Qt6WidgetsConfig.cmake"),
        ]
        if any(os.path.exists(p) for p in widgets_markers):
            print("  ✔ Qt6 Widgets config seems present")
        else:
            print("  ⚠ Qt6Config.cmake found, but Widgets config file names weren't detected in common locations.")
    else:
        print("  ✖ Qt6Config.cmake not found in common locations.")
        print("    You may need to set Qt6_DIR to the directory containing Qt6Config.cmake.")

def main():
    print("=== Qt + Ninja Setup Script ===\n")

    pm = detect_package_manager()
    if not pm:
        print("[!] Could not detect supported package manager (apt/dnf/pacman)")
        sys.exit(1)

    print(f"[+] Detected package manager: {pm}\n")

    if pm == "apt":
        install_apt()
    elif pm == "dnf":
        install_dnf()
    elif pm == "pacman":
        install_pacman()

    setup_generator_file()
    verify()

    print("\n[✓] Setup complete!")
    print("You can now build your project with:")
    print("    python build_multi.py\n")

if __name__ == "__main__":
    main()
