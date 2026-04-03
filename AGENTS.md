# Repository Guidelines

## Project Structure & Module Organization

- `src/` contains the Win32 editor implementation, resources, and icons.
- `lib/` vendors Scintilla and Lexilla. Treat it as upstream code unless a deliberate local patch is required.
- `installer/` contains the NSIS installer script and Windows packaging helpers.
- `benchmark/` contains lightweight verification helpers. `benchmark/check_large_file_policy.sh` is the current regression check for large-file mode thresholds and syntax-policy wiring.
- Disposable build outputs include `xnote.exe`, `build/`, `src/*.o`, and installer artifacts. Do not commit them.

## Build, Test, and Development Commands

- Windows release build: `build.bat`
- Windows make flow: `make`, `make run`, `make clean`, `make rebuild`
- WSL cross-compile: `./build-wsl.sh build`
- Minimal WSL build: `make -f Makefile.wsl`
- Strict warning-free verification: `make -f Makefile.wsl clean all CFLAGS='-Wall -Wextra -Werror -O3 -DUNICODE -D_UNICODE'`
- Policy regression check: `bash benchmark/check_large_file_policy.sh`
- Installer build on Windows after `xnote.exe` exists: `installer/make_installer.bat`

## Coding Style & Naming Conventions

- Use 4-space indentation in C source.
- Keep macros uppercase.
- Prefer PascalCase for functions.
- Prefix globals with `g_`.
- Prefer static translation-unit helpers for local logic.
- Keep comments short and only where they add real context.
- Stay Win32 and UNICODE friendly with types like `TCHAR`, `HWND`, and `DWORD`.

## Testing Guidelines

- There is no full automated unit-test suite; combine build verification with manual smoke testing.
- Minimum verification for source changes:
  - `bash benchmark/check_large_file_policy.sh`
  - `make -f Makefile.wsl clean all`
- For warning cleanup or build-system work, also run:
  - `make -f Makefile.wsl clean all CFLAGS='-Wall -Wextra -Werror -O3 -DUNICODE -D_UNICODE'`
- Manual smoke test:
  - Launch `xnote.exe sample.txt`
  - Open multiple tabs
  - Toggle Vim mode
  - Confirm status bar updates and line numbers remain in sync
- Large-file QA:
  - Open a file around `80 MB` and verify partial loading plus `F5`
  - Open a file around `300 MB` and verify read-only preview and responsive UI
  - If thresholds, status text, or file mode behavior change, update dialogs, status-bar text, `README.md`, and `CHANGELOG.md`

## Commit & Pull Request Guidelines

- Use short imperative commit subjects such as `Align large-file policy across open paths`.
- Keep commits focused. Do not mix unrelated cleanup into behavior changes.
- Do not commit build outputs or generated installer binaries.
- PRs should include:
  - motivation
  - user-visible behavior changes
  - exact verification commands run
  - manual test evidence for GUI-affecting changes

## Performance & Safety Notes

- File-loading changes must keep `FileModeType`, dialogs, status-bar text, and QA expectations in sync.
- Binary-file detection should fail closed by default, except where the UI intentionally offers an override.
- Treat `lib/` as external code and document any local deviations that would matter during future upstream syncs.
