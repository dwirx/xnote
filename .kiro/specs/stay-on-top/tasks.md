# Implementation Plan

- [x] 1. Add resource definitions for Stay on Top feature




  - [x] 1.1 Add IDM_VIEW_STAYONTOP constant to resource.h


    - Define `#define IDM_VIEW_STAYONTOP 269` in resource.h
    - _Requirements: 1.1_




  - [ ] 1.2 Add menu item and accelerator to notepad.rc
    - Add "Stay on Top\tCtrl+Shift+T" menu item in View menu after Vim Mode
    - Add accelerator entry for Ctrl+Shift+T


    - _Requirements: 1.1, 2.1_



- [ ] 2. Implement settings persistence for Stay on Top
  - [ ] 2.1 Add global state variable and getter/setter functions in settings.c
    - Add `static BOOL g_bStayOnTop = FALSE`




    - Add `BOOL IsStayOnTopEnabled(void)` function


    - Add `void SetStayOnTop(BOOL bEnabled)` function
    - _Requirements: 3.1, 3.2, 3.3_
  - [ ] 2.2 Modify LoadSettings() to parse stayOnTop from JSON
    - Add `g_bStayOnTop = ParseJsonBool(json, "stayOnTop", FALSE)`


    - _Requirements: 3.2, 3.3_


  - [ ] 2.3 Modify SaveSettings() to write stayOnTop to JSON
    - Add `fprintf(fp, "  \"stayOnTop\": %s,\n", g_bStayOnTop ? "true" : "false")`
    - _Requirements: 3.1_
  - [ ]* 2.4 Write unit tests for settings round-trip
    - **Property 3: Settings round-trip**
    - **Validates: Requirements 3.1, 3.2**



- [ ] 3. Implement Stay on Top toggle functionality in main.c
  - [ ] 3.1 Add function declarations to notepad.h
    - Declare `BOOL IsStayOnTopEnabled(void)` and `void SetStayOnTop(BOOL bEnabled)`
    - _Requirements: 1.1_
  - [ ] 3.2 Implement ToggleStayOnTop() function in main.c
    - Toggle g_bStayOnTop state using SetStayOnTop()
    - Call SetWindowPos with HWND_TOPMOST or HWND_NOTOPMOST
    - Update menu checkmark using CheckMenuItem()
    - Mark session dirty for auto-save
    - _Requirements: 1.1, 1.2, 1.3, 1.4, 1.5_
  - [ ] 3.3 Add WM_COMMAND handler for IDM_VIEW_STAYONTOP
    - Call ToggleStayOnTop(hwnd) when menu item selected
    - _Requirements: 1.1, 2.1_
  - [ ] 3.4 Initialize Stay on Top state in WM_CREATE
    - Load saved state and apply to window
    - Initialize menu checkmark based on saved state
    - _Requirements: 3.2, 1.4, 1.5_
  - [ ]* 3.5 Write unit tests for toggle functionality
    - **Property 1: Toggle inverts state**
    - **Property 2: Menu state synchronization**
    - **Validates: Requirements 1.1, 1.4, 1.5, 2.1, 2.2**

- [ ] 4. Final Checkpoint - Verify implementation
  - Ensure all tests pass, ask the user if questions arise.
