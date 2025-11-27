# Requirements Document

## Introduction

Fitur ini mengintegrasikan Vim mode yang sudah ada di XNote ke dalam sticky notes dengan cara reuse fungsi `ProcessVimKey` yang sudah ada. Pendekatan ini memastikan konsistensi perilaku Vim di seluruh aplikasi dan meminimalkan duplikasi kode.

## Glossary

- **Vim Mode**: Mode editing yang meniru perilaku editor Vim dengan mode Normal, Insert, Visual, dan Command
- **Sticky Note**: Jendela catatan kecil yang mengambang di atas jendela utama XNote
- **ProcessVimKey**: Fungsi yang sudah ada untuk memproses input keyboard dalam Vim mode
- **g_VimState**: Global state Vim yang sudah ada di vim_mode.c

## Requirements

### Requirement 1

**User Story:** Sebagai pengguna, saya ingin Vim mode yang sudah ada berfungsi di sticky notes, sehingga saya mendapat pengalaman editing yang konsisten.

#### Acceptance Criteria

1. WHEN Vim mode is enabled globally AND a user types in a sticky note THEN the Sticky_Note_System SHALL route keyboard input through the existing ProcessVimKey function
2. WHEN Vim mode is disabled globally THEN the Sticky_Note_System SHALL allow normal text input in sticky notes without Vim processing
3. WHEN a user uses any Vim command in a sticky note THEN the Sticky_Note_System SHALL produce the same result as in the main editor

### Requirement 2

**User Story:** Sebagai pengguna, saya ingin melihat indikator mode Vim di sticky note, sehingga saya tahu mode apa yang sedang aktif.

#### Acceptance Criteria

1. WHEN Vim mode is enabled AND a sticky note is active THEN the Sticky_Note_System SHALL display the current Vim mode in the sticky note title bar
2. WHEN Vim mode state changes THEN the Sticky_Note_System SHALL update the title bar indicator immediately
3. WHEN Vim mode is disabled THEN the Sticky_Note_System SHALL display the normal title without mode indicator
