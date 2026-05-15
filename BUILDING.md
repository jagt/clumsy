# Building clumsy

This document describes how to build clumsy from source on Windows.

## Prerequisites

- **Windows 10 or later** (x64 recommended)
- **Visual Studio 2022** (Community, Professional, or Enterprise) with:
  - **"Desktop development with C++" workload** (required; install via Visual Studio Installer if missing)
  - Windows 10 SDK (or later, included with the C++ workload)
- **Git** (to clone the repository)

## Quick Start (VS2022)

### 1. Clone the repository

```powershell
git clone <repo-url> clumsy
cd clumsy
```

### 2. Get GENie (optional)

The Visual Studio solution is already generated and committed to `projects\`. You can skip this step and jump straight to building.

If you modify `genie.lua` or need to regenerate, download GENie:

```powershell
# From the repo root:
powershell -ExecutionPolicy Bypass -File .\tools\get_genie.ps1
```

This downloads `genie.exe` to the `tools\` directory. If the download fails, you can:

- **Build GENie from source**: Clone https://github.com/bkaradzic/GENie and follow its build instructions.
- **Download manually**: Get `genie.exe` from https://github.com/bkaradzic/bx/raw/master/tools/bin/windows/genie.exe and place it in the `tools\` directory.

### 3. Generate the Visual Studio solution

```powershell
# From the repo root:
.\tools\genie.exe vs2022
```

This creates `projects\clumsy.sln` and the associated `.vcxproj` files.

> **Note:** If your `genie.exe` does not support `vs2022`, try `vs2019` instead:
> ```powershell
> .\tools\genie.exe vs2019
> ```
> Visual Studio 2022 can open and upgrade VS2019 solutions automatically.

### 4. Build in Visual Studio

1. Open `projects\clumsy.sln` in Visual Studio 2022.
2. Select the desired configuration:
   - **x64 Debug** — Debug build with console output via `OutputDebugString()`
   - **x64 Release** — Optimized release build
   - **x32 Debug** / **x32 Release** — 32-bit builds
3. Build → Build Solution (Ctrl+Shift+B).

The post-build step copies runtime dependencies to the output directory automatically.

### 5. Output directory

After a successful build, the output directory contains:

```
bin\vs\<Configuration>\<Platform>\
├── clumsy.exe        # Main executable
├── WinDivert.dll     # WinDivert user-mode library
├── WinDivert64.sys   # WinDivert kernel driver (x64)
├── WinDivert32.sys   # WinDivert kernel driver (x86)
├── iup.dll           # IUP GUI library
└── config.txt        # Default filter configuration
```

For example, a Release x64 build outputs to `bin\vs\Release\x64\`.

### 6. Run clumsy

> **Important:** clumsy must be run as **Administrator** because it uses WinDivert to intercept network packets.

Navigate to the output directory and run:

```powershell
cd bin\vs\Release\x64
.\clumsy.exe
```

Or run directly from Visual Studio (ensure VS is running as Administrator, or the self-elevation prompt will appear).

## Building with Zig (alternative)

The project also includes a `build.zig` for building with the Zig compiler. This is maintained independently and is not affected by the Visual Studio build changes.

```powershell
zig build -Darch=x64 -Dconf=Release
```

See `build.zig` for all available build options.

## GENie Actions Reference

| Command | Description |
|---------|-------------|
| `genie.exe vs2022` | Generate Visual Studio 2022 solution |
| `genie.exe vs2019` | Generate Visual Studio 2019 solution |
| `genie.exe vs2017` | Generate Visual Studio 2017 solution |
| `genie.exe gmake` | Generate MinGW makefiles (requires MSYS2 with Clang) |
| `genie.exe clean` | Remove build/ and bin/ directories |

## Troubleshooting

### "genie.exe is not recognized"
Ensure `genie.exe` is in the `tools\` directory. Run the download script:
```powershell
powershell -ExecutionPolicy Bypass -File .\tools\get_genie.ps1
```

### "No such action 'vs2022'"
Your genie.exe version doesn't support VS2022. Either:
- Download a newer genie.exe using the script above.
- Use `genie.exe vs2019` and let VS2022 upgrade the solution.

### Linker error LNK2026 (SAFESEH) on x64
This is fixed in the current `genie.lua` — `/SAFESEH:NO` is only applied to x86 builds.

### Post-build robocopy errors
The post-build step uses robocopy, which returns non-zero exit codes on success. The build script wraps each robocopy call to force a success exit code. If you see robocopy errors in the build output but the build succeeds, this is expected.

### "clumsy.exe has stopped working" on launch
Ensure you are running as Administrator. clumsy requires elevated privileges to intercept network packets.
