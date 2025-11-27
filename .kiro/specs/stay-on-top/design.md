# Design Document: Stay on Top Feature

## Overview

Fitur "Stay on Top" memungkinkan jendela XNote untuk tetap berada di atas semua jendela aplikasi lain. Implementasi menggunakan Windows API `SetWindowPos` dengan flag `HWND_TOPMOST` dan `HWND_NOTOPMOST` untuk mengontrol z-order jendela. Fitur ini dapat diaktifkan melalui menu View atau keyboard shortcut Ctrl+Shift+T, dan statusnya akan disimpan dalam file settings JSON.

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                      Main Window (WndProc)                   │
├─────────────────────────────────────────────────────────────┤
│  WM_CREATE: Initialize menu, load settings, apply topmost   │
│  WM_COMMAND: Handle IDM_VIEW_STAYONTOP toggle               │
│  WM_CLOSE: Save settings before exit                        │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    Settings Module                           │
├─────────────────────────────────────────────────────────────┤
│  g_bStayOnTop: Global state variable                        │
│  LoadSettings(): Parse "stayOnTop" from JSON                │
│  SaveSettings(): Write "stayOnTop" to JSON                  │
│  IsStayOnTopEnabled(): Getter function                      │
│  SetStayOnTop(): Setter function                            │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    Windows API                               │
├─────────────────────────────────────────────────────────────┤
│  SetWindowPos(hwnd, HWND_TOPMOST, ...)                      │
│  SetWindowPos(hwnd, HWND_NOTOPMOST, ...)                    │
│  CheckMenuItem(hMenu, IDM_VIEW_STAYONTOP, ...)              │
└─────────────────────────────────────────────────────────────┘
```

## Components and Interfaces

### 1. Resource Definitions (resource.h, notepad.rc)

```c
// New menu command ID
#define IDM_VIEW_STAYONTOP  269

// Menu item in View menu
MENUITEM "&Stay on Top\tCtrl+Shift+T", IDM_VIEW_STAYONTOP

// Accelerator entry
"T", IDM_VIEW_STAYONTOP, VIRTKEY, CONTROL, SHIFT
```

### 2. Settings Module (settings.c)

```c
// Global state variable
static BOOL g_bStayOnTop = FALSE;

// Getter function
BOOL IsStayOnTopEnabled(void);

// Setter function  
void SetStayOnTop(BOOL bEnabled);

// Modified LoadSettings() - add parsing for "stayOnTop"
// Modified SaveSettings() - add writing for "stayOnTop"
```

### 3. Main Window Handler (main.c)

```c
// Toggle function
void ToggleStayOnTop(HWND hwnd);

// Apply topmost state to window
void ApplyStayOnTop(HWND hwnd, BOOL bEnabled);

// WM_CREATE handler - initialize menu checkmark and apply saved state
// WM_COMMAND handler - handle IDM_VIEW_STAYONTOP
```

## Data Models

### Settings JSON Structure

```json
{
  "wordWrap": false,
  "showLineNumbers": true,
  "stayOnTop": false,
  ...
}
```

### State Variables

| Variable | Type | Default | Description |
|----------|------|---------|-------------|
| g_bStayOnTop | BOOL | FALSE | Current stay-on-top state |

## Correctness Properties

*A property is a characteristic or behavior that should hold true across all valid executions of a system-essentially, a formal statement about what the system should do. Properties serve as the bridge between human-readable specifications and machine-verifiable correctness guarantees.*

### Property 1: Toggle inverts state
*For any* initial stay-on-top state (true or false), calling the toggle function SHALL result in the state being inverted (false becomes true, true becomes false).
**Validates: Requirements 1.1, 2.1**

### Property 2: Menu state synchronization
*For any* stay-on-top state, the menu checkmark state SHALL match the internal g_bStayOnTop variable (checked when true, unchecked when false).
**Validates: Requirements 1.4, 1.5, 2.2**

### Property 3: Settings round-trip
*For any* stay-on-top state, saving settings and then loading settings SHALL preserve the original state value.
**Validates: Requirements 3.1, 3.2**

## Error Handling

| Scenario | Handling |
|----------|----------|
| SetWindowPos fails | Log error, continue without topmost (graceful degradation) |
| Settings file missing | Use default value (FALSE) |
| Settings file corrupted | Use default value (FALSE) |
| Invalid JSON value | Use default value (FALSE) |

## Testing Strategy

### Unit Tests
- Test toggle function inverts state correctly
- Test getter/setter functions work correctly
- Test default value when no setting exists

### Property-Based Testing

Library: Tidak diperlukan untuk fitur sederhana ini karena state hanya boolean (2 kemungkinan nilai). Unit tests sudah cukup untuk coverage penuh.

Namun jika diperlukan, property tests dapat menggunakan simple randomization:
- Generate random initial state
- Apply toggle
- Verify state is inverted
- Verify menu checkmark matches state

### Integration Tests
- Test menu click triggers toggle
- Test keyboard shortcut triggers toggle
- Test state persists after restart
