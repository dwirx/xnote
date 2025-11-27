# Implementation Plan

- [x] 1. Create sticky notes module foundation




  - [ ] 1.1 Create sticky_notes.h header file
    - Define StickyNoteColor enum with 6 colors
    - Define StickyNote struct with all fields (hwnd, position, size, color, content)
    - Define StickyNotesManager struct


    - Declare all public function prototypes
    - _Requirements: 1.1, 1.3, 4.3_
  - [ ] 1.2 Create sticky_notes.c implementation file
    - Implement global StickyNotesManager instance
    - Define color array with RGB values
    - Implement StickyNotes_Init() and StickyNotes_Cleanup()

    - _Requirements: 1.1, 4.3_

  - [ ]* 1.3 Write property test for color array
    - **Property: Color array has at least 6 predefined colors**
    - **Validates: Requirements 4.3**


- [ ] 2. Implement sticky note window creation
  - [ ] 2.1 Register sticky note window class
    - Create RegisterStickyNoteClass() function
    - Set up window procedure, background brush, cursor
    - _Requirements: 1.1_
  - [ ] 2.2 Implement StickyNotes_Create() function
    - Check maximum note limit
    - Calculate position with offset from previous notes
    - Create owned window with default dimensions
    - Create edit control inside note
    - Assign unique sequential ID
    - _Requirements: 1.1, 1.2, 1.4, 8.1, 8.3_
  - [ ]* 2.3 Write property test for note creation
    - **Property 1: Note creation produces valid window with correct dimensions**

    - **Validates: Requirements 1.1**
  - [x]* 2.4 Write property test for positioning offset

    - **Property 2: Note positioning offset**
    - **Validates: Requirements 1.2**
  - [ ]* 2.5 Write property test for unique IDs
    - **Property 8: Unique sequential note IDs**
    - **Validates: Requirements 8.1, 8.3**


- [ ] 3. Implement sticky note window procedure
  - [ ] 3.1 Implement StickyNoteWndProc
    - Handle WM_CREATE for initialization
    - Handle WM_PAINT for custom title bar and background color
    - Handle WM_SIZE with minimum size constraint
    - Handle WM_NCHITTEST for resize borders

    - Handle WM_COMMAND for edit control notifications

    - _Requirements: 2.1, 2.2, 3.1, 3.2, 3.3, 8.2_
  - [ ] 3.2 Implement title bar drawing
    - Draw compact title bar with note number
    - Draw close button (X) on right side

    - Handle close button click
    - _Requirements: 8.1, 8.2, 5.1_
  - [x]* 3.3 Write property test for minimum size constraint

    - **Property 3: Minimum size constraint enforcement**
    - **Validates: Requirements 3.3**


- [ ] 4. Implement note management functions
  - [ ] 4.1 Implement StickyNotes_Close()
    - Destroy note window
    - Remove from notes array
    - Update note count
    - _Requirements: 5.1, 5.2, 5.3_
  - [ ] 4.2 Implement StickyNotes_CloseAll()
    - Close all notes in reverse order
    - Clear storage
    - _Requirements: 5.3_

  - [ ] 4.3 Implement color management functions
    - StickyNotes_SetColor() - update color and repaint
    - StickyNotes_GetColor() - return current color

    - _Requirements: 4.2, 4.4_

  - [ ] 4.4 Implement context menu for color selection
    - Create popup menu with color options
    - Handle WM_RBUTTONUP to show menu
    - Handle menu selection to change color

    - _Requirements: 4.1, 4.2_
  - [ ]* 4.5 Write property test for close operation
    - **Property 5: Close operation removes note**
    - **Validates: Requirements 5.1, 5.2**
  - [x]* 4.6 Write property test for color change

    - **Property 4: Color change updates note state**
    - **Validates: Requirements 4.2, 4.4**

- [ ] 5. Checkpoint - Make sure all tests are passing
  - Ensure all tests pass, ask the user if questions arise.

- [x] 6. Implement persistence

  - [x] 6.1 Implement StickyNotes_Save()

    - Create JSON structure for all notes
    - Write to %APPDATA%/XNote/sticky_notes.json
    - Include id, position, size, color, content, visible

    - _Requirements: 6.1, 3.4_
  - [ ] 6.2 Implement StickyNotes_Load()
    - Read JSON file

    - Parse note data
    - Recreate notes with saved properties
    - Handle missing/corrupted file gracefully
    - _Requirements: 6.2, 6.4_
  - [ ] 6.3 Implement auto-save mechanism
    - Set timer for 5 second delay after modification

    - Mark notes as modified on content change
    - Save on timer expiration
    - _Requirements: 6.3, 2.3_


  - [ ]* 6.4 Write property test for persistence round-trip
    - **Property 6: Persistence round-trip**
    - **Validates: Requirements 6.1, 6.2**



- [x] 7. Implement visibility control


  - [ ] 7.1 Implement StickyNotes_ShowAll() and StickyNotes_HideAll()
    - Show/hide all note windows
    - Track visibility state
    - _Requirements: 7.2, 7.3_
  - [x] 7.2 Hook into main window minimize/restore




    - Handle WM_SIZE in main window for minimize detection


    - Call HideAll on minimize, ShowAll on restore


    - _Requirements: 7.2, 7.3_
  - [ ] 7.3 Handle main window close
    - Save all notes before closing
    - Close all note windows
    - _Requirements: 7.4_
  - [ ]* 7.4 Write property test for visibility
    - **Property 7: Visibility follows parent window**
    - **Validates: Requirements 7.2, 7.3**

- [ ] 8. Integrate with main application
  - [ ] 8.1 Add menu item and accelerator
    - Add "New Sticky Note" to View menu
    - Add Ctrl+Shift+N accelerator
    - _Requirements: 1.1_
  - [ ] 8.2 Update resource.h with new IDs
    - Add IDM_VIEW_STICKYNOTE
    - Add timer ID for auto-save
    - _Requirements: 1.1_
  - [ ] 8.3 Update main.c WndProc
    - Handle IDM_VIEW_STICKYNOTE command
    - Call StickyNotes_Init on startup
    - Call StickyNotes_Load after init
    - Call StickyNotes_Save and Cleanup on exit
    - _Requirements: 1.1, 6.1, 6.2, 7.4_
  - [ ] 8.4 Update notepad.rc
    - Add menu item for sticky notes
    - Add accelerator entry
    - _Requirements: 1.1_

- [ ] 9. Update build system
  - [ ] 9.1 Update Makefile
    - Add sticky_notes.c to source files
    - Add sticky_notes.o to object files
    - _Requirements: N/A (build system)_

- [ ] 10. Final Checkpoint - Make sure all tests are passing
  - Ensure all tests pass, ask the user if questions arise.
