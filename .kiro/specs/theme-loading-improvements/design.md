# Design Document: Theme and Loading Improvements

## Overview

Dokumen ini menjelaskan desain teknis untuk perbaikan sistem tema dan loading file di XNote. Perbaikan difokuskan pada:

1. **Konsistensi Tema** - Memastikan semua elemen UI memiliki warna yang cocok dan transisi yang mulus
2. **Tema Terang yang Lebih Baik** - Optimasi warna untuk tema light agar lebih nyaman digunakan
3. **Loading File Tanpa Glitch** - Menghilangkan flicker dan artefak visual saat loading
4. **Progress yang Informatif** - Menampilkan informasi loading yang jelas dan akurat

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        XNote Application                         │
├─────────────────────────────────────────────────────────────────┤
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐  │
│  │   Theme System  │  │  File Loader    │  │   UI Manager    │  │
│  │                 │  │                 │  │                 │  │
│  │ - ThemeColors   │  │ - Background    │  │ - Status Bar    │  │
│  │ - ApplyTheme()  │  │   Thread        │  │ - Progress Dlg  │  │
│  │ - GetContrast() │  │ - Progress      │  │ - Tab Control   │  │
│  │                 │  │   Tracking      │  │                 │  │
│  └────────┬────────┘  └────────┬────────┘  └────────┬────────┘  │
│           │                    │                    │           │
│           └────────────────────┼────────────────────┘           │
│                                │                                │
│  ┌─────────────────────────────┴─────────────────────────────┐  │
│  │                    RichEdit Control                        │  │
│  │  - WM_SETREDRAW control                                    │  │
│  │  - EM_STREAMIN for content                                 │  │
│  │  - EM_SETBKGNDCOLOR, EM_SETCHARFORMAT for colors          │  │
│  └────────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

## Components and Interfaces

### 1. Enhanced Theme System (theme.c/theme.h)

```c
/* New function to apply theme without flicker */
void ApplyThemeToEditNoFlicker(HWND hwndEdit);

/* Calculate contrast ratio between two colors */
float CalculateContrastRatio(COLORREF crFg, COLORREF crBg);

/* Check if theme is a light theme */
BOOL IsLightTheme(ThemeType theme);

/* Get relative luminance of a color */
float GetRelativeLuminance(COLORREF cr);
```

### 2. Improved File Loading (file_ops.c)

```c
/* Enhanced progress tracking structure */
typedef struct {
    DWORD dwFileSize;
    DWORD dwBytesRead;
    DWORD dwProgress;
    DWORD dwLastUpdateTime;
    TCHAR szStatusText[256];
} LoadingProgress;

/* Load file with improved progress and no flicker */
BOOL ReadFileContentNoFlicker(HWND hEdit, const TCHAR* szFileName);

/* Update status bar with loading info */
void UpdateLoadingStatus(HWND hwndStatus, LoadingProgress* pProgress);
```

### 3. UI Manager Improvements

```c
/* Batch UI updates to prevent flicker */
void BeginUIUpdate(HWND hwnd);
void EndUIUpdate(HWND hwnd);

/* Update status bar with file statistics */
void UpdateFileStatistics(HWND hwndStatus, const TCHAR* szFileName, 
                          DWORD dwSize, DWORD dwLines, const TCHAR* szEncoding);
```

## Data Models

### ThemeColors Structure (existing, no changes needed)

```c
typedef struct {
    COLORREF crBackground;      /* Editor background */
    COLORREF crForeground;      /* Default text color */
    COLORREF crLineNumber;      /* Line number color */
    COLORREF crLineNumBg;       /* Line number background */
    COLORREF crCurrentLine;     /* Current line highlight */
    COLORREF crSelection;       /* Selection background */
    COLORREF crCursor;          /* Cursor color */
    /* ... syntax colors ... */
    COLORREF crTabBg;           /* Tab bar background */
    COLORREF crTabActive;       /* Active tab background */
    COLORREF crTabInactive;     /* Inactive tab background */
    COLORREF crTabText;         /* Tab text color */
    COLORREF crStatusBg;        /* Status bar background */
    COLORREF crStatusText;      /* Status bar text */
} ThemeColors;
```

### LoadingState Structure (new)

```c
typedef struct {
    BOOL bLoading;              /* Currently loading */
    BOOL bRedrawDisabled;       /* Redraw is disabled */
    DWORD dwStartTime;          /* Loading start time */
    DWORD dwCursorPos;          /* Saved cursor position */
    DWORD dwScrollPos;          /* Saved scroll position */
    LoadingProgress progress;   /* Progress tracking */
} LoadingState;
```

## Correctness Properties

*A property is a characteristic or behavior that should hold true across all valid executions of a system-essentially, a formal statement about what the system should do. Properties serve as the bridge between human-readable specifications and machine-verifiable correctness guarantees.*

### Property 1: Theme Contrast Ratio
*For any* theme in the theme system, the contrast ratio between foreground (crForeground) and background (crBackground) colors SHALL be at least 4.5:1 as calculated by the WCAG 2.0 formula.
**Validates: Requirements 1.3**

### Property 2: Light Theme Luminance
*For any* light theme (THEME_LIGHT, THEME_TOKYO_NIGHT_LIGHT, THEME_EVERFOREST_LIGHT, THEME_FROSTED_GLASS), the background color SHALL have relative luminance > 0.5 and the foreground color SHALL have relative luminance < 0.5.
**Validates: Requirements 1.4, 2.2**

### Property 3: Syntax Color Contrast on Light Themes
*For any* light theme and any syntax color (comment, keyword, string, number, function, type, operator, preprocessor), the contrast ratio between that syntax color and the theme background SHALL be at least 4.5:1.
**Validates: Requirements 2.3**

### Property 4: Theme Persistence Round Trip
*For any* valid theme selection, saving the session and loading it back SHALL restore the same theme.
**Validates: Requirements 2.4**

### Property 5: Status Bar Contains File Info After Load
*For any* successfully loaded file, the status bar text SHALL contain the file size information.
**Validates: Requirements 5.2, 5.3**

### Property 6: Tab Theme Consistency
*For any* tab switch operation, the visible tab SHALL have theme colors matching the current global theme.
**Validates: Requirements 6.2**

### Property 7: New Tab Inherits Theme
*For any* newly created tab, its edit control SHALL have background and foreground colors matching the current theme.
**Validates: Requirements 6.3**

### Property 8: Lazy Syntax Highlighting
*For any* theme change with multiple open tabs, only the visible tab SHALL have syntax highlighting applied immediately; other tabs SHALL be marked with bNeedsSyntaxRefresh = TRUE.
**Validates: Requirements 7.1, 7.2**

### Property 9: Dirty Tab Refresh on Visibility
*For any* tab that becomes visible and has bNeedsSyntaxRefresh = TRUE, syntax highlighting SHALL be applied and bNeedsSyntaxRefresh SHALL be set to FALSE.
**Validates: Requirements 7.3**

## Error Handling

### Theme Application Errors
- If theme index is out of range, fall back to THEME_TOKYO_NIGHT
- If color application fails, log error and continue with partial theme

### File Loading Errors
- Display clear error message with cause (file not found, access denied, out of memory)
- Clean up any partial state (close handles, free memory)
- Restore UI to pre-loading state

### Progress Dialog Errors
- If dialog creation fails, continue loading without progress display
- If timer fails, use polling-based progress updates

## Testing Strategy

### Unit Testing Framework
- Use existing manual testing approach as per AGENTS.md
- Focus on visual verification of theme consistency
- Test file loading with various file sizes

### Property-Based Testing
- Use **fast-check** library for JavaScript/TypeScript property tests
- Since XNote is a C application, property tests will be implemented as C test functions that verify properties programmatically
- Each property test should run at least 100 iterations with random inputs

### Test Categories

1. **Theme Contrast Tests**
   - Verify all themes meet WCAG contrast requirements
   - Test light theme luminance properties

2. **Loading Tests**
   - Test with files of various sizes (1KB, 100KB, 1MB, 10MB)
   - Verify progress updates occur
   - Verify no flicker (visual inspection)

3. **Integration Tests**
   - Open file, switch theme, verify colors
   - Create multiple tabs, switch between them
   - Save session, reload, verify theme persists

### Manual Testing Checklist
- [ ] Apply each theme and verify all UI elements update
- [ ] Open large file (>2MB) and verify progress dialog
- [ ] Switch tabs rapidly and verify no flicker
- [ ] Change theme with multiple tabs open
- [ ] Restart application and verify theme persists
