# 3D-Pipes-Screensaver-Utah-Teapot
Y'all what if we brought back the Utah Teapot in the Windows 95 3D Pipes screensaver????? :O

This is a modern open-source rebuild of the classic **Windows NT/2000 OpenGL 3D Pipes screensaver**, ported from the original NT SDK source. It includes the famous **Utah Teapot easter egg** that was quietly dropped in the Windows XP rewrite.

---

## Features

- Classic 3D pipes rendered with OpenGL (works on any modern Windows)
- Utah Teapot easter egg — appears randomly at pipe joints
- Settings dialog with controls for:
  - Pipe count (single / multiple)
  - Pipe style (traditional elbow joints / flex)
  - Joint type (elbow, ball, mixed, cycle)
  - Resolution (tessellation quality)
  - Surface style (solid / textured, with custom texture picker)
  - **Teapot Frequency** slider — how often the teapot appears (Common → Rare)
  - **Speed** slider — pipe growth speed (smooth continuous range)
  - **Enable Utah Teapot easter egg** checkbox — turn the teapot on/off entirely
- Smooth animated dissolve transition between pipe resets
- All settings saved to the Windows registry

---

## Building on Linux (cross-compile for Windows with MinGW-w64)

These are the steps used to produce the `.scr` file from source on a Linux machine.

### 1. Install the MinGW-w64 cross-compiler

**Ubuntu / Debian:**
```bash
sudo apt-get install mingw-w64 cmake
```

**Fedora / RHEL:**
```bash
sudo dnf install mingw64-gcc mingw64-gcc-c++ cmake
```

**Arch Linux:**
```bash
sudo pacman -S mingw-w64-gcc cmake
```

### 2. Clone the repository

```bash
git clone https://github.com/serenasligh/3D-Pipes-Screensaver-Utah-Teapot.git
cd 3D-Pipes-Screensaver-Utah-Teapot
```

### 3. Configure and build

```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE=toolchain-mingw64.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The output file is **`build/sspipes.scr`**.

---

## Building on Windows (native, with MSYS2)

### 1. Install MSYS2

Download and install from [msys2.org](https://www.msys2.org/), then open the **MSYS2 MinGW 64-bit** shell.

### 2. Install dependencies

```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake
```

### 3. Clone and build

```bash
git clone https://github.com/serenasligh/3D-Pipes-Screensaver-Utah-Teapot.git
cd 3D-Pipes-Screensaver-Utah-Teapot
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The output file is **`build/sspipes.scr`**.

---

## Installation on Windows

1. Copy **`build/sspipes.scr`** to `C:\Windows\System32\`
2. Right-click the file → **Install**  
   *or* go to **Settings → Personalization → Lock screen → Screen saver** and select **3D Pipes (OpenGL)** from the dropdown.
3. Click **Settings** to open the configuration dialog and adjust the options.

### Keeping your existing texture

If you had a custom texture configured in a previous version, it is stored in the registry at:

```
HKEY_CURRENT_USER\Software\Microsoft\ScreenSavers\Screen Saver.3DPipes
```

This build uses the same registry path, so your texture setting will carry over automatically.

---

## How the Utah Teapot works

The teapot easter egg comes from the original Windows NT 4.0 OpenGL screensaver source (circa 1995). When the pipe mode is set to **Mixed** joint type, there is a small random chance per joint that a Utah Teapot is drawn instead of an elbow or ball joint. This build makes it available for all joint types and adds a slider to control the frequency, along with a checkbox to disable it entirely.

The teapot geometry uses standard **OpenGL evaluators** (`glMap2f` / `glEvalMesh2`) with the original Bézier patch control points — no external dependencies beyond `opengl32.dll`, which ships with every version of Windows.

---

## Credits

- Original source: Windows NT 4.0 SDK OpenGL screen saver samples, Copyright © 1994–1995 Microsoft Corporation
- Utah Teapot geometry: public domain (Martin Newell, 1975)
- This port: open source, see repository history
