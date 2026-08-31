# Assetto Corsa Mod Organizer

<div align="center">

![Platform](https://img.shields.io/badge/platform-Windows%2010%20%7C%2011%20(x64)-blue.svg)
![C++](https://img.shields.io/badge/C%2B%2B-20-00599C.svg?logo=cplusplus)
![Qt](https://img.shields.io/badge/Qt-6.8-41CD52.svg?logo=qt)
![Release](https://img.shields.io/badge/version-v1.0.0-informational.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)

**A blazing-fast, modern desktop tool for organizing, auto-detecting, and batch-updating brand metadata and country origins across your Assetto Corsa car mod collection.**

[Download Latest Release (ACBO.exe)](https://github.com/hrubcin/ACModOrganize/releases/latest) • [Features](#key-features) • [Building from Source](#building-from-source)

</div>

---

## 🌟 Key Features

### 🔍 Blazing Fast Asynchronous Scanner
- Scans **1,400+ car mods in under 2 seconds** on modern NVMe / SSD drives.
- Asynchronous multi-threaded worker engine keeps the UI perfectly smooth and responsive (60+ FPS).

### 🧠 Intelligent Brand & Country Detection
- **Folder & Name Pattern Recognition**: Recognizes manufacturer prefixes, clean model lore, and modding team tags.
- **Brand-Country Consistency Verification**: Checks if the vehicle's country matches the manufacturer's true origin and highlights mismatches.
- **Dynamic In-Memory Learning**: Dynamically discovers brand logos (`badge.png`) and inherits badges across cars of the same brand.
- **Auto-Suggestions**: 1-click **Apply All Suggestions** instantly resolves missing brands and countries across your library.

### ⚡ Single Standalone Portable Executable
- **Zero Installation Required**: Download `ACBO.exe` and run it directly. No zip extraction or scattered DLL files needed.
- **Integrated GitHub Auto-Updater**: Checks for new releases via the GitHub API and allows 1-click in-place update and restart.

### 🛡️ Non-Destructive Backup Protection
- Creates a pristine `.bak` file before writing any changes to `ui_car.json`.
- Resilient JSON parser preserves mod author comments, custom tags, and multi-line descriptions with unescaped line breaks.

### 🎨 Fluent Design 2 / WinUI 3 Interface
- Native Windows 11 aesthetics with responsive layouts, fluid animations, and high contrast typography.
- **Grid & Table View**: Switch between visual cards and dense multi-column tables.
- **Dark & Light Mode**: Built-in theme switcher with WCAG AA compliant contrast.

---

## 🚀 Download & Quick Start

1. Download **[`ACBO.exe`](https://github.com/hrubcin/ACModOrganize/releases/latest/download/ACBO.exe)** from the latest release.
2. Double-click `ACBO.exe` to launch.
3. Select your Assetto Corsa `content/cars` directory (automatically detected if installed in standard Steam libraries).
4. Click **Scan Cars** to analyze your library.
5. Review suggestions and click **Apply All Suggestions** followed by **Save All Pending**.

---

## 🛠️ Building from Source

### Prerequisites
- **Windows 10 / 11 (64-bit)**
- **Visual Studio 2022** (with MSVC C++20 toolset)
- **CMake 3.22+**
- **Qt 6.4+** (Core, Gui, Quick, Qml, QuickControls2, Svg, Concurrent, Network)

### 1-Click Build Script
Clone the repository and run:
```cmd
git clone https://github.com/hrubcin/ACModOrganize.git
cd ACModOrganize
build_single_exe.bat
```
The compiled standalone executable will be generated at `dist\ACBO.exe`.

### Manual Build via CMake
```cmd
mkdir build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target ACModOrganize ACBO acbo_tests
build\Release\acbo_tests.exe
```

---

## 🧪 Testing Suite
ACBO includes a comprehensive test suite covering:
- **Brand Detection Engine**: Heuristic aliases, prefix stripping, manufacturer mismatches.
- **JSON Writer & Backup**: Corrupted JSON recovery, unescaped CR/LF descriptions, backup creation.
- **Scanner Engine**: Full directory scanning, multi-threaded worker pipeline, folder caching.
- **Auto-Updater**: Semantic version parsing and comparison.

Run all tests:
```cmd
build\Release\acbo_tests.exe
```

---

## 📄 License
This project is licensed under the [MIT License](LICENSE).
Assetto Corsa is a registered trademark of Kunos Simulazioni S.r.l. This project is an independent community utility.
