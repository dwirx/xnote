# Requirements Document

## Introduction

Fitur Sticky Notes Persistence menambahkan kemampuan untuk menyimpan dan memuat data sticky notes secara persisten. Saat ini, ketika aplikasi XNote ditutup, sticky notes hilang karena data tidak tersimpan dengan benar. Fitur ini memastikan data sticky notes tersimpan otomatis, bisa di-export/import ke file terpisah, dan pengguna bisa memilih lokasi penyimpanan.

## Glossary

- **Sticky Note**: Jendela catatan kecil yang mengambang (floating) di atas jendela utama aplikasi
- **Persistence**: Kemampuan untuk menyimpan data secara permanen sehingga tetap ada setelah aplikasi ditutup
- **Auto-save**: Penyimpanan otomatis tanpa intervensi pengguna
- **Export**: Menyimpan data sticky notes ke file terpisah yang bisa dipindahkan
- **Import**: Memuat data sticky notes dari file eksternal
- **Storage Location**: Lokasi folder tempat file sticky notes disimpan

## Requirements

### Requirement 1

**User Story:** As a user, I want my sticky notes to be automatically saved when XNote closes, so that I don't lose my notes.

#### Acceptance Criteria

1. WHEN the XNote application is closing THEN the XNote application SHALL save all sticky notes data to storage before exit
2. WHEN the XNote application starts THEN the XNote application SHALL load all previously saved sticky notes from storage
3. WHEN a sticky note content changes THEN the XNote application SHALL mark the note for auto-save
4. WHEN the auto-save timer triggers THEN the XNote application SHALL save all modified notes to storage

### Requirement 2

**User Story:** As a user, I want to export my sticky notes to a file, so that I can backup or share them.

#### Acceptance Criteria

1. WHEN a user selects "Export Sticky Notes" from the File menu THEN the XNote application SHALL display a save file dialog
2. WHEN a user confirms the export location THEN the XNote application SHALL save all sticky notes to a JSON file at the selected location
3. WHEN the export completes successfully THEN the XNote application SHALL display a confirmation message with the number of notes exported
4. WHEN the export fails THEN the XNote application SHALL display an error message describing the failure

### Requirement 3

**User Story:** As a user, I want to import sticky notes from a file, so that I can restore backups or receive shared notes.

#### Acceptance Criteria

1. WHEN a user selects "Import Sticky Notes" from the File menu THEN the XNote application SHALL display an open file dialog
2. WHEN a user selects a valid sticky notes file THEN the XNote application SHALL ask whether to merge or replace existing notes
3. WHEN the user chooses "Merge" THEN the XNote application SHALL add imported notes to existing notes
4. WHEN the user chooses "Replace" THEN the XNote application SHALL delete existing notes and load only imported notes
5. WHEN the import completes successfully THEN the XNote application SHALL display a confirmation message with the number of notes imported
6. WHEN the import file is invalid THEN the XNote application SHALL display an error message

### Requirement 4

**User Story:** As a user, I want to choose where my sticky notes are stored, so that I can sync them across devices or keep them in a specific location.

#### Acceptance Criteria

1. WHEN a user selects "Sticky Notes Settings" from the View menu THEN the XNote application SHALL display a settings dialog
2. WHEN the settings dialog is displayed THEN the XNote application SHALL show the current storage location
3. WHEN a user clicks "Browse" THEN the XNote application SHALL display a folder selection dialog
4. WHEN a user selects a new folder THEN the XNote application SHALL move existing sticky notes data to the new location
5. WHEN a user clicks "Reset to Default" THEN the XNote application SHALL set storage location to the default AppData folder

### Requirement 5

**User Story:** As a user, I want visual feedback when sticky notes are being saved, so that I know my data is safe.

#### Acceptance Criteria

1. WHEN sticky notes are being saved THEN the XNote application SHALL display a brief "Saving..." indicator in the status bar
2. WHEN sticky notes save completes THEN the XNote application SHALL display "Saved" indicator briefly
3. WHEN sticky notes save fails THEN the XNote application SHALL display an error indicator and notification

