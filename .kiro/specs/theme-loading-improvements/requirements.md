# Requirements Document

## Introduction

Dokumen ini mendefinisikan requirements untuk perbaikan sistem tema dan loading file di XNote. Tujuan utama adalah:
1. Memperbaiki tampilan tema agar lebih konsisten dan menyediakan opsi tema terang yang lebih baik
2. Memperbaiki proses loading file agar lebih cepat, informatif, dan tanpa glitch visual

## Glossary

- **XNote**: Aplikasi text editor berbasis Win32 dengan dukungan multi-tab
- **Theme System**: Sistem yang mengatur warna dan tampilan visual editor
- **File Loading**: Proses membaca dan menampilkan konten file ke editor
- **RichEdit Control**: Komponen Windows untuk menampilkan dan mengedit teks
- **Glitch**: Artefak visual yang tidak diinginkan seperti flicker atau tampilan tidak konsisten
- **Progress Dialog**: Dialog yang menampilkan kemajuan operasi loading
- **Syntax Highlighting**: Pewarnaan kode berdasarkan bahasa pemrograman

## Requirements

### Requirement 1

**User Story:** As a user, I want consistent theme colors across all UI elements, so that the editor looks professional and cohesive.

#### Acceptance Criteria

1. WHEN a theme is applied THEN the XNote System SHALL update all UI elements (editor, tabs, status bar, line numbers) with matching colors within 100ms
2. WHEN switching between themes THEN the XNote System SHALL apply colors without visible flicker or intermediate states
3. WHEN the editor displays text THEN the XNote System SHALL ensure foreground and background colors have sufficient contrast ratio (minimum 4.5:1)
4. WHEN a light theme is selected THEN the XNote System SHALL display light backgrounds with dark text consistently across all components

### Requirement 2

**User Story:** As a user, I want improved light theme options, so that I can work comfortably in bright environments.

#### Acceptance Criteria

1. WHEN the Frosted Glass theme is active THEN the XNote System SHALL display a soft light background (#F2F6FA) with dark text (#2F3C49)
2. WHEN any light theme is active THEN the XNote System SHALL apply appropriate light colors to tab bar, status bar, and line number gutter
3. WHEN syntax highlighting is enabled on a light theme THEN the XNote System SHALL use darker syntax colors that are readable on light backgrounds
4. WHEN the user selects a theme from the menu THEN the XNote System SHALL persist the selection across sessions

### Requirement 3

**User Story:** As a user, I want faster file loading with clear progress indication, so that I know the application is working and can estimate wait time.

#### Acceptance Criteria

1. WHEN loading a file larger than 512KB THEN the XNote System SHALL display a progress dialog with percentage and size information
2. WHEN the progress dialog is shown THEN the XNote System SHALL update progress at least every 100ms
3. WHEN loading completes THEN the XNote System SHALL close the progress dialog and display content within 50ms
4. WHEN loading a file THEN the XNote System SHALL prevent UI freeze by processing in background thread

### Requirement 4

**User Story:** As a user, I want smooth file loading without visual glitches, so that the editor feels polished and professional.

#### Acceptance Criteria

1. WHEN content is streamed to the editor THEN the XNote System SHALL disable redraw until streaming completes
2. WHEN loading completes THEN the XNote System SHALL restore cursor position and scroll state before enabling redraw
3. WHEN applying theme colors during load THEN the XNote System SHALL batch color operations to prevent flicker
4. WHEN the editor receives new content THEN the XNote System SHALL apply syntax highlighting only after content is fully loaded

### Requirement 5

**User Story:** As a user, I want informative status messages during file operations, so that I understand what the application is doing.

#### Acceptance Criteria

1. WHEN loading starts THEN the XNote System SHALL display "Loading: [filename]" in the status bar
2. WHEN loading a large file THEN the XNote System SHALL display file size and loading mode in the status bar
3. WHEN loading completes THEN the XNote System SHALL display file statistics (size, lines, encoding) in the status bar
4. WHEN an error occurs during loading THEN the XNote System SHALL display a clear error message with the cause

### Requirement 6

**User Story:** As a user, I want the theme to be applied correctly when opening files, so that new content matches my selected theme.

#### Acceptance Criteria

1. WHEN a new file is opened THEN the XNote System SHALL apply the current theme colors before displaying content
2. WHEN switching tabs THEN the XNote System SHALL ensure the visible tab has correct theme colors applied
3. WHEN creating a new tab THEN the XNote System SHALL inherit theme colors from the current theme setting
4. WHEN theme colors are applied to an edit control THEN the XNote System SHALL set both background and foreground colors atomically

### Requirement 7

**User Story:** As a developer, I want optimized theme application, so that theme changes are fast and don't impact editor performance.

#### Acceptance Criteria

1. WHEN applying theme to multiple tabs THEN the XNote System SHALL only apply syntax highlighting to the visible tab
2. WHEN theme colors change THEN the XNote System SHALL mark non-visible tabs for lazy refresh
3. WHEN a tab becomes visible THEN the XNote System SHALL apply pending syntax highlighting if marked dirty
4. WHEN applying theme THEN the XNote System SHALL complete the operation within 200ms for files under 1MB
