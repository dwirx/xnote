# Requirements Document

## Introduction

Fitur Restore Closed Sticky Notes menambahkan kemampuan untuk menampilkan kembali sticky notes yang sudah ditutup (closed) dan menyediakan panel untuk melihat daftar semua sticky notes yang aktif. Saat ini, ketika sticky note ditutup, note tersebut langsung dihapus secara permanen. Fitur ini memungkinkan pengguna untuk menutup sticky note sementara (hide) tanpa kehilangan kontennya, dan dapat menampilkannya kembali kapan saja melalui menu atau panel daftar notes.

## Glossary

- **Sticky Note**: Jendela catatan kecil yang mengambang (floating) di atas jendela utama aplikasi
- **Active Sticky Note**: Sticky note yang sedang ditampilkan (visible) di layar
- **Hidden Sticky Note**: Sticky note yang ditutup sementara tetapi masih tersimpan dan dapat ditampilkan kembali
- **Sticky Notes Panel**: Dialog atau panel yang menampilkan daftar semua sticky notes (aktif dan hidden)
- **Restore**: Aksi untuk menampilkan kembali sticky note yang sebelumnya ditutup/hidden

## Requirements

### Requirement 1

**User Story:** As a user, I want to hide sticky notes temporarily without deleting them, so that I can restore them later when needed.

#### Acceptance Criteria

1. WHEN a user clicks the close button (X) on a sticky note THEN the XNote application SHALL hide the sticky note instead of deleting it permanently
2. WHEN a sticky note is hidden THEN the XNote application SHALL preserve the note's content, position, size, and color in storage
3. WHEN a sticky note is hidden THEN the XNote application SHALL set the note's visibility state to false
4. WHEN a user wants to permanently delete a sticky note THEN the XNote application SHALL provide a "Delete Note" option in the context menu

### Requirement 2

**User Story:** As a user, I want to restore hidden sticky notes, so that I can view notes I previously closed.

#### Acceptance Criteria

1. WHEN a user selects "Restore Sticky Note" from the View menu THEN the XNote application SHALL display a submenu listing all hidden sticky notes
2. WHEN a user selects a hidden note from the restore submenu THEN the XNote application SHALL make the note visible at its last saved position
3. WHEN a hidden note is restored THEN the XNote application SHALL set the note's visibility state to true and save the change
4. WHEN there are no hidden sticky notes THEN the XNote application SHALL disable or hide the "Restore Sticky Note" menu option

### Requirement 3

**User Story:** As a user, I want to see a list of all my sticky notes, so that I can manage and navigate between them easily.

#### Acceptance Criteria

1. WHEN a user selects "Manage Sticky Notes" from the View menu or presses Ctrl+Shift+M THEN the XNote application SHALL display a dialog showing all sticky notes
2. WHEN the sticky notes dialog is displayed THEN the XNote application SHALL show each note's ID, preview of content (first 30 characters), color indicator, and visibility status
3. WHEN a user double-clicks a note in the dialog THEN the XNote application SHALL bring that note to focus if visible, or restore it if hidden
4. WHEN a user selects a note and clicks "Show/Hide" button THEN the XNote application SHALL toggle the note's visibility
5. WHEN a user selects a note and clicks "Delete" button THEN the XNote application SHALL permanently delete the note after confirmation

### Requirement 4

**User Story:** As a user, I want visual indication of hidden notes count, so that I know there are notes I can restore.

#### Acceptance Criteria

1. WHEN there are hidden sticky notes THEN the XNote application SHALL display the count of hidden notes in the status bar
2. WHEN the hidden notes count changes THEN the XNote application SHALL update the status bar immediately
3. WHEN all hidden notes are restored or deleted THEN the XNote application SHALL remove the hidden notes indicator from status bar

### Requirement 5

**User Story:** As a user, I want to show or hide all sticky notes at once, so that I can quickly clear or restore my workspace.

#### Acceptance Criteria

1. WHEN a user selects "Hide All Sticky Notes" from the View menu THEN the XNote application SHALL hide all visible sticky notes
2. WHEN a user selects "Show All Sticky Notes" from the View menu THEN the XNote application SHALL restore all hidden sticky notes to visible state
3. WHEN all notes are hidden THEN the XNote application SHALL enable "Show All Sticky Notes" and disable "Hide All Sticky Notes"
4. WHEN all notes are visible THEN the XNote application SHALL enable "Hide All Sticky Notes" and disable "Show All Sticky Notes"

