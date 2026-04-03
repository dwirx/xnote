# Changelog

All notable changes to this repository will be documented in this file.

## Unreleased

### Changed

- Updated large-file thresholds to keep files around `80 MB` in partial loading mode and files around `300 MB` in read-only preview mode.
- Centralized syntax-highlighting eligibility so open, restore, recent-file, and startup-file paths follow the same policy.
- Refreshed `README.md` and repository guidance to reflect current build and verification workflows.

### Fixed

- Fixed inconsistent large-file behavior across normal open, recent files, session restore, and startup file loading paths.
- Prevented obvious binary files from being opened as text by default while preserving the explicit Excel raw-text override flow.
- Removed current WSL build warnings and verified the project builds cleanly with `-Wall -Wextra -Werror`.

### Added

- Added `benchmark/check_large_file_policy.sh` to verify large-file mode thresholds and syntax-highlighting policy wiring.
