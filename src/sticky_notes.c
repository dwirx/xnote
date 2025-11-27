/**
 * sticky_notes.c - Sticky Notes implementation for XNote
 * Enhanced version with better UI and more features
 */

#include "sticky_notes.h"
#include "notepad.h"
#include "resource.h"
#include <shlobj.h>
#include <stdio.h>
#include <windowsx.h>
#include <commctrl.h>

/* Window class name for sticky notes */
static const TCHAR szStickyNoteClass[] = TEXT("XNoteStickyNote");

/* Global sticky notes manager */
static StickyNotesManager g_StickyManager = {0};

/* Predefined colors for sticky notes - more vibrant colors */
static const COLORREF g_StickyColors[STICKY_COLOR_COUNT] = {
    RGB(255, 245, 157),  /* Yellow - warm */
    RGB(248, 187, 208),  /* Pink - soft */
    RGB(179, 229, 252),  /* Blue - sky */
    RGB(200, 230, 201),  /* Green - mint */
    RGB(255, 204, 128),  /* Orange - peach */
    RGB(206, 147, 216)   /* Purple - lavender */
};

/* Darker title bar colors */
static const COLORREF g_StickyTitleColors[STICKY_COLOR_COUNT] = {
    RGB(255, 235, 59),   /* Yellow */
    RGB(240, 98, 146),   /* Pink */
    RGB(100, 181, 246),  /* Blue */
    RGB(129, 199, 132),  /* Green */
    RGB(255, 167, 38),   /* Orange */
    RGB(171, 71, 188)    /* Purple */
};

/* Color names for menu */
static const TCHAR* g_StickyColorNames[STICKY_COLOR_COUNT] = {
    TEXT("🟡 Yellow"),
    TEXT("🩷 Pink"),
    TEXT("🔵 Blue"),
    TEXT("🟢 Green"),
    TEXT("🟠 Orange"),
    TEXT("🟣 Purple")
};

/* Auto-save timer state */
static BOOL g_bAutoSavePending = FALSE;

/* Hover state for close button */
static int g_nHoverCloseNote = -1;

/* Forward declarations */
static BOOL RegisterStickyNoteClass(HINSTANCE hInstance);
static void UnregisterStickyNoteClass(HINSTANCE hInstance);
static void CalculateNewNotePosition(int* pX, int* pY);
static BOOL GetStickyNotesPath(TCHAR* szPath, DWORD nSize);

/* Get color RGB value */
COLORREF StickyNotes_GetColorRGB(StickyNoteColor color) {
    if (color >= 0 && color < STICKY_COLOR_COUNT) {
        return g_StickyColors[color];
    }
    return g_StickyColors[STICKY_COLOR_YELLOW];
}

/* Get title bar color */
static COLORREF GetTitleBarColor(StickyNoteColor color) {
    if (color >= 0 && color < STICKY_COLOR_COUNT) {
        return g_StickyTitleColors[color];
    }
    return g_StickyTitleColors[STICKY_COLOR_YELLOW];
}

/* Initialize sticky notes system */
void StickyNotes_Init(HWND hwndParent, HINSTANCE hInstance) {
    if (g_StickyManager.bInitialized) return;
    
    g_StickyManager.hwndParent = hwndParent;
    g_StickyManager.hInstance = hInstance;
    g_StickyManager.nNoteCount = 0;
    g_StickyManager.nNextNoteId = 1;
    
    /* Initialize all notes */
    for (int i = 0; i < MAX_STICKY_NOTES; i++) {
        g_StickyManager.notes[i].hwndNote = NULL;
        g_StickyManager.notes[i].hwndEdit = NULL;
        g_StickyManager.notes[i].nNoteId = 0;
        g_StickyManager.notes[i].szContent[0] = TEXT('\0');
    }
    
    /* Register window class */
    RegisterStickyNoteClass(hInstance);
    
    g_StickyManager.bInitialized = TRUE;
}

/* Cleanup sticky notes system */
void StickyNotes_Cleanup(void) {
    if (!g_StickyManager.bInitialized) return;
    
    /* Save before cleanup */
    StickyNotes_Save();
    
    /* Close all notes without saving (already saved) */
    for (int i = g_StickyManager.nNoteCount - 1; i >= 0; i--) {
        StickyNote* pNote = &g_StickyManager.notes[i];
        if (pNote->hwndEdit) {
            DestroyWindow(pNote->hwndEdit);
            pNote->hwndEdit = NULL;
        }
        if (pNote->hwndNote) {
            DestroyWindow(pNote->hwndNote);
            pNote->hwndNote = NULL;
        }
    }
    g_StickyManager.nNoteCount = 0;
    
    /* Unregister window class */
    UnregisterStickyNoteClass(g_StickyManager.hInstance);
    
    g_StickyManager.bInitialized = FALSE;
}

/* Register sticky note window class */
static BOOL RegisterStickyNoteClass(HINSTANCE hInstance) {
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW;  /* Add drop shadow */
    wc.lpfnWndProc = StickyNoteWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(LONG_PTR);
    wc.hInstance = hInstance;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = szStickyNoteClass;
    wc.hIconSm = NULL;
    
    return RegisterClassEx(&wc) != 0;
}

/* Unregister sticky note window class */
static void UnregisterStickyNoteClass(HINSTANCE hInstance) {
    UnregisterClass(szStickyNoteClass, hInstance);
}

/* Calculate position for new note with offset */
static void CalculateNewNotePosition(int* pX, int* pY) {
    RECT rcParent;
    GetWindowRect(g_StickyManager.hwndParent, &rcParent);
    
    /* Base position - center of parent window */
    int baseX = rcParent.left + (rcParent.right - rcParent.left) / 2 - STICKY_NOTE_DEFAULT_WIDTH / 2;
    int baseY = rcParent.top + 100;
    
    /* Add offset based on existing notes */
    *pX = baseX + (g_StickyManager.nNoteCount * STICKY_NOTE_OFFSET);
    *pY = baseY + (g_StickyManager.nNoteCount * STICKY_NOTE_OFFSET);
    
    /* Make sure it's on screen */
    RECT rcWork;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, 0);
    
    if (*pX + STICKY_NOTE_DEFAULT_WIDTH > rcWork.right) {
        *pX = rcWork.left + 50;
    }
    if (*pY + STICKY_NOTE_DEFAULT_HEIGHT > rcWork.bottom) {
        *pY = rcWork.top + 50;
    }
    if (*pX < rcWork.left) *pX = rcWork.left + 20;
    if (*pY < rcWork.top) *pY = rcWork.top + 20;
}

/* Create a new sticky note */
BOOL StickyNotes_Create(void) {
    if (!g_StickyManager.bInitialized) return FALSE;
    
    /* Check maximum limit */
    if (g_StickyManager.nNoteCount >= MAX_STICKY_NOTES) {
        MessageBox(g_StickyManager.hwndParent, 
                   TEXT("Maximum number of sticky notes (10) reached."),
                   TEXT("XNote - Sticky Notes"), MB_OK | MB_ICONWARNING);
        return FALSE;
    }
    
    int nIndex = g_StickyManager.nNoteCount;
    StickyNote* pNote = &g_StickyManager.notes[nIndex];
    
    /* Calculate position */
    CalculateNewNotePosition(&pNote->nPosX, &pNote->nPosY);
    
    /* Set default properties */
    pNote->nWidth = STICKY_NOTE_DEFAULT_WIDTH;
    pNote->nHeight = STICKY_NOTE_DEFAULT_HEIGHT;
    pNote->color = STICKY_COLOR_YELLOW;
    pNote->nNoteId = g_StickyManager.nNextNoteId++;
    pNote->szContent[0] = TEXT('\0');
    pNote->bModified = FALSE;
    pNote->bVisible = TRUE;
    
    /* Create the window with layered style for better appearance */
    pNote->hwndNote = CreateWindowEx(
        WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
        szStickyNoteClass,
        TEXT(""),
        WS_POPUP | WS_THICKFRAME | WS_VISIBLE,
        pNote->nPosX, pNote->nPosY,
        pNote->nWidth, pNote->nHeight,
        g_StickyManager.hwndParent,
        NULL,
        g_StickyManager.hInstance,
        (LPVOID)(LONG_PTR)nIndex
    );
    
    if (!pNote->hwndNote) {
        return FALSE;
    }
    
    g_StickyManager.nNoteCount++;
    
    /* Focus the edit control */
    if (pNote->hwndEdit) {
        SetFocus(pNote->hwndEdit);
    }
    
    /* Save immediately */
    StickyNotes_Save();
    
    return TRUE;
}

/* Delete a sticky note permanently */
void StickyNotes_Delete(int nIndex) {
    if (nIndex < 0 || nIndex >= g_StickyManager.nNoteCount) return;
    
    StickyNote* pNote = &g_StickyManager.notes[nIndex];
    
    /* Destroy windows */
    if (pNote->hwndEdit) {
        DestroyWindow(pNote->hwndEdit);
        pNote->hwndEdit = NULL;
    }
    if (pNote->hwndNote) {
        DestroyWindow(pNote->hwndNote);
        pNote->hwndNote = NULL;
    }
    
    /* Shift remaining notes */
    for (int i = nIndex; i < g_StickyManager.nNoteCount - 1; i++) {
        g_StickyManager.notes[i] = g_StickyManager.notes[i + 1];
        /* Update window's stored index */
        if (g_StickyManager.notes[i].hwndNote) {
            SetWindowLongPtr(g_StickyManager.notes[i].hwndNote, 0, (LONG_PTR)i);
        }
    }
    
    g_StickyManager.nNoteCount--;
    
    /* Clear the last slot */
    StickyNote* pLast = &g_StickyManager.notes[g_StickyManager.nNoteCount];
    pLast->hwndNote = NULL;
    pLast->hwndEdit = NULL;
    pLast->nNoteId = 0;
    pLast->szContent[0] = TEXT('\0');
    
    /* Save changes immediately */
    StickyNotes_Save();
}

/* Hide a sticky note (keep in storage, just hide window) */
void StickyNotes_Hide(int nIndex) {
    if (nIndex < 0 || nIndex >= g_StickyManager.nNoteCount) return;
    
    StickyNote* pNote = &g_StickyManager.notes[nIndex];
    if (!pNote->bVisible) return; /* Already hidden */
    
    /* Save content from edit control before hiding */
    if (pNote->hwndEdit) {
        GetWindowText(pNote->hwndEdit, pNote->szContent, STICKY_NOTE_MAX_CONTENT);
    }
    
    /* Save position before hiding */
    if (pNote->hwndNote) {
        RECT rc;
        GetWindowRect(pNote->hwndNote, &rc);
        pNote->nPosX = rc.left;
        pNote->nPosY = rc.top;
        pNote->nWidth = rc.right - rc.left;
        pNote->nHeight = rc.bottom - rc.top;
        
        ShowWindow(pNote->hwndNote, SW_HIDE);
    }
    
    pNote->bVisible = FALSE;
    StickyNotes_Save();
}

/* Restore a hidden sticky note */
void StickyNotes_Restore(int nIndex) {
    if (nIndex < 0 || nIndex >= g_StickyManager.nNoteCount) return;
    
    StickyNote* pNote = &g_StickyManager.notes[nIndex];
    if (pNote->bVisible) return; /* Already visible */
    
    if (pNote->hwndNote) {
        ShowWindow(pNote->hwndNote, SW_SHOW);
        SetForegroundWindow(pNote->hwndNote);
        if (pNote->hwndEdit) {
            SetFocus(pNote->hwndEdit);
        }
    }
    
    pNote->bVisible = TRUE;
    StickyNotes_Save();
}

/* Toggle visibility of a sticky note */
void StickyNotes_ToggleVisibility(int nIndex) {
    if (nIndex < 0 || nIndex >= g_StickyManager.nNoteCount) return;
    
    if (g_StickyManager.notes[nIndex].bVisible) {
        StickyNotes_Hide(nIndex);
    } else {
        StickyNotes_Restore(nIndex);
    }
}

/* Close (hide) a sticky note - now hides instead of delete */
void StickyNotes_Close(int nIndex) {
    if (nIndex < 0 || nIndex >= g_StickyManager.nNoteCount) return;
    
    /* Close now means hide, not delete */
    StickyNotes_Hide(nIndex);
}

/* Close all sticky notes */
void StickyNotes_CloseAll(void) {
    while (g_StickyManager.nNoteCount > 0) {
        StickyNotes_Delete(g_StickyManager.nNoteCount - 1);
    }
}

/* Get note count */
int StickyNotes_GetCount(void) {
    return g_StickyManager.nNoteCount;
}

/* Get note by index */
StickyNote* StickyNotes_GetNote(int nIndex) {
    if (nIndex >= 0 && nIndex < g_StickyManager.nNoteCount) {
        return &g_StickyManager.notes[nIndex];
    }
    return NULL;
}

/* Set note color */
void StickyNotes_SetColor(int nIndex, StickyNoteColor color) {
    if (nIndex < 0 || nIndex >= g_StickyManager.nNoteCount) return;
    if (color < 0 || color >= STICKY_COLOR_COUNT) return;
    
    g_StickyManager.notes[nIndex].color = color;
    
    /* Repaint the note and edit control */
    if (g_StickyManager.notes[nIndex].hwndNote) {
        InvalidateRect(g_StickyManager.notes[nIndex].hwndNote, NULL, TRUE);
        if (g_StickyManager.notes[nIndex].hwndEdit) {
            InvalidateRect(g_StickyManager.notes[nIndex].hwndEdit, NULL, TRUE);
        }
    }
    
    StickyNotes_Save();
}

/* Get note color */
StickyNoteColor StickyNotes_GetColor(int nIndex) {
    if (nIndex >= 0 && nIndex < g_StickyManager.nNoteCount) {
        return g_StickyManager.notes[nIndex].color;
    }
    return STICKY_COLOR_YELLOW;
}

/* Set note content */
void StickyNotes_SetContent(int nIndex, const TCHAR* szContent) {
    if (nIndex < 0 || nIndex >= g_StickyManager.nNoteCount) return;
    if (!szContent) return;
    
    _tcsncpy(g_StickyManager.notes[nIndex].szContent, szContent, STICKY_NOTE_MAX_CONTENT - 1);
    g_StickyManager.notes[nIndex].szContent[STICKY_NOTE_MAX_CONTENT - 1] = TEXT('\0');
    
    StickyNotes_MarkModified(nIndex);
}

/* Get note content */
const TCHAR* StickyNotes_GetContent(int nIndex) {
    if (nIndex >= 0 && nIndex < g_StickyManager.nNoteCount) {
        return g_StickyManager.notes[nIndex].szContent;
    }
    return TEXT("");
}

/* Show all sticky notes (restore all hidden) */
void StickyNotes_ShowAll(void) {
    for (int i = 0; i < g_StickyManager.nNoteCount; i++) {
        if (g_StickyManager.notes[i].hwndNote) {
            if (!g_StickyManager.notes[i].bVisible) {
                g_StickyManager.notes[i].bVisible = TRUE;
            }
            ShowWindow(g_StickyManager.notes[i].hwndNote, SW_SHOWNOACTIVATE);
        }
    }
    StickyNotes_Save();
}

/* Hide all sticky notes */
void StickyNotes_HideAll(void) {
    for (int i = 0; i < g_StickyManager.nNoteCount; i++) {
        if (g_StickyManager.notes[i].hwndNote) {
            /* Save content before hiding */
            if (g_StickyManager.notes[i].hwndEdit) {
                GetWindowText(g_StickyManager.notes[i].hwndEdit, 
                             g_StickyManager.notes[i].szContent, STICKY_NOTE_MAX_CONTENT);
            }
            g_StickyManager.notes[i].bVisible = FALSE;
            ShowWindow(g_StickyManager.notes[i].hwndNote, SW_HIDE);
        }
    }
    StickyNotes_Save();
}

/* Get count of hidden notes */
int StickyNotes_GetHiddenCount(void) {
    int count = 0;
    for (int i = 0; i < g_StickyManager.nNoteCount; i++) {
        if (!g_StickyManager.notes[i].bVisible) {
            count++;
        }
    }
    return count;
}

/* Get count of visible notes */
int StickyNotes_GetVisibleCount(void) {
    int count = 0;
    for (int i = 0; i < g_StickyManager.nNoteCount; i++) {
        if (g_StickyManager.notes[i].bVisible) {
            count++;
        }
    }
    return count;
}

/* Get list of hidden note indices */
int StickyNotes_GetHiddenNotes(int* pIndices, int nMaxCount) {
    int count = 0;
    for (int i = 0; i < g_StickyManager.nNoteCount && count < nMaxCount; i++) {
        if (!g_StickyManager.notes[i].bVisible) {
            pIndices[count++] = i;
        }
    }
    return count;
}

/* Get list of visible note indices */
int StickyNotes_GetVisibleNotes(int* pIndices, int nMaxCount) {
    int count = 0;
    for (int i = 0; i < g_StickyManager.nNoteCount && count < nMaxCount; i++) {
        if (g_StickyManager.notes[i].bVisible) {
            pIndices[count++] = i;
        }
    }
    return count;
}

/* Build restore submenu with hidden notes */
void StickyNotes_BuildRestoreMenu(HMENU hMenu, int nBaseId) {
    /* Remove existing items */
    while (GetMenuItemCount(hMenu) > 0) {
        DeleteMenu(hMenu, 0, MF_BYPOSITION);
    }
    
    int hiddenCount = 0;
    for (int i = 0; i < g_StickyManager.nNoteCount; i++) {
        if (!g_StickyManager.notes[i].bVisible) {
            TCHAR szItem[64];
            TCHAR szPreview[32];
            
            /* Get preview of content */
            const TCHAR* content = g_StickyManager.notes[i].szContent;
            int len = (int)_tcslen(content);
            if (len > 25) {
                _tcsncpy(szPreview, content, 25);
                szPreview[25] = TEXT('\0');
                _tcscat(szPreview, TEXT("..."));
            } else if (len > 0) {
                _tcscpy(szPreview, content);
            } else {
                _tcscpy(szPreview, TEXT("(empty)"));
            }
            
            /* Replace newlines with spaces in preview */
            for (TCHAR* p = szPreview; *p; p++) {
                if (*p == TEXT('\n') || *p == TEXT('\r')) *p = TEXT(' ');
            }
            
            wsprintf(szItem, TEXT("Note %d: %s"), g_StickyManager.notes[i].nNoteId, szPreview);
            AppendMenu(hMenu, MF_STRING, nBaseId + i, szItem);
            hiddenCount++;
        }
    }
    
    if (hiddenCount == 0) {
        AppendMenu(hMenu, MF_STRING | MF_GRAYED, 0, TEXT("(No hidden notes)"));
    }
}

/* Update status bar with hidden notes count */
void StickyNotes_UpdateStatusBar(HWND hwndStatusBar, int nPart) {
    if (!hwndStatusBar) return;
    
    int hiddenCount = StickyNotes_GetHiddenCount();
    TCHAR szText[32];
    
    if (hiddenCount > 0) {
        wsprintf(szText, TEXT("Hidden: %d"), hiddenCount);
    } else {
        szText[0] = TEXT('\0');
    }
    
    SendMessage(hwndStatusBar, SB_SETTEXT, nPart, (LPARAM)szText);
}

/* Mark note as modified */
void StickyNotes_MarkModified(int nIndex) {
    if (nIndex < 0 || nIndex >= g_StickyManager.nNoteCount) return;
    
    g_StickyManager.notes[nIndex].bModified = TRUE;
    
    /* Start auto-save timer if not already pending */
    if (!g_bAutoSavePending && g_StickyManager.hwndParent) {
        SetTimer(g_StickyManager.hwndParent, TIMER_STICKY_AUTOSAVE, STICKY_AUTOSAVE_DELAY, NULL);
        g_bAutoSavePending = TRUE;
    }
}

/* Get path for sticky notes JSON file */
static BOOL GetStickyNotesPath(TCHAR* szPath, DWORD nSize) {
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szPath))) {
        _tcsncat(szPath, TEXT("\\XNote"), nSize - _tcslen(szPath) - 1);
        CreateDirectory(szPath, NULL);
        _tcsncat(szPath, TEXT("\\sticky_notes.json"), nSize - _tcslen(szPath) - 1);
        return TRUE;
    }
    return FALSE;
}

/* Helper to escape JSON string */
static void EscapeJsonStr(const TCHAR* src, char* dest, int destSize) {
    int j = 0;
    for (int i = 0; src[i] && j < destSize - 6; i++) {
        if (src[i] == '\\') { dest[j++] = '\\'; dest[j++] = '\\'; }
        else if (src[i] == '"') { dest[j++] = '\\'; dest[j++] = '"'; }
        else if (src[i] == '\n') { dest[j++] = '\\'; dest[j++] = 'n'; }
        else if (src[i] == '\r') { dest[j++] = '\\'; dest[j++] = 'r'; }
        else if (src[i] == '\t') { dest[j++] = '\\'; dest[j++] = 't'; }
        else if (src[i] < 128) { dest[j++] = (char)src[i]; }
    }
    dest[j] = '\0';
}


/* Save sticky notes to JSON file */
void StickyNotes_Save(void) {
    TCHAR szPath[MAX_PATH];
    if (!GetStickyNotesPath(szPath, MAX_PATH)) return;
    
    FILE* fp = _tfopen(szPath, TEXT("w"));
    if (!fp) return;
    
    fprintf(fp, "{\n");
    fprintf(fp, "  \"nextNoteId\": %d,\n", g_StickyManager.nNextNoteId);
    fprintf(fp, "  \"stickyNotes\": [\n");
    
    for (int i = 0; i < g_StickyManager.nNoteCount; i++) {
        StickyNote* pNote = &g_StickyManager.notes[i];
        
        /* Get current window position/size */
        if (pNote->hwndNote) {
            RECT rc;
            GetWindowRect(pNote->hwndNote, &rc);
            pNote->nPosX = rc.left;
            pNote->nPosY = rc.top;
            pNote->nWidth = rc.right - rc.left;
            pNote->nHeight = rc.bottom - rc.top;
            
            /* Get content from edit control */
            if (pNote->hwndEdit) {
                GetWindowText(pNote->hwndEdit, pNote->szContent, STICKY_NOTE_MAX_CONTENT);
            }
        }
        
        char escaped[STICKY_NOTE_MAX_CONTENT * 2];
        EscapeJsonStr(pNote->szContent, escaped, sizeof(escaped));
        
        fprintf(fp, "    {\n");
        fprintf(fp, "      \"id\": %d,\n", pNote->nNoteId);
        fprintf(fp, "      \"x\": %d,\n", pNote->nPosX);
        fprintf(fp, "      \"y\": %d,\n", pNote->nPosY);
        fprintf(fp, "      \"width\": %d,\n", pNote->nWidth);
        fprintf(fp, "      \"height\": %d,\n", pNote->nHeight);
        fprintf(fp, "      \"color\": %d,\n", (int)pNote->color);
        fprintf(fp, "      \"visible\": %s,\n", pNote->bVisible ? "true" : "false");
        fprintf(fp, "      \"content\": \"%s\"\n", escaped);
        fprintf(fp, "    }%s\n", (i < g_StickyManager.nNoteCount - 1) ? "," : "");
        
        pNote->bModified = FALSE;
    }
    
    fprintf(fp, "  ]\n");
    fprintf(fp, "}\n");
    fclose(fp);
    
    g_bAutoSavePending = FALSE;
}

/* Helper to unescape JSON string */
static void UnescapeJsonStr(const char* src, TCHAR* dest, int destSize) {
    int j = 0;
    for (int i = 0; src[i] && j < destSize - 1; i++) {
        if (src[i] == '\\' && src[i+1]) {
            i++;
            if (src[i] == '\\') dest[j++] = '\\';
            else if (src[i] == '"') dest[j++] = '"';
            else if (src[i] == 'n') dest[j++] = '\n';
            else if (src[i] == 'r') dest[j++] = '\r';
            else if (src[i] == 't') dest[j++] = '\t';
            else dest[j++] = (TCHAR)src[i];
        } else {
            dest[j++] = (TCHAR)src[i];
        }
    }
    dest[j] = '\0';
}

/* Parse JSON integer value */
static int ParseJsonIntValue(const char* json, const char* key, int def) {
    char searchKey[64];
    sprintf(searchKey, "\"%s\"", key);
    const char* p = strstr(json, searchKey);
    if (!p) return def;
    p = strchr(p, ':');
    if (!p) return def;
    while (*++p == ' ' || *p == '\t');
    return atoi(p);
}

/* Parse JSON boolean value */
static BOOL ParseJsonBoolValue(const char* json, const char* key, BOOL def) {
    char searchKey[64];
    sprintf(searchKey, "\"%s\"", key);
    const char* p = strstr(json, searchKey);
    if (!p) return def;
    p = strchr(p, ':');
    if (!p) return def;
    while (*++p == ' ' || *p == '\t');
    if (strncmp(p, "true", 4) == 0) return TRUE;
    return (strncmp(p, "false", 5) == 0) ? FALSE : def;
}

/* Parse JSON string value */
static void ParseJsonStringValue(const char* json, const char* key, TCHAR* dest, int destSize) {
    char searchKey[64];
    sprintf(searchKey, "\"%s\"", key);
    const char* p = strstr(json, searchKey);
    dest[0] = '\0';
    if (!p) return;
    p = strchr(p, ':');
    if (!p) return;
    while (*++p == ' ' || *p == '\t');
    if (*p != '"') return;
    p++;
    
    char temp[STICKY_NOTE_MAX_CONTENT * 2];
    int i = 0;
    while (*p && *p != '"' && i < (int)sizeof(temp) - 1) {
        if (*p == '\\' && *(p+1)) {
            temp[i++] = *p++;
        }
        temp[i++] = *p++;
    }
    temp[i] = '\0';
    UnescapeJsonStr(temp, dest, destSize);
}

/* Load sticky notes from JSON file */
void StickyNotes_Load(void) {
    if (!g_StickyManager.bInitialized) return;
    
    TCHAR szPath[MAX_PATH];
    if (!GetStickyNotesPath(szPath, MAX_PATH)) return;
    
    FILE* fp = _tfopen(szPath, TEXT("rb"));
    if (!fp) return;
    
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    if (size <= 0 || size > 1024 * 1024) {
        fclose(fp);
        return;
    }
    
    char* json = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size + 1);
    if (!json) {
        fclose(fp);
        return;
    }
    
    fread(json, 1, size, fp);
    fclose(fp);
    json[size] = '\0';
    
    /* Parse nextNoteId */
    g_StickyManager.nNextNoteId = ParseJsonIntValue(json, "nextNoteId", 1);
    
    /* Find stickyNotes array */
    const char* pNotes = strstr(json, "\"stickyNotes\"");
    if (!pNotes) {
        HeapFree(GetProcessHeap(), 0, json);
        return;
    }
    
    pNotes = strchr(pNotes, '[');
    if (!pNotes) {
        HeapFree(GetProcessHeap(), 0, json);
        return;
    }
    pNotes++;
    
    /* Parse each note object */
    while (*pNotes && g_StickyManager.nNoteCount < MAX_STICKY_NOTES) {
        const char* pObj = strchr(pNotes, '{');
        if (!pObj) break;
        
        const char* pObjEnd = strchr(pObj, '}');
        if (!pObjEnd) break;
        
        int objLen = (int)(pObjEnd - pObj + 1);
        char* objStr = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, objLen + 1);
        if (!objStr) break;
        
        strncpy(objStr, pObj, objLen);
        objStr[objLen] = '\0';
        
        int nIndex = g_StickyManager.nNoteCount;
        StickyNote* pNote = &g_StickyManager.notes[nIndex];
        
        pNote->nNoteId = ParseJsonIntValue(objStr, "id", g_StickyManager.nNextNoteId);
        pNote->nPosX = ParseJsonIntValue(objStr, "x", 100);
        pNote->nPosY = ParseJsonIntValue(objStr, "y", 100);
        pNote->nWidth = ParseJsonIntValue(objStr, "width", STICKY_NOTE_DEFAULT_WIDTH);
        pNote->nHeight = ParseJsonIntValue(objStr, "height", STICKY_NOTE_DEFAULT_HEIGHT);
        pNote->color = (StickyNoteColor)ParseJsonIntValue(objStr, "color", STICKY_COLOR_YELLOW);
        pNote->bVisible = ParseJsonBoolValue(objStr, "visible", TRUE);
        ParseJsonStringValue(objStr, "content", pNote->szContent, STICKY_NOTE_MAX_CONTENT);
        pNote->bModified = FALSE;
        
        if (pNote->color < 0 || pNote->color >= STICKY_COLOR_COUNT) {
            pNote->color = STICKY_COLOR_YELLOW;
        }
        
        if (pNote->nWidth < STICKY_NOTE_MIN_WIDTH) pNote->nWidth = STICKY_NOTE_MIN_WIDTH;
        if (pNote->nHeight < STICKY_NOTE_MIN_HEIGHT) pNote->nHeight = STICKY_NOTE_MIN_HEIGHT;
        
        /* Ensure position is on screen */
        RECT rcWork;
        SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, 0);
        if (pNote->nPosX < rcWork.left) pNote->nPosX = rcWork.left + 20;
        if (pNote->nPosY < rcWork.top) pNote->nPosY = rcWork.top + 20;
        if (pNote->nPosX + pNote->nWidth > rcWork.right) 
            pNote->nPosX = rcWork.right - pNote->nWidth - 20;
        if (pNote->nPosY + pNote->nHeight > rcWork.bottom) 
            pNote->nPosY = rcWork.bottom - pNote->nHeight - 20;
        
        pNote->hwndNote = CreateWindowEx(
            WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
            szStickyNoteClass,
            TEXT(""),
            WS_POPUP | WS_THICKFRAME | (pNote->bVisible ? WS_VISIBLE : 0),
            pNote->nPosX, pNote->nPosY,
            pNote->nWidth, pNote->nHeight,
            g_StickyManager.hwndParent,
            NULL,
            g_StickyManager.hInstance,
            (LPVOID)(LONG_PTR)nIndex
        );
        
        if (pNote->hwndNote) {
            g_StickyManager.nNoteCount++;
            if (pNote->nNoteId >= g_StickyManager.nNextNoteId) {
                g_StickyManager.nNextNoteId = pNote->nNoteId + 1;
            }
        }
        
        HeapFree(GetProcessHeap(), 0, objStr);
        pNotes = pObjEnd + 1;
    }
    
    HeapFree(GetProcessHeap(), 0, json);
}


/* Show context menu for color selection and options */
static void ShowColorContextMenu(HWND hwnd, int nIndex, POINT pt) {
    HMENU hMenu = CreatePopupMenu();
    if (!hMenu) return;
    
    /* Color submenu */
    HMENU hColorMenu = CreatePopupMenu();
    for (int i = 0; i < STICKY_COLOR_COUNT; i++) {
        UINT flags = MF_STRING;
        if ((int)g_StickyManager.notes[nIndex].color == i) {
            flags |= MF_CHECKED;
        }
        AppendMenu(hColorMenu, flags, 1000 + i, g_StickyColorNames[i]);
    }
    
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hColorMenu, TEXT("Change Color"));
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenu, MF_STRING, 2001, TEXT("New Note\tCtrl+Shift+N"));
    AppendMenu(hMenu, MF_STRING, 2003, TEXT("Manage Notes...\tCtrl+Shift+M"));
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenu, MF_STRING, 2002, TEXT("Hide Note"));
    AppendMenu(hMenu, MF_STRING, 2000, TEXT("Delete Note Permanently"));
    
    ClientToScreen(hwnd, &pt);
    int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_RIGHTBUTTON,
                             pt.x, pt.y, 0, hwnd, NULL);
    
    if (cmd >= 1000 && cmd < 1000 + STICKY_COLOR_COUNT) {
        StickyNotes_SetColor(nIndex, (StickyNoteColor)(cmd - 1000));
    } else if (cmd == 2000) {
        /* Confirm delete */
        int result = MessageBox(hwnd, 
            TEXT("Are you sure you want to permanently delete this note?\nThis cannot be undone."),
            TEXT("Delete Sticky Note"),
            MB_YESNO | MB_ICONWARNING);
        if (result == IDYES) {
            StickyNotes_Delete(nIndex);
        }
    } else if (cmd == 2001) {
        StickyNotes_Create();
    } else if (cmd == 2002) {
        StickyNotes_Hide(nIndex);
    } else if (cmd == 2003) {
        StickyNotes_ShowManageDialog(g_StickyManager.hwndParent);
    }
    
    DestroyMenu(hColorMenu);
    DestroyMenu(hMenu);
}

/* Sticky note window procedure */
LRESULT CALLBACK StickyNoteWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    int nIndex;
    
    switch (msg) {
        case WM_CREATE: {
            CREATESTRUCT* pcs = (CREATESTRUCT*)lParam;
            nIndex = (int)(LONG_PTR)pcs->lpCreateParams;
            SetWindowLongPtr(hwnd, 0, (LONG_PTR)nIndex);
            
            StickyNote* pNote = &g_StickyManager.notes[nIndex];
            
            /* Create edit control with better styling */
            pNote->hwndEdit = CreateWindowEx(
                0,
                TEXT("EDIT"),
                pNote->szContent,
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
                4, STICKY_NOTE_TITLE_HEIGHT + 4,
                pNote->nWidth - 10,
                pNote->nHeight - STICKY_NOTE_TITLE_HEIGHT - 10,
                hwnd,
                (HMENU)1001,
                g_StickyManager.hInstance,
                NULL
            );
            
            if (pNote->hwndEdit) {
                /* Create a nicer font */
                HFONT hFont = CreateFont(
                    -14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                    CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS,
                    TEXT("Segoe UI")
                );
                if (hFont) {
                    SendMessage(pNote->hwndEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
                }
                
                /* Set text limit */
                SendMessage(pNote->hwndEdit, EM_SETLIMITTEXT, STICKY_NOTE_MAX_CONTENT - 1, 0);
            }
            
            return 0;
        }
        
        case WM_SIZE: {
            nIndex = (int)GetWindowLongPtr(hwnd, 0);
            if (nIndex >= 0 && nIndex < g_StickyManager.nNoteCount) {
                StickyNote* pNote = &g_StickyManager.notes[nIndex];
                int width = LOWORD(lParam);
                int height = HIWORD(lParam);
                
                if (pNote->hwndEdit) {
                    MoveWindow(pNote->hwndEdit, 4, STICKY_NOTE_TITLE_HEIGHT + 4,
                               width - 10, height - STICKY_NOTE_TITLE_HEIGHT - 10, TRUE);
                }
                
                StickyNotes_MarkModified(nIndex);
            }
            return 0;
        }
        
        case WM_GETMINMAXINFO: {
            MINMAXINFO* pmmi = (MINMAXINFO*)lParam;
            pmmi->ptMinTrackSize.x = STICKY_NOTE_MIN_WIDTH;
            pmmi->ptMinTrackSize.y = STICKY_NOTE_MIN_HEIGHT;
            return 0;
        }
        
        case WM_PAINT: {
            nIndex = (int)GetWindowLongPtr(hwnd, 0);
            if (nIndex < 0 || nIndex >= g_StickyManager.nNoteCount) break;
            
            StickyNote* pNote = &g_StickyManager.notes[nIndex];
            COLORREF crBg = StickyNotes_GetColorRGB(pNote->color);
            COLORREF crTitle = GetTitleBarColor(pNote->color);
            
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT rcClient;
            GetClientRect(hwnd, &rcClient);
            
            /* Fill background */
            HBRUSH hBrush = CreateSolidBrush(crBg);
            FillRect(hdc, &rcClient, hBrush);
            DeleteObject(hBrush);
            
            /* Draw title bar with gradient effect */
            RECT rcTitle = rcClient;
            rcTitle.bottom = STICKY_NOTE_TITLE_HEIGHT;
            
            hBrush = CreateSolidBrush(crTitle);
            FillRect(hdc, &rcTitle, hBrush);
            DeleteObject(hBrush);
            
            /* Draw bottom border of title bar */
            HPEN hPen = CreatePen(PS_SOLID, 1, RGB(
                GetRValue(crTitle) * 8 / 10,
                GetGValue(crTitle) * 8 / 10,
                GetBValue(crTitle) * 8 / 10
            ));
            HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
            MoveToEx(hdc, 0, STICKY_NOTE_TITLE_HEIGHT - 1, NULL);
            LineTo(hdc, rcClient.right, STICKY_NOTE_TITLE_HEIGHT - 1);
            SelectObject(hdc, hOldPen);
            DeleteObject(hPen);
            
            /* Draw title text */
            TCHAR szTitle[64];
            wsprintf(szTitle, TEXT("📝 Note %d"), pNote->nNoteId);
            
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(50, 50, 50));
            
            HFONT hFont = CreateFont(
                -13, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS,
                TEXT("Segoe UI")
            );
            HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
            
            RECT rcText = rcTitle;
            rcText.left += 8;
            rcText.right -= STICKY_NOTE_CLOSE_BTN_SIZE + 12;
            DrawText(hdc, szTitle, -1, &rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            
            /* Draw close button */
            RECT rcClose;
            rcClose.right = rcClient.right - 6;
            rcClose.left = rcClose.right - STICKY_NOTE_CLOSE_BTN_SIZE;
            rcClose.top = (STICKY_NOTE_TITLE_HEIGHT - STICKY_NOTE_CLOSE_BTN_SIZE) / 2;
            rcClose.bottom = rcClose.top + STICKY_NOTE_CLOSE_BTN_SIZE;
            
            /* Hover effect for close button */
            BOOL bHover = (g_nHoverCloseNote == nIndex);
            if (bHover) {
                HBRUSH hCloseBrush = CreateSolidBrush(RGB(255, 100, 100));
                RECT rcCloseBg = rcClose;
                InflateRect(&rcCloseBg, 2, 2);
                FillRect(hdc, &rcCloseBg, hCloseBrush);
                DeleteObject(hCloseBrush);
            }
            
            /* Draw X */
            HPEN hClosePen = CreatePen(PS_SOLID, 2, bHover ? RGB(255, 255, 255) : RGB(80, 80, 80));
            HPEN hOldClosePen = (HPEN)SelectObject(hdc, hClosePen);
            
            MoveToEx(hdc, rcClose.left + 4, rcClose.top + 4, NULL);
            LineTo(hdc, rcClose.right - 4, rcClose.bottom - 4);
            MoveToEx(hdc, rcClose.right - 4, rcClose.top + 4, NULL);
            LineTo(hdc, rcClose.left + 4, rcClose.bottom - 4);
            
            SelectObject(hdc, hOldClosePen);
            DeleteObject(hClosePen);
            
            SelectObject(hdc, hOldFont);
            DeleteObject(hFont);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_CTLCOLOREDIT: {
            nIndex = (int)GetWindowLongPtr(hwnd, 0);
            if (nIndex >= 0 && nIndex < g_StickyManager.nNoteCount) {
                HDC hdcEdit = (HDC)wParam;
                COLORREF crBg = StickyNotes_GetColorRGB(g_StickyManager.notes[nIndex].color);
                SetBkColor(hdcEdit, crBg);
                SetTextColor(hdcEdit, RGB(30, 30, 30));
                
                static HBRUSH hBrushEdit = NULL;
                if (hBrushEdit) DeleteObject(hBrushEdit);
                hBrushEdit = CreateSolidBrush(crBg);
                return (LRESULT)hBrushEdit;
            }
            break;
        }
        
        case WM_MOUSEMOVE: {
            nIndex = (int)GetWindowLongPtr(hwnd, 0);
            POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            
            /* Check if hovering over close button */
            RECT rcClient;
            GetClientRect(hwnd, &rcClient);
            RECT rcClose;
            rcClose.right = rcClient.right - 6;
            rcClose.left = rcClose.right - STICKY_NOTE_CLOSE_BTN_SIZE;
            rcClose.top = (STICKY_NOTE_TITLE_HEIGHT - STICKY_NOTE_CLOSE_BTN_SIZE) / 2;
            rcClose.bottom = rcClose.top + STICKY_NOTE_CLOSE_BTN_SIZE;
            InflateRect(&rcClose, 4, 4);
            
            int nOldHover = g_nHoverCloseNote;
            if (pt.y < STICKY_NOTE_TITLE_HEIGHT && PtInRect(&rcClose, pt)) {
                g_nHoverCloseNote = nIndex;
            } else {
                if (g_nHoverCloseNote == nIndex) g_nHoverCloseNote = -1;
            }
            
            if (nOldHover != g_nHoverCloseNote) {
                InvalidateRect(hwnd, NULL, FALSE);
            }
            
            /* Track mouse leave */
            TRACKMOUSEEVENT tme = {sizeof(tme), TME_LEAVE, hwnd, 0};
            TrackMouseEvent(&tme);
            break;
        }
        
        case WM_MOUSELEAVE: {
            nIndex = (int)GetWindowLongPtr(hwnd, 0);
            if (g_nHoverCloseNote == nIndex) {
                g_nHoverCloseNote = -1;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        }
        
        case WM_NCHITTEST: {
            LRESULT hit = DefWindowProc(hwnd, msg, wParam, lParam);
            if (hit == HTCLIENT) {
                POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
                ScreenToClient(hwnd, &pt);
                if (pt.y < STICKY_NOTE_TITLE_HEIGHT) {
                    RECT rcClient;
                    GetClientRect(hwnd, &rcClient);
                    if (pt.x > rcClient.right - STICKY_NOTE_CLOSE_BTN_SIZE - 12) {
                        return HTCLIENT;
                    }
                    return HTCAPTION;
                }
            }
            return hit;
        }
        
        case WM_LBUTTONDOWN: {
            POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            if (pt.y < STICKY_NOTE_TITLE_HEIGHT) {
                RECT rcClient;
                GetClientRect(hwnd, &rcClient);
                if (pt.x > rcClient.right - STICKY_NOTE_CLOSE_BTN_SIZE - 12) {
                    nIndex = (int)GetWindowLongPtr(hwnd, 0);
                    /* Hide instead of delete when clicking X */
                    StickyNotes_Hide(nIndex);
                    return 0;
                }
            }
            break;
        }
        
        case WM_RBUTTONUP: {
            nIndex = (int)GetWindowLongPtr(hwnd, 0);
            POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            ShowColorContextMenu(hwnd, nIndex, pt);
            return 0;
        }
        
        case WM_COMMAND: {
            if (HIWORD(wParam) == EN_CHANGE) {
                nIndex = (int)GetWindowLongPtr(hwnd, 0);
                StickyNotes_MarkModified(nIndex);
            }
            break;
        }
        
        case WM_MOVE: {
            nIndex = (int)GetWindowLongPtr(hwnd, 0);
            StickyNotes_MarkModified(nIndex);
            break;
        }
        
        case WM_ACTIVATE: {
            if (LOWORD(wParam) != WA_INACTIVE) {
                /* Bring to top when activated */
                SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, 
                            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            }
            break;
        }
        
        case WM_DESTROY: {
            return 0;
        }
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

/* ============================================ */
/* Manage Sticky Notes Dialog Implementation   */
/* ============================================ */

static HWND g_hwndManageList = NULL;

/* Populate the list view with all sticky notes */
static void PopulateManageList(HWND hwndList) {
    ListView_DeleteAllItems(hwndList);
    
    for (int i = 0; i < g_StickyManager.nNoteCount; i++) {
        StickyNote* pNote = &g_StickyManager.notes[i];
        
        LVITEM lvi = {0};
        lvi.mask = LVIF_TEXT | LVIF_PARAM;
        lvi.iItem = i;
        lvi.iSubItem = 0;
        lvi.lParam = (LPARAM)i;
        
        /* Note ID */
        TCHAR szId[16];
        wsprintf(szId, TEXT("%d"), pNote->nNoteId);
        lvi.pszText = szId;
        ListView_InsertItem(hwndList, &lvi);
        
        /* Preview (first 30 chars) */
        TCHAR szPreview[35];
        const TCHAR* content = pNote->szContent;
        int len = (int)_tcslen(content);
        if (len > 30) {
            _tcsncpy(szPreview, content, 30);
            szPreview[30] = TEXT('\0');
            _tcscat(szPreview, TEXT("..."));
        } else if (len > 0) {
            _tcscpy(szPreview, content);
        } else {
            _tcscpy(szPreview, TEXT("(empty)"));
        }
        /* Replace newlines */
        for (TCHAR* p = szPreview; *p; p++) {
            if (*p == TEXT('\n') || *p == TEXT('\r')) *p = TEXT(' ');
        }
        ListView_SetItemText(hwndList, i, 1, szPreview);
        
        /* Color indicator */
        const TCHAR* colorNames[] = {TEXT("Yellow"), TEXT("Pink"), TEXT("Blue"), 
                                      TEXT("Green"), TEXT("Orange"), TEXT("Purple")};
        ListView_SetItemText(hwndList, i, 2, (TCHAR*)colorNames[pNote->color]);
        
        /* Status */
        ListView_SetItemText(hwndList, i, 3, pNote->bVisible ? TEXT("Shown") : TEXT("Hidden"));
    }
}

/* Update status text in dialog */
static void UpdateManageDialogStatus(HWND hwndDlg) {
    TCHAR szStatus[64];
    int total = g_StickyManager.nNoteCount;
    int hidden = StickyNotes_GetHiddenCount();
    wsprintf(szStatus, TEXT("Total: %d notes, %d hidden"), total, hidden);
    SetDlgItemText(hwndDlg, 1400, szStatus);
}

/* Dialog procedure for Manage Sticky Notes */
static INT_PTR CALLBACK ManageStickyNotesDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_INITDIALOG: {
            g_hwndManageList = GetDlgItem(hwndDlg, IDC_STICKYNOTES_LIST);
            
            /* Set up list view columns */
            LVCOLUMN lvc = {0};
            lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
            
            lvc.iSubItem = 0;
            lvc.pszText = TEXT("ID");
            lvc.cx = 40;
            ListView_InsertColumn(g_hwndManageList, 0, &lvc);
            
            lvc.iSubItem = 1;
            lvc.pszText = TEXT("Preview");
            lvc.cx = 150;
            ListView_InsertColumn(g_hwndManageList, 1, &lvc);
            
            lvc.iSubItem = 2;
            lvc.pszText = TEXT("Color");
            lvc.cx = 55;
            ListView_InsertColumn(g_hwndManageList, 2, &lvc);
            
            lvc.iSubItem = 3;
            lvc.pszText = TEXT("Status");
            lvc.cx = 50;
            ListView_InsertColumn(g_hwndManageList, 3, &lvc);
            
            /* Enable full row select */
            ListView_SetExtendedListViewStyle(g_hwndManageList, 
                LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
            
            PopulateManageList(g_hwndManageList);
            UpdateManageDialogStatus(hwndDlg);
            
            /* Center dialog */
            RECT rcDlg, rcParent;
            GetWindowRect(hwndDlg, &rcDlg);
            GetWindowRect(GetParent(hwndDlg), &rcParent);
            int x = rcParent.left + (rcParent.right - rcParent.left - (rcDlg.right - rcDlg.left)) / 2;
            int y = rcParent.top + (rcParent.bottom - rcParent.top - (rcDlg.bottom - rcDlg.top)) / 2;
            SetWindowPos(hwndDlg, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
            
            return TRUE;
        }
        
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case IDC_BTN_SHOWHIDE: {
                    int sel = ListView_GetNextItem(g_hwndManageList, -1, LVNI_SELECTED);
                    if (sel >= 0) {
                        LVITEM lvi = {0};
                        lvi.mask = LVIF_PARAM;
                        lvi.iItem = sel;
                        ListView_GetItem(g_hwndManageList, &lvi);
                        int nIndex = (int)lvi.lParam;
                        
                        StickyNotes_ToggleVisibility(nIndex);
                        PopulateManageList(g_hwndManageList);
                        UpdateManageDialogStatus(hwndDlg);
                        ListView_SetItemState(g_hwndManageList, sel, LVIS_SELECTED, LVIS_SELECTED);
                    }
                    return TRUE;
                }
                
                case IDC_BTN_DELETENOTE: {
                    int sel = ListView_GetNextItem(g_hwndManageList, -1, LVNI_SELECTED);
                    if (sel >= 0) {
                        int result = MessageBox(hwndDlg, 
                            TEXT("Are you sure you want to permanently delete this note?"),
                            TEXT("Delete Sticky Note"),
                            MB_YESNO | MB_ICONQUESTION);
                        if (result == IDYES) {
                            LVITEM lvi = {0};
                            lvi.mask = LVIF_PARAM;
                            lvi.iItem = sel;
                            ListView_GetItem(g_hwndManageList, &lvi);
                            int nIndex = (int)lvi.lParam;
                            
                            StickyNotes_Delete(nIndex);
                            PopulateManageList(g_hwndManageList);
                            UpdateManageDialogStatus(hwndDlg);
                        }
                    }
                    return TRUE;
                }
                
                case IDC_BTN_NEWNOTE: {
                    StickyNotes_Create();
                    PopulateManageList(g_hwndManageList);
                    UpdateManageDialogStatus(hwndDlg);
                    return TRUE;
                }
                
                case IDCANCEL:
                    EndDialog(hwndDlg, IDCANCEL);
                    return TRUE;
            }
            break;
        }
        
        case WM_NOTIFY: {
            NMHDR* pnmh = (NMHDR*)lParam;
            if (pnmh->idFrom == IDC_STICKYNOTES_LIST) {
                if (pnmh->code == NM_DBLCLK) {
                    /* Double-click to show/restore note */
                    int sel = ListView_GetNextItem(g_hwndManageList, -1, LVNI_SELECTED);
                    if (sel >= 0) {
                        LVITEM lvi = {0};
                        lvi.mask = LVIF_PARAM;
                        lvi.iItem = sel;
                        ListView_GetItem(g_hwndManageList, &lvi);
                        int nIndex = (int)lvi.lParam;
                        
                        StickyNote* pNote = StickyNotes_GetNote(nIndex);
                        if (pNote) {
                            if (!pNote->bVisible) {
                                StickyNotes_Restore(nIndex);
                            } else if (pNote->hwndNote) {
                                SetForegroundWindow(pNote->hwndNote);
                                if (pNote->hwndEdit) {
                                    SetFocus(pNote->hwndEdit);
                                }
                            }
                            PopulateManageList(g_hwndManageList);
                            UpdateManageDialogStatus(hwndDlg);
                            ListView_SetItemState(g_hwndManageList, sel, LVIS_SELECTED, LVIS_SELECTED);
                        }
                    }
                }
            }
            break;
        }
        
        case WM_CLOSE:
            EndDialog(hwndDlg, IDCANCEL);
            return TRUE;
    }
    
    return FALSE;
}

/* Show the manage sticky notes dialog */
void StickyNotes_ShowManageDialog(HWND hwndParent) {
    DialogBox(g_StickyManager.hInstance, MAKEINTRESOURCE(IDD_MANAGE_STICKYNOTES), 
              hwndParent, ManageStickyNotesDlgProc);
}
