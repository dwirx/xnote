# XNote

XNote is a lightweight Win32 text editor for Windows focused on fast startup, simple editing, and practical large-file handling. It includes tabs, Vim mode, syntax highlighting, drag and drop, sticky notes, session restore, and WSL-friendly cross-compilation.

## Highlights

- Multiple tabs with recent files and session restore
- Vim mode with normal, insert, visual, command, and search flows
- Syntax highlighting for 20+ languages
- Line numbers, relative line numbers, zoom, word wrap, and status bar details
- Drag and drop file opening
- Sticky notes support
- Large-file protection with partial loading and read-only preview modes
- Binary-file preflight checks to avoid opening obvious non-text files as text

## Large File Behavior

XNote intentionally changes behavior as file size increases to keep the UI responsive:

- `< 50 MB`: normal editing mode
- `50 MB - 200 MB`: partial loading mode, initial chunk loaded first, `F5` loads more
- `200 MB - 1 GB`: read-only preview mode
- `> 1 GB`: memory-mapped read-only preview

Syntax highlighting is also disabled automatically for large or high-line-count documents to avoid freezes.

## Supported Languages

C, C++, Java, JavaScript, TypeScript, Python, Go, Rust, HTML, CSS, JSON, XML, YAML, SQL, PHP, Ruby, Shell, Batch, PowerShell, Markdown

## Build

### Windows

Requires MinGW-w64 with `gcc` and `windres`.

```bat
build.bat
```

Alternative Make targets in a Windows shell:

```bash
make
make clean
make rebuild
make run
```

### WSL Cross-Compile

Install MinGW-w64 first, then build:

```bash
./build-wsl.sh build
```

Minimal alternative:

```bash
make -f Makefile.wsl
```

To run from WSL through Windows interop:

```bash
./build-wsl.sh run
```

### Warning-Free Verification

For a stricter build that treats warnings as errors:

```bash
make -f Makefile.wsl clean all CFLAGS='-Wall -Wextra -Werror -O3 -DUNICODE -D_UNICODE'
```

## Installer

After producing `xnote.exe`, build the NSIS installer on Windows:

```bat
installer\make_installer.bat
```

## Manual Verification

There is no full automated test suite yet. Recommended checks:

```bash
bash benchmark/check_large_file_policy.sh
make -f Makefile.wsl clean all
```

Manual smoke test:

- Launch `xnote.exe sample.txt`
- Open multiple tabs
- Toggle Vim mode
- Confirm status bar and line numbers stay in sync

Large-file QA:

- Open a file around `80 MB` and verify partial loading plus `F5`
- Open a file around `300 MB` and verify read-only preview plus responsive UI

## Keyboard Shortcuts

### File

| Shortcut | Action |
|----------|--------|
| `Ctrl+N` | New document |
| `Ctrl+T` | New tab |
| `Ctrl+O` | Open file |
| `Ctrl+S` | Save |
| `Ctrl+Shift+S` | Save As |
| `Ctrl+W` | Close tab |

### Navigation

| Shortcut | Action |
|----------|--------|
| `Ctrl+Tab` | Next tab |
| `Ctrl+Shift+Tab` | Previous tab |
| `Ctrl+PageDown` | Next tab |
| `Ctrl+PageUp` | Previous tab |
| `Ctrl+1-9` | Jump to tab |
| `Ctrl+G` | Go to line |
| `F3` | Find next |

### Editing

| Shortcut | Action |
|----------|--------|
| `Ctrl+Z` | Undo |
| `Ctrl+X` | Cut |
| `Ctrl+C` | Copy |
| `Ctrl+V` | Paste |
| `Ctrl+A` | Select all |
| `Ctrl+D` | Duplicate line |
| `Ctrl+Shift+K` | Delete line |
| `Alt+Up` | Move line up |
| `Alt+Down` | Move line down |
| `Ctrl+/` | Toggle comment |
| `Tab` | Indent |
| `Shift+Tab` | Unindent |

### View

| Shortcut | Action |
|----------|--------|
| `Ctrl+L` | Toggle line numbers |
| `Ctrl++` | Zoom in |
| `Ctrl+-` | Zoom out |
| `Ctrl+0` | Reset zoom |
| `Alt+Z` | Toggle word wrap |
| `Ctrl+Shift+V` | Toggle Vim mode |
| `F1` | Help |
| `F5` | Load more content in partial mode |

## Vim Mode

Enable Vim mode from the View menu or with `Ctrl+Shift+V`.

Common commands:

- `:w` save
- `:q` close tab if clean
- `:q!` force close
- `:wq` or `:x` save and close
- `:e` open file
- `:tabn` and `:tabp` switch tabs
- `:set nu`, `:set nonu`, `:set rnu`
- `/pattern`, `?pattern`, `n`, `N`

Common motions:

- `h j k l`
- `w b e`
- `0 $ ^`
- `gg G`
- `f<char>` `F<char>` `t<char>` `T<char>`
- `zz` `zt` `zb`

Common edits:

- `dd` delete line
- `yy` yank line
- `p` and `P` paste
- `dw` delete word
- `cw` change word
- `>>` and `<<` indent or unindent

## Repository Notes

- Vendor code in `lib/` should be treated as external
- Build artifacts like `xnote.exe`, `build/`, and installer binaries should not be committed
- See `AGENTS.md` for repository-specific development guidance

## License

MIT License
