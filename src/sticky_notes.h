/**
 * sticky_notes.h - Sticky Notes support for XNote
 * Floating notes that stay on top of the main window
 */

#ifndef STICKY_NOTES_H
#define STICKY_NOTES_H

#include <windows.h>
#include <tchar.h>

/* Maximum number of sticky notes */
#define MAX_STICKY_NOTES 10

/* Sticky note dimensions */
#define STICKY_NOTE_MIN_WIDTH 100
#define STICKY_NOTE_MIN_HEIGHT 80
#define STICKY_NOTE_DEFAULT_WIDTH 200
#define STICKY_NOTE_DEFAULT_HEIGHT 150
#define STICKY_NOTE_OFFSET 30
#define STICKY_NOTE_TITLE_HEIGHT 24
#define STICKY_NOTE_CLOSE_BTN_SIZE 16

/* Maximum content size */
#define STICKY_NOTE_MAX_CONTENT 4096

/* Auto-save timer */
#define TIMER_STICKY_AUTOSAVE 10
#define STICKY_AUTOSAVE_DELAY 2000  /* 2 seconds - faster autosave */

/* Sticky note colors */
typedef enum {
    STICKY_COLOR_YELLOW = 0,
    STICKY_COLOR_PINK,
    STICKY_COLOR_BLUE,
    STICKY_COLOR_GREEN,
    STICKY_COLOR_ORANGE,
    STICKY_COLOR_PURPLE,
    STICKY_COLOR_COUNT
} StickyNoteColor;

/* Sticky note state structure */
typedef struct {
    HWND hwndNote;                          /* Window handle for the sticky note */
    HWND hwndEdit;                          /* Edit control inside the note */
    int nNoteId;                            /* Unique note ID (1-based) */
    int nPosX;                              /* X position */
    int nPosY;                              /* Y position */
    int nWidth;                             /* Width */
    int nHeight;                            /* Height */
    StickyNoteColor color;                  /* Background color */
    TCHAR szContent[STICKY_NOTE_MAX_CONTENT]; /* Note content */
    BOOL bModified;                         /* Modified flag for auto-save */
    BOOL bVisible;                          /* Visibility state */
} StickyNote;

/* Sticky notes manager structure */
typedef struct {
    StickyNote notes[MAX_STICKY_NOTES];
    int nNoteCount;
    int nNextNoteId;                        /* Next available note ID */
    HWND hwndParent;                        /* Parent window (main XNote window) */
    HINSTANCE hInstance;
    BOOL bInitialized;
} StickyNotesManager;

/* Initialization and cleanup */
void StickyNotes_Init(HWND hwndParent, HINSTANCE hInstance);
void StickyNotes_Cleanup(void);

/* Note management */
BOOL StickyNotes_Create(void);
void StickyNotes_Close(int nIndex);
void StickyNotes_Delete(int nIndex);
void StickyNotes_CloseAll(void);
int StickyNotes_GetCount(void);
StickyNote* StickyNotes_GetNote(int nIndex);

/* Note properties */
void StickyNotes_SetColor(int nIndex, StickyNoteColor color);
StickyNoteColor StickyNotes_GetColor(int nIndex);
void StickyNotes_SetContent(int nIndex, const TCHAR* szContent);
const TCHAR* StickyNotes_GetContent(int nIndex);

/* Visibility control */
void StickyNotes_ShowAll(void);
void StickyNotes_HideAll(void);
void StickyNotes_ToggleAll(void);

/* Hide/Restore functions - NEW */
void StickyNotes_Hide(int nIndex);
void StickyNotes_Restore(int nIndex);
void StickyNotes_ToggleVisibility(int nIndex);

/* Hidden notes tracking - NEW */
int StickyNotes_GetHiddenCount(void);
int StickyNotes_GetVisibleCount(void);
int StickyNotes_GetHiddenNotes(int* pIndices, int nMaxCount);
int StickyNotes_GetVisibleNotes(int* pIndices, int nMaxCount);

/* Manage dialog - NEW */
void StickyNotes_ShowManageDialog(HWND hwndParent);

/* Build restore submenu - NEW */
void StickyNotes_BuildRestoreMenu(HMENU hMenu, int nBaseId);

/* Update status bar with hidden count - NEW */
void StickyNotes_UpdateStatusBar(HWND hwndStatusBar, int nPart);

/* Persistence */
void StickyNotes_Save(void);
void StickyNotes_Load(void);
void StickyNotes_ForceSave(void);

/* Export/Import */
BOOL StickyNotes_Export(HWND hwndParent);
int StickyNotes_Import(HWND hwndParent);

/* Mark note as modified (triggers auto-save) */
void StickyNotes_MarkModified(int nIndex);

/* Get color RGB value */
COLORREF StickyNotes_GetColorRGB(StickyNoteColor color);

/* Window procedure */
LRESULT CALLBACK StickyNoteWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif /* STICKY_NOTES_H */
