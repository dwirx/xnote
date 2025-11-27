# Requirements Document

## Introduction

Fitur "Stay on Top" (Always on Top) memungkinkan pengguna untuk menjaga jendela XNote tetap berada di atas semua jendela aplikasi lain. Fitur ini berguna ketika pengguna perlu mereferensikan catatan atau kode sambil bekerja di aplikasi lain. Pengguna dapat mengaktifkan dan menonaktifkan fitur ini melalui menu View atau shortcut keyboard.

## Glossary

- **XNote**: Aplikasi text editor berbasis Windows yang sedang dikembangkan
- **Stay on Top**: Mode tampilan di mana jendela aplikasi selalu berada di atas jendela aplikasi lain
- **Topmost Window**: Jendela Windows yang memiliki atribut WS_EX_TOPMOST sehingga selalu tampil di depan jendela non-topmost
- **Toggle**: Aksi untuk mengubah status antara aktif dan non-aktif

## Requirements

### Requirement 1

**User Story:** As a user, I want to toggle the stay-on-top mode for the XNote window, so that I can keep my notes visible while working in other applications.

#### Acceptance Criteria

1. WHEN a user selects the "Stay on Top" menu item from the View menu THEN XNote SHALL toggle the topmost window state
2. WHEN the stay-on-top mode is activated THEN XNote SHALL display the window above all non-topmost windows
3. WHEN the stay-on-top mode is deactivated THEN XNote SHALL return the window to normal z-order behavior
4. WHEN the stay-on-top mode is active THEN XNote SHALL display a checkmark next to the "Stay on Top" menu item
5. WHEN the stay-on-top mode is inactive THEN XNote SHALL remove the checkmark from the "Stay on Top" menu item

### Requirement 2

**User Story:** As a user, I want to use a keyboard shortcut to toggle stay-on-top mode, so that I can quickly enable or disable the feature without using the mouse.

#### Acceptance Criteria

1. WHEN a user presses Ctrl+Shift+T THEN XNote SHALL toggle the stay-on-top mode
2. WHEN the keyboard shortcut is used THEN XNote SHALL update the menu checkmark state accordingly

### Requirement 3

**User Story:** As a user, I want the stay-on-top setting to persist across sessions, so that I do not have to re-enable it every time I open XNote.

#### Acceptance Criteria

1. WHEN XNote exits with stay-on-top mode active THEN XNote SHALL save the stay-on-top state to settings
2. WHEN XNote starts THEN XNote SHALL restore the previously saved stay-on-top state
3. WHEN no previous setting exists THEN XNote SHALL default to stay-on-top mode disabled
