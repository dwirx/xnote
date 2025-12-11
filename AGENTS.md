# Repository Guidelines

## Project Structure & Module Organization
- `src/` holds the Win32 editor code (file_ops, edit_ops, dialogs, syntax, vim_mode, session, performance, sticky_notes, multi_cursor) plus resources (`notepad.rc`, `resource.h`) and icons under `src/icons/`.
- `lib/` vendors Scintilla and Lexilla; treat as read-only unless you intend to sync with upstream.
- `installer/` contains the NSIS script (`xnote.nsi`) and helpers for packaging.
- `benchmark/` includes a small C harness for performance checks; use only when investigating regressions.
- Build artifacts such as `xnote.exe` or `build/` are disposable; avoid committing them.

## Build, Test, and Development Commands
- Windows (MinGW-w64 `gcc` + `windres`): run `build.bat` for a release build that outputs `xnote.exe` in the repo root.
- Makefile flow (Windows shell): `make` (build), `make run` (launch), `make clean` / `make rebuild` (cleanup and rebuild).
- WSL cross-compile: `./build-wsl.sh build` (default), `./build-wsl.sh run` (launch via `cmd.exe`/`explorer.exe`), `make -f Makefile.wsl` is the minimal alternative once MinGW cross-tools are installed.
- Installer: after producing `xnote.exe`, run `installer/make_installer.bat` on Windows to emit the NSIS installer.

## Coding Style & Naming Conventions
- C code uses 4-space indentation, uppercase macros, and PascalCase function names; globals use a `g_` prefix, pointer variables commonly start with `p`.
- Favor static helpers within the translation unit; keep comments brief and purposeful.
- Stick to Win32/UNICODE-friendly types (e.g., `TCHAR`, `HWND`); keep non-ASCII out of source unless required for resources or UI strings.

## Testing Guidelines
- There is no automated test suite; rely on manual validation after each change.
- Smoke test: `xnote.exe sample.txt`, open multiple tabs, toggle Vim mode, confirm status bar updates and line numbers stay in sync.
- Large-file QA (core requirement): open ~80MB to confirm the Partial Loading dialog and F5 “Load More” flow; open ~300MB to see the Read-Only Preview message and confirm the UI stays responsive.
- If you adjust chunk sizes or file mode thresholds, update dialogs/status text and re-run the above scenarios.

## Commit & Pull Request Guidelines
- Use concise, imperative commit subjects (e.g., `Tighten partial load progress updates`); keep scope focused.
- Do not commit build outputs (`xnote.exe`, `*.o`, `build/`, installer binaries).
- Pull requests should describe motivation, summarize functional impact, list manual test evidence (commands and outcomes, screenshots for dialogs), and link any related issue or benchmark result.

## Performance & Safety Notes
- Changes affecting file loading must respect `FileModeType` behavior and associated status messages; keep thresholds consistent across dialogs and the status bar.
- Treat vendor code in `lib/` as external; isolate local fixes and document deviations for future upstream syncs.
