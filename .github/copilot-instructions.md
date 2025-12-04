# XNote - AI Coding Instructions

## Project Overview

XNote is a Win32-based text editor written in pure C, targeting Windows with MinGW-w64. It uses RichEdit controls for text editing and implements features like Vim mode, syntax highlighting, sticky notes, and session persistence.

## Build Commands

### Windows (MinGW-w64 native)
```bash
make              # Build xnote.exe
make run          # Build and run
make clean        # Clean build artifacts
make rebuild      # Clean and rebuild
```

### WSL Cross-Compilation
```bash
./build-wsl.sh build        # Build from WSL
./build-wsl.sh install-deps # Install MinGW-w64 (apt/pacman/dnf)
./build-wsl.sh rebuild      # Clean and rebuild
make -f Makefile.wsl        # Alternative: use Makefile.wsl
```

**Requirements**: MinGW-w64 with GCC, windres.
**Libraries**: comctl32, comdlg32, shell32, shlwapi, d2d1, dwrite, psapi, ole32.

## Architecture

### Core Application Flow
```
WinMain (main.c) → Message Loop → WndProc → Component Functions
```

### Global State (defined in main.c)
- `g_AppState` (AppState): Main app state with tabs array, window handles
- `g_VimState` (VimState): Vim mode state machine
- `g_CurrentTheme` (ThemeType): Active theme
- `g_FontSettings` (FontSettings): Global font config
- `g_StickyManager` (StickyNotesManager): Sticky notes state

### Module Responsibilities
| File | Purpose |
|------|---------|
| `main.c` | Entry point, WndProc, tab management, window subclassing |
| `file_ops.c` | File I/O, large file handling (chunked, memory-mapped) |
| `vim_mode.c` | Vim state machine (Normal, Insert, Visual, Visual Line, Command, Search) |
| `syntax.c` | Syntax highlighting via RichEdit color application |
| `theme.c` | 12 themes (Tokyo Night, Dracula, Nord, Gruvbox, etc.) |
| `session.c` | JSON-based session persistence to AppData |
| `sticky_notes.c` | Floating sticky notes with hide/show, persistence |
| `multi_cursor.c` | Multiple cursor support |
| `line_numbers.c` | Custom line number window with scroll sync |
| `json_format.c` | JSON pretty-print and minify |

### Key Data Structures

**TabState** (`notepad.h`): Per-tab state - edit control, file path, syntax language, file mode, multi-cursor, zoom level.

**VimState** (`vim_mode.h`): Mode enum, repeat count, pending operator, command buffer, yank register.

**StickyNote** (`sticky_notes.h`): Window handle, position, color, content, visibility state.

## Development Patterns

### Adding a New Menu Command
1. Add ID to `resource.h`: `#define IDM_YOUR_CMD 601`
2. Add menu item to `notepad.rc` (with accelerator if needed)
3. Handle in `WndProc` WM_COMMAND switch in `main.c`:
```c
case IDM_YOUR_CMD:
    YourFunction(hwnd);
    break;
```
4. Add accelerator to `IDR_ACCEL` table in `notepad.rc` if keyboard shortcut needed

### Adding a Vim Command
1. Handle key in `ProcessVimKey()` in `vim_mode.c`
2. Implement motion/edit function
3. Consider repeat counts (`g_VimState.nRepeatCount`)
4. Update mode state if needed

### Window Subclassing Pattern
Edit controls are subclassed via `EditSubclassProc` to intercept:
- Vim key sequences when vim mode enabled
- Scroll events for line number synchronization
- Custom shortcuts

### Large File Handling
Files use different modes based on size thresholds (in `notepad.h`):
- `FILEMODE_NORMAL`: Full load (<2MB)
- `FILEMODE_PARTIAL`: Chunked loading (2-10MB)
- `FILEMODE_READONLY`: Preview mode (10-50MB)
- `FILEMODE_MMAP`: Memory-mapped (>50MB)

### Theme Integration
Syntax colors come from current theme via `GetSyntaxColor()`. To add new syntax elements:
1. Add color type to `SyntaxColorType` enum in `syntax.h`
2. Map to theme color in `GetSyntaxColor()` function

## Key Conventions

- **Unicode**: All strings use `TCHAR`, compile with `-DUNICODE -D_UNICODE`
- **Memory**: Use `HeapAlloc/HeapFree` with `GetProcessHeap()`
- **Error handling**: Use `ShowErrorDialog()` for user-facing errors
- **Tab limits**: Max 32 tabs (`MAX_TABS` in `notepad.h`)
- **Session auto-save**: Every 5 seconds via `TIMER_AUTOSAVE`
- **Resource IDs**: Menu commands 100-299, dialogs 500-699, controls 400-499

## Important Implementation Notes

- Line numbers rendered in separate custom window (`LineNumberWndProc`) synced with edit scroll
- RichEdit controls (RICHEDIT50W preferred, fallback to RichEdit20W)
- Tab control is owner-drawn (`TCS_OWNERDRAWFIXED`) for close buttons and theming
- Vim mode intercepts keys before edit control via subclass proc
- Sticky notes are independent windows with per-note persistence
- Session stored in `%APPDATA%\XNote\xnote_session.json`
