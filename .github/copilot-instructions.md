# XNote - AI Coding Instructions

## Project Overview

XNote is a Win32-based text editor written in pure C, targeting Windows with MinGW-w64. It uses RichEdit controls for text editing and implements features like Vim mode, syntax highlighting, and session persistence.

## Build Commands

```bash
make              # Build xnote.exe
make run          # Build and run
make clean        # Clean build artifacts
make rebuild      # Clean and rebuild
```

**Requirements**: MinGW-w64 with GCC, windres. Libraries: comctl32, comdlg32, shell32, shlwapi, d2d1, dwrite, ole32.

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

### Module Responsibilities
| File | Purpose |
|------|---------|
| `main.c` | Entry point, WndProc message handler, tab management, window subclassing |
| `file_ops.c` | File I/O, large file handling (chunked loading, memory-mapping) |
| `vim_mode.c` | Vim state machine with 6 modes (Normal, Insert, Visual, Visual Line, Command, Search) |
| `syntax.c` | Syntax highlighting via RichEdit color application |
| `theme.c` | Theme definitions (12 themes including Tokyo Night, Dracula, Nord) |
| `session.c` | JSON-based session persistence to AppData |
| `multi_cursor.c` | Multiple cursor support |
| `line_numbers.c` | Custom line number window with scroll synchronization |

### Key Data Structures

**TabState** (`notepad.h`): Per-tab state including edit control handle, file path, syntax language, file mode for large files, and multi-cursor state.

**VimState** (`vim_mode.h`): Mode enum, repeat count, pending operator, command buffer, yank register.

## Development Patterns

### Adding a New Menu Command
1. Add ID to `resource.h`: `#define IDM_YOUR_CMD 601`
2. Add menu item to `notepad.rc`
3. Handle in `WndProc` WM_COMMAND switch in `main.c`:
```c
case IDM_YOUR_CMD:
    YourFunction(hwnd);
    break;
```

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
Files use different modes based on size thresholds (defined in `notepad.h`):
- `FILEMODE_NORMAL`: Full load (<2MB)
- `FILEMODE_PARTIAL`: Chunked loading (2-10MB)
- `FILEMODE_READONLY`: Preview mode (10-50MB)
- `FILEMODE_MMAP`: Memory-mapped (>50MB)

### Theme Integration
Syntax colors come from current theme via `GetSyntaxColor()`. When adding new syntax elements:
1. Add color type to `SyntaxColorType` enum in `syntax.h`
2. Map to theme color in `GetSyntaxColor()` function

## Key Conventions

- **Unicode**: All strings use `TCHAR`, compile with `-DUNICODE -D_UNICODE`
- **Memory**: Use `HeapAlloc/HeapFree` with `GetProcessHeap()`
- **Error handling**: Use `ShowErrorDialog()` for user-facing errors
- **Tab limits**: Max 32 tabs (`MAX_TABS` in `notepad.h`)
- **Session auto-save**: Every 5 seconds via `TIMER_AUTOSAVE`

## Important Implementation Notes

- Line numbers are rendered in a separate custom window (`LineNumberWndProc`) that must stay synced with edit control scroll position
- RichEdit controls (RICHEDIT50W preferred, fallback to RichEdit20W) are used for better large file support
- Tab control is owner-drawn (`TCS_OWNERDRAWFIXED`) to support close buttons and theme colors
- Vim mode intercepts keys before they reach the edit control via subclass proc
