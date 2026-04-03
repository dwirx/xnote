#!/usr/bin/env bash

set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

fail() {
    printf 'FAIL: %s\n' "$1" >&2
    exit 1
}

expect_literal() {
    local pattern="$1"
    local file="$2"
    local message="$3"

    if ! rg -Fq "$pattern" "$file"; then
        fail "$message"
    fi
}

expect_literal "#define THRESHOLD_PARTIAL       (50 * 1024 * 1024)" "src/notepad.h" \
    "THRESHOLD_PARTIAL should keep ~80MB files in partial mode"
expect_literal "#define THRESHOLD_READONLY      (200 * 1024 * 1024)" "src/notepad.h" \
    "THRESHOLD_READONLY should keep ~300MB files in read-only preview"
expect_literal "#define THRESHOLD_MMAP          (1024 * 1024 * 1024)" "src/notepad.h" \
    "THRESHOLD_MMAP should reserve memory-mapped mode for ~1GB+ files"

expect_literal "BOOL ShouldEnableSyntaxHighlighting(DWORD dwFileSize, int nLineCount, FileModeType fileMode);" \
    "src/notepad.h" "Shared syntax-highlighting policy helper is missing"
expect_literal "BOOL ShouldEnableSyntaxHighlighting(DWORD dwFileSize, int nLineCount, FileModeType fileMode) {" \
    "src/file_ops.c" "Shared syntax-highlighting policy helper is not implemented"

for file in src/file_ops.c src/main.c src/settings.c src/session.c; do
    expect_literal "ShouldEnableSyntaxHighlighting(" "$file" \
        "Expected $file to use the shared syntax-highlighting policy"
done

printf 'PASS: large-file policy checks passed\n'
