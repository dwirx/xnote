# Implementation Plan

- [x] 1. Add edit control subclass for Vim key handling




  - [ ] 1.1 Create subclass procedure `StickyEditSubclassProc` in sticky_notes.c
    - Add function to intercept WM_KEYDOWN, WM_CHAR, WM_SYSKEYDOWN messages
    - Call `ProcessVimKey` when Vim mode is enabled


    - Return to default behavior if ProcessVimKey returns FALSE
    - _Requirements: 1.1, 1.2_


  - [x] 1.2 Apply subclass to edit control in WM_CREATE




    - Use `SetWindowSubclass` to install the subclass procedure
    - Store note index in subclass data for reference
    - _Requirements: 1.1_


  - [ ] 1.3 Remove subclass in WM_DESTROY
    - Use `RemoveWindowSubclass` to clean up
    - _Requirements: 1.1_

- [ ] 2. Update title bar to show Vim mode indicator
  - [x] 2.1 Modify WM_PAINT handler to include Vim mode in title


    - Check `IsVimModeEnabled()` before formatting title
    - Use `GetVimModeString()` to get current mode text
    - Format: "📝 Note X [MODE]" when Vim enabled, "📝 Note X" when disabled
    - _Requirements: 2.1, 2.3_
  - [ ] 2.2 Add title bar refresh on Vim mode change
    - Call `InvalidateRect` on sticky note window when mode changes
    - Trigger refresh from subclass proc after ProcessVimKey handles input
    - _Requirements: 2.2_


- [ ] 3. Include vim_mode.h in sticky_notes.c
  - Add `#include "vim_mode.h"` to access Vim functions
  - _Requirements: 1.1_

- [ ] 4. Checkpoint - Manual testing
  - Ensure Vim navigation works in sticky notes (h, j, k, l)
  - Ensure mode transitions work (i, Escape)
  - Ensure title bar shows correct mode
  - Ensure normal input works when Vim is disabled
