# Implementation Plan

## 1. Improve Theme System Anti-Flicker

- [x] 1.1 Add contrast ratio calculation function to theme.c
  - Implement `CalculateContrastRatio(COLORREF crFg, COLORREF crBg)` using WCAG 2.0 formula
  - Implement `GetRelativeLuminance(COLORREF cr)` helper function
  - _Requirements: 1.3_

- [ ]* 1.2 Write property test for theme contrast ratio
  - **Property 1: Theme Contrast Ratio**
  - **Validates: Requirements 1.3**

- [x] 1.3 Add light theme detection function
  - Implement `IsLightTheme(ThemeType theme)` to identify light themes
  - Return TRUE for THEME_LIGHT, THEME_TOKYO_NIGHT_LIGHT, THEME_EVERFOREST_LIGHT, THEME_FROSTED_GLASS
  - _Requirements: 1.4, 2.2_

- [ ]* 1.4 Write property test for light theme luminance
  - **Property 2: Light Theme Luminance**
  - **Validates: Requirements 1.4, 2.2**

- [x] 1.5 Improve ApplyThemeToEdit function to prevent flicker
  - Disable redraw with WM_SETREDRAW FALSE before color changes
  - Batch all color operations (background, foreground, selection)
  - Re-enable redraw and invalidate only once at the end
  - _Requirements: 1.1, 1.2, 4.3_

## 2. Optimize Theme Application for Multiple Tabs

- [x] 2.1 Implement lazy syntax highlighting for non-visible tabs
  - Modify ApplyThemeToWindow to only apply syntax highlighting to current tab
  - Set bNeedsSyntaxRefresh = TRUE for other tabs
  - _Requirements: 7.1, 7.2_

- [ ]* 2.2 Write property test for lazy syntax highlighting
  - **Property 8: Lazy Syntax Highlighting**
  - **Validates: Requirements 7.1, 7.2**

- [x] 2.3 Apply pending syntax highlighting when tab becomes visible
  - In tab switch handler, check bNeedsSyntaxRefresh flag
  - Apply syntax highlighting if flag is TRUE, then set to FALSE
  - _Requirements: 7.3_

- [ ]* 2.4 Write property test for dirty tab refresh
  - **Property 9: Dirty Tab Refresh on Visibility**
  - **Validates: Requirements 7.3**

## 3. Checkpoint - Verify Theme Improvements
- [x] 3. Ensure all tests pass, ask the user if questions arise.

## 4. Improve File Loading Progress Display

- [x] 4.1 Enhance progress dialog with better information
  - Show file name, current size loaded, total size, percentage
  - Update progress text format: "Loading... X% (Y MB / Z MB)"
  - _Requirements: 3.1, 3.2_

- [x] 4.2 Improve status bar messages during loading
  - Display "Loading: [filename]" when loading starts
  - Show file size and mode for large files
  - Display statistics (size, lines, encoding) after load completes
  - _Requirements: 5.1, 5.2, 5.3_

- [ ]* 4.3 Write property test for status bar file info
  - **Property 5: Status Bar Contains File Info After Load**
  - **Validates: Requirements 5.2, 5.3**

## 5. Eliminate Loading Glitches

- [x] 5.1 Implement flicker-free content streaming
  - Save cursor position and scroll state before loading
  - Disable redraw before streaming content
  - Restore cursor and scroll state after streaming
  - Enable redraw only after all operations complete
  - _Requirements: 4.1, 4.2_

- [x] 5.2 Apply theme colors before displaying content
  - Call ApplyThemeToEdit before setting content
  - Ensure background color is set first to prevent white flash
  - _Requirements: 6.1, 6.4_

- [x] 5.3 Delay syntax highlighting until content is fully loaded
  - Move syntax highlighting call to after EM_STREAMIN completes
  - Only apply if file size is under syntax highlighting threshold
  - _Requirements: 4.4_

## 6. Ensure New Tabs Inherit Theme

- [x] 6.1 Apply theme to new tab edit control immediately after creation
  - In CreateNewTab, call ApplyThemeToEdit after creating RichEdit control
  - Set background color before any content is added
  - _Requirements: 6.3_

- [ ]* 6.2 Write property test for new tab theme inheritance
  - **Property 7: New Tab Inherits Theme**
  - **Validates: Requirements 6.3**

## 7. Ensure Tab Switch Theme Consistency

- [x] 7.1 Verify theme colors on tab switch
  - In tab switch handler, verify visible tab has correct theme colors
  - Apply theme if colors don't match current theme
  - _Requirements: 6.2_

- [ ]* 7.2 Write property test for tab theme consistency
  - **Property 6: Tab Theme Consistency**
  - **Validates: Requirements 6.2**

## 8. Verify Theme Persistence

- [x] 8.1 Ensure theme is saved in session
  - Verify SaveSession writes g_CurrentTheme to session file
  - Verify LoadSession restores g_CurrentTheme correctly
  - _Requirements: 2.4_

- [ ]* 8.2 Write property test for theme persistence
  - **Property 4: Theme Persistence Round Trip**
  - **Validates: Requirements 2.4**

## 9. Verify Syntax Colors on Light Themes

- [ ]* 9.1 Write property test for syntax color contrast on light themes
  - **Property 3: Syntax Color Contrast on Light Themes**
  - **Validates: Requirements 2.3**

- [x] 9.2 Adjust syntax colors if any fail contrast test
  - Review and adjust syntax colors for light themes if needed
  - Ensure all syntax colors have contrast ratio >= 4.5:1 against light backgrounds
  - _Requirements: 2.3_

## 10. Final Checkpoint - Verify All Improvements
- [x] 10. Ensure all tests pass, ask the user if questions arise.
