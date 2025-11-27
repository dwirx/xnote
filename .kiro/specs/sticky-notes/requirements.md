# Requirements Document

## Introduction

Fitur Sticky Notes memungkinkan pengguna untuk membuat catatan kecil yang mengambang (floating notes) di atas jendela utama XNote. Sticky notes ini berguna untuk menyimpan catatan sementara, referensi cepat, atau informasi penting yang perlu selalu terlihat saat bekerja dengan dokumen utama. Setiap sticky note dapat dipindahkan, diubah ukurannya, dan memiliki warna berbeda untuk organisasi visual.

## Glossary

- **Sticky Note**: Jendela catatan kecil yang mengambang (floating) di atas jendela utama aplikasi
- **XNote**: Aplikasi notepad utama yang menjadi host untuk sticky notes
- **Floating Window**: Jendela yang selalu berada di atas jendela parent-nya
- **Note Color**: Warna latar belakang sticky note untuk identifikasi visual
- **Note Persistence**: Penyimpanan otomatis sticky notes ke file konfigurasi

## Requirements

### Requirement 1

**User Story:** As a user, I want to create new sticky notes, so that I can capture quick thoughts and references while working on my main document.

#### Acceptance Criteria

1. WHEN a user selects "New Sticky Note" from the View menu or presses Ctrl+Shift+N THEN the XNote application SHALL create a new floating sticky note window with default size 200x150 pixels
2. WHEN a new sticky note is created THEN the XNote application SHALL position the sticky note at a visible location on screen with slight offset from previous notes
3. WHEN a sticky note is created THEN the XNote application SHALL assign a default yellow background color to the note
4. WHEN the maximum number of sticky notes (10) is reached THEN the XNote application SHALL display an error message and prevent creation of additional notes

### Requirement 2

**User Story:** As a user, I want to edit text in sticky notes, so that I can write and modify my quick notes.

#### Acceptance Criteria

1. WHEN a user clicks inside a sticky note THEN the XNote application SHALL activate the text editing area and allow text input
2. WHEN a user types in a sticky note THEN the XNote application SHALL display the typed characters in the note's text area
3. WHEN a user modifies sticky note content THEN the XNote application SHALL mark the note as modified for auto-save
4. WHEN a sticky note contains text THEN the XNote application SHALL support basic text selection, copy, cut, and paste operations

### Requirement 3

**User Story:** As a user, I want to move and resize sticky notes, so that I can organize them on my screen.

#### Acceptance Criteria

1. WHEN a user drags the title bar of a sticky note THEN the XNote application SHALL move the note to the new position
2. WHEN a user drags the edges or corners of a sticky note THEN the XNote application SHALL resize the note accordingly
3. WHEN a sticky note is resized below minimum size (100x80 pixels) THEN the XNote application SHALL enforce the minimum size constraint
4. WHEN a sticky note position or size changes THEN the XNote application SHALL save the new geometry for persistence

### Requirement 4

**User Story:** As a user, I want to change the color of sticky notes, so that I can visually categorize and organize my notes.

#### Acceptance Criteria

1. WHEN a user right-clicks on a sticky note THEN the XNote application SHALL display a context menu with color options
2. WHEN a user selects a color from the context menu THEN the XNote application SHALL change the sticky note's background color immediately
3. WHEN a color is changed THEN the XNote application SHALL provide at least 6 predefined colors (yellow, pink, blue, green, orange, purple)
4. WHEN a sticky note color is changed THEN the XNote application SHALL save the color preference for persistence

### Requirement 5

**User Story:** As a user, I want to close sticky notes, so that I can remove notes I no longer need.

#### Acceptance Criteria

1. WHEN a user clicks the close button (X) on a sticky note THEN the XNote application SHALL close and remove the sticky note
2. WHEN a sticky note is closed THEN the XNote application SHALL remove the note from the saved notes list
3. WHEN all sticky notes are closed THEN the XNote application SHALL clear the sticky notes storage

### Requirement 6

**User Story:** As a user, I want my sticky notes to persist across sessions, so that I don't lose my notes when I close and reopen XNote.

#### Acceptance Criteria

1. WHEN the XNote application exits THEN the XNote application SHALL save all sticky notes content, positions, sizes, and colors to a configuration file
2. WHEN the XNote application starts THEN the XNote application SHALL restore all previously saved sticky notes with their content, positions, sizes, and colors
3. WHEN a sticky note is modified THEN the XNote application SHALL auto-save the changes within 5 seconds
4. WHEN the configuration file is corrupted or missing THEN the XNote application SHALL start with no sticky notes and create a new configuration file

### Requirement 7

**User Story:** As a user, I want sticky notes to stay on top of the main window, so that I can always see my notes while working.

#### Acceptance Criteria

1. WHEN a sticky note is created THEN the XNote application SHALL display the note as a topmost window relative to the main XNote window
2. WHEN the main XNote window is minimized THEN the XNote application SHALL hide all sticky notes
3. WHEN the main XNote window is restored THEN the XNote application SHALL show all previously visible sticky notes
4. WHEN the main XNote window is closed THEN the XNote application SHALL close all sticky notes

### Requirement 8

**User Story:** As a user, I want a title bar on sticky notes showing note number, so that I can identify and manage multiple notes.

#### Acceptance Criteria

1. WHEN a sticky note is created THEN the XNote application SHALL display a compact title bar with note number (e.g., "Note 1", "Note 2")
2. WHEN a sticky note title bar is displayed THEN the XNote application SHALL include a close button (X) on the right side
3. WHEN multiple sticky notes exist THEN the XNote application SHALL assign unique sequential numbers to each note
