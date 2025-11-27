# Implementation Plan

- [x] 1. Fix current save/load issue - ensure data persists on app close







  - [ ] 1.1 Verify StickyNotes_Save is called in WM_CLOSE/WM_DESTROY
    - Check main.c for proper cleanup sequence


    - Ensure save happens before window destruction
    - _Requirements: 1.1_
  - [ ] 1.2 Add StickyNotes_ForceSave function
    - Implement immediate save without timer
    - Call from WM_CLOSE handler

    - _Requirements: 1.1_

  - [ ]* 1.3 Write property test for save-load round trip
    - **Property 1: Save-Load Round Trip**

    - **Validates: Requirements 1.1, 1.2**


- [ ] 2. Implement Export functionality
  - [ ] 2.1 Add resource IDs for export menu
    - Add IDM_FILE_EXPORT_STICKYNOTES to resource.h
    - _Requirements: 2.1_

  - [ ] 2.2 Add Export menu item to File menu in notepad.rc
    - Add "Export Sticky Notes..." menu item
    - _Requirements: 2.1_

  - [x] 2.3 Implement StickyNotes_Export function

    - Show save file dialog
    - Write all notes to JSON file with version and metadata

    - Return success/failure
    - _Requirements: 2.2, 2.3, 2.4_

  - [ ] 2.4 Handle export menu command in main.c WndProc
    - Call StickyNotes_Export with user-selected path
    - Show success/error message
    - _Requirements: 2.1, 2.3_


- [ ] 3. Implement Import functionality
  - [x] 3.1 Add resource IDs for import menu

    - Add IDM_FILE_IMPORT_STICKYNOTES to resource.h
    - _Requirements: 3.1_
  - [ ] 3.2 Add Import menu item to File menu in notepad.rc
    - Add "Import Sticky Notes..." menu item
    - _Requirements: 3.1_
  - [ ] 3.3 Implement StickyNotes_Import function
    - Show open file dialog
    - Parse JSON file
    - Support merge and replace modes
    - Return number of imported notes or -1 on error
    - _Requirements: 3.2, 3.3, 3.4, 3.5, 3.6_
  - [x] 3.4 Create import mode selection dialog


    - Ask user to choose Merge or Replace
    - _Requirements: 3.2_
  - [ ] 3.5 Handle import menu command in main.c WndProc
    - Call StickyNotes_Import with user-selected path and mode
    - Show success/error message
    - _Requirements: 3.1, 3.5_
  - [ ]* 3.6 Write property test for export-import round trip
    - **Property 2: Export-Import Round Trip**
    - **Validates: Requirements 2.2, 3.4**
  - [ ]* 3.7 Write property test for import merge
    - **Property 3: Import Merge Preserves All Notes**
    - **Validates: Requirements 3.3**
  - [ ]* 3.8 Write property test for import replace
    - **Property 4: Import Replace Removes Old Notes**
    - **Validates: Requirements 3.4**

- [ ] 4. Checkpoint - Make sure export/import works
  - Ensure all tests pass, ask the user if questions arise.

- [ ] 5. Implement custom storage location
  - [ ] 5.1 Add config file support
    - Create sticky_notes_config.json structure
    - Implement load/save config functions
    - _Requirements: 4.2_
  - [ ] 5.2 Implement StickyNotes_GetStoragePath function
    - Read from config or return default
    - _Requirements: 4.2_
  - [ ] 5.3 Implement StickyNotes_SetStoragePath function
    - Validate new path is writable
    - Move existing data to new location
    - Update config file
    - _Requirements: 4.4_
  - [ ] 5.4 Implement StickyNotes_ResetStoragePath function
    - Reset to default AppData location
    - Move data back if needed
    - _Requirements: 4.5_
  - [ ]* 5.5 Write property test for storage path change
    - **Property 5: Storage Path Change Moves Data**
    - **Validates: Requirements 4.4**

- [ ] 6. Implement Settings dialog
  - [ ] 6.1 Add dialog resource to notepad.rc
    - Create IDD_STICKYNOTES_SETTINGS dialog
    - Add path display, Browse button, Reset button
    - _Requirements: 4.1_
  - [ ] 6.2 Add resource IDs to resource.h
    - Add IDM_VIEW_STICKYNOTES_SETTINGS
    - Add IDD_STICKYNOTES_SETTINGS and control IDs
    - _Requirements: 4.1_
  - [ ] 6.3 Implement StickyNotes_ShowSettingsDialog function
    - Create dialog procedure
    - Show current storage path
    - Handle Browse and Reset buttons
    - _Requirements: 4.1, 4.2, 4.3, 4.5_
  - [ ] 6.4 Add Settings menu item to View menu
    - Add "Sticky Notes Settings..." menu item
    - _Requirements: 4.1_
  - [ ] 6.5 Handle settings menu command in main.c WndProc
    - Call StickyNotes_ShowSettingsDialog
    - _Requirements: 4.1_

- [ ] 7. Implement save status indicator
  - [ ] 7.1 Add save status callback mechanism
    - Implement SetSaveCallback function
    - Call callback before and after save
    - _Requirements: 5.1, 5.2_
  - [ ] 7.2 Update status bar during save operations
    - Show "Saving..." when save starts
    - Show "Saved" briefly when complete
    - Show error indicator on failure
    - _Requirements: 5.1, 5.2, 5.3_
  - [ ]* 7.3 Write property test for content change sets modified
    - **Property 6: Content Change Sets Modified Flag**
    - **Validates: Requirements 1.3**

- [ ] 8. Final Checkpoint - Make sure all features work
  - Ensure all tests pass, ask the user if questions arise.
