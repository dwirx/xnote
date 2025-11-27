/**
 * performance.h - Advanced Performance Optimization for XNote
 * 
 * Provides high-performance features including:
 * - Asynchronous I/O with IOCP (I/O Completion Ports)
 * - DirectWrite text rendering with hardware acceleration
 * - Memory pool allocator for reduced fragmentation
 * - Background search indexing with Boyer-Moore algorithm
 * - Incremental syntax highlighting
 * - Smooth scrolling with easing
 * - Auto-save manager
 * - Performance monitoring
 */

#ifndef PERFORMANCE_H
#define PERFORMANCE_H

#include <windows.h>
#include <psapi.h>

/* Forward declarations for DirectWrite/Direct2D types */
/* These are opaque pointers - actual types defined in d2d1.h/dwrite.h */
typedef struct ID2D1Factory ID2D1Factory;
typedef struct IDWriteFactory IDWriteFactory;
typedef struct IDWriteTextFormat IDWriteTextFormat;
typedef struct ID2D1HwndRenderTarget ID2D1HwndRenderTarget;
typedef struct ID2D1SolidColorBrush ID2D1SolidColorBrush;
typedef struct IDWriteTextLayout IDWriteTextLayout;
typedef struct ID2D1Brush ID2D1Brush;

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * Constants
 *============================================================================*/

/* Memory Pool Constants */
#define MEMPOOL_MAX_SIZE        (500 * 1024 * 1024)  /* 500MB max */
#define MEMPOOL_INITIAL_SIZE    (64 * 1024 * 1024)   /* 64MB initial */
#define MEMPOOL_BLOCK_ALIGN     16                    /* 16-byte alignment */
#define MEMPOOL_WARN_THRESHOLD  (400 * 1024 * 1024)  /* 400MB warning */

/* IOCP Constants */
#define IOCP_THREAD_COUNT       4                     /* Worker threads */
#define IOCP_READ_BUFFER_SIZE   (64 * 1024)          /* 64KB read buffer */

/* Scroll Constants */
#define SCROLL_ANIMATION_MS     150                   /* Animation duration */
#define SCROLL_PREDICT_LINES    10                    /* Lines to pre-render */
#define SCROLL_FPS_TARGET       60                    /* Target frame rate */

/* Auto-save Constants */
#define AUTOSAVE_INTERVAL_MS    60000                 /* 60 seconds */
#define AUTOSAVE_DELAY_MS       3000                  /* 3 seconds after typing */

/* Performance Thresholds */
#define PERF_SLOW_OP_MS         100                   /* Slow operation threshold */
#define PERF_MEMORY_WARN_MB     400                   /* Memory warning threshold */
#define PERF_SYNTAX_TIME_MS     16                    /* Max syntax highlight time */

/* Search Constants */
#define SEARCH_MAX_RESULTS      10000                 /* Max search results */

/* Custom Window Messages for Async Operations */
#define WM_ASYNC_LOAD_COMPLETE  (WM_USER + 100)
#define WM_ASYNC_SAVE_COMPLETE  (WM_USER + 101)
#define WM_AUTOSAVE_COMPLETE    (WM_USER + 102)
#define WM_INDEX_BUILD_COMPLETE (WM_USER + 103)

/*============================================================================
 * Error Codes
 *============================================================================*/

typedef enum {
    PERFERR_SUCCESS = 0,
    PERFERR_DWRITE_INIT_FAILED,
    PERFERR_D2D_INIT_FAILED,
    PERFERR_IOCP_INIT_FAILED,
    PERFERR_MEMPOOL_EXHAUSTED,
    PERFERR_AUTOSAVE_FAILED,
    PERFERR_INDEX_BUILD_FAILED,
    PERFERR_THREAD_CREATE_FAILED,
    PERFERR_INVALID_PARAM
} PerfErrorCode;

/*============================================================================
 * Memory Pool Allocator
 *============================================================================*/

/* Memory pool block */
typedef struct MemBlock {
    struct MemBlock* pNext;     /* Next free block */
    DWORD dwSize;               /* Block size */
    BYTE data[1];               /* Flexible array member */
} MemBlock;

/* Memory pool */
typedef struct {
    MemBlock* pFreeList;        /* Free block list */
    LPVOID pPoolBase;           /* Pool base address */
    DWORD dwPoolSize;           /* Total pool size */
    DWORD dwUsedSize;           /* Currently used size */
    DWORD dwMaxSize;            /* Maximum allowed size (500MB) */
    DWORD dwAllocCount;         /* Number of allocations */
    CRITICAL_SECTION cs;        /* Thread safety */
    BOOL bInitialized;          /* Initialization flag */
} MemoryPool;

/* Memory pool functions */
BOOL MemPool_Initialize(MemoryPool* pPool);
void MemPool_Shutdown(MemoryPool* pPool);
LPVOID MemPool_Alloc(MemoryPool* pPool, DWORD dwSize);
void MemPool_Free(MemoryPool* pPool, LPVOID pMem);
DWORD MemPool_GetUsage(MemoryPool* pPool);
BOOL MemPool_IsNearLimit(MemoryPool* pPool);
void MemPool_GetStats(MemoryPool* pPool, DWORD* pdwUsed, DWORD* pdwTotal, DWORD* pdwAllocCount);

/*============================================================================
 * Asynchronous I/O Manager (IOCP)
 *============================================================================*/

/* Async operation types */
typedef enum {
    ASYNC_OP_READ = 0,
    ASYNC_OP_WRITE
} AsyncOpType;

/* Async file operation structure */
typedef struct {
    OVERLAPPED overlapped;      /* Must be first for IOCP */
    HANDLE hFile;               /* File handle */
    LPVOID pBuffer;             /* Data buffer */
    DWORD dwBufferSize;         /* Buffer size */
    DWORD dwBytesTransferred;   /* Bytes read/written */
    HWND hwndNotify;            /* Window to notify on completion */
    UINT uMsgComplete;          /* Message to post on completion */
    void* pUserData;            /* User context data */
    AsyncOpType opType;         /* Operation type */
    TCHAR szFileName[MAX_PATH]; /* File name for reference */
    BOOL bSuccess;              /* Operation success flag */
    DWORD dwError;              /* Error code if failed */
} AsyncFileOp;

/* I/O Completion Port Manager */
typedef struct {
    HANDLE hIOCP;               /* I/O Completion Port handle */
    HANDLE hThreadPool[IOCP_THREAD_COUNT]; /* Worker threads */
    volatile BOOL bShutdown;    /* Shutdown flag */
    CRITICAL_SECTION cs;        /* Thread safety */
    BOOL bInitialized;          /* Initialization flag */
} IOCPManager;

/* IOCP functions */
BOOL IOCP_Initialize(IOCPManager* pMgr);
void IOCP_Shutdown(IOCPManager* pMgr);
BOOL IOCP_QueueRead(IOCPManager* pMgr, AsyncFileOp* pOp);
BOOL IOCP_QueueWrite(IOCPManager* pMgr, AsyncFileOp* pOp);
DWORD WINAPI IOCP_WorkerThread(LPVOID lpParam);
AsyncFileOp* IOCP_CreateOp(const TCHAR* szFileName, HWND hwndNotify, UINT uMsg, void* pUserData);
void IOCP_FreeOp(AsyncFileOp* pOp);

/*============================================================================
 * DirectWrite Text Renderer
 *============================================================================*/

/* DirectWrite renderer state */
typedef struct {
    ID2D1Factory* pD2DFactory;
    IDWriteFactory* pDWriteFactory;
    IDWriteTextFormat* pTextFormat;
    ID2D1HwndRenderTarget* pRenderTarget;
    ID2D1SolidColorBrush* pTextBrush;
    ID2D1SolidColorBrush* pBgBrush;
    BOOL bHardwareAccelerated;
    BOOL bInitialized;
    HWND hwnd;
    WCHAR szFontName[64];
    float fFontSize;
} DWriteRenderer;

/* DirectWrite functions */
BOOL DWrite_Initialize(DWriteRenderer* pRenderer, HWND hwnd);
void DWrite_Shutdown(DWriteRenderer* pRenderer);
BOOL DWrite_SetFont(DWriteRenderer* pRenderer, const WCHAR* szFontName, float fSize);
void DWrite_RenderText(DWriteRenderer* pRenderer, const WCHAR* szText, DWORD dwLen, RECT* pRect);
void DWrite_RenderLine(DWriteRenderer* pRenderer, const WCHAR* szText, DWORD dwLen, int x, int y);
BOOL DWrite_IsAvailable(void);
void DWrite_Resize(DWriteRenderer* pRenderer, int nWidth, int nHeight);
void DWrite_SetTextColor(DWriteRenderer* pRenderer, COLORREF cr);
void DWrite_SetBgColor(DWriteRenderer* pRenderer, COLORREF cr);

/*============================================================================
 * Background Search Indexer
 *============================================================================*/

/* Search match entry */
typedef struct {
    DWORD dwLineNumber;         /* Line number (1-based) */
    DWORD dwOffset;             /* Character offset in line */
    DWORD dwLength;             /* Match length */
    DWORD dwAbsolutePos;        /* Absolute position in text */
} SearchMatch;

/* Search index */
typedef struct {
    WCHAR* pText;               /* Indexed text (copy) */
    DWORD dwTextLen;            /* Text length */
    DWORD* pLineOffsets;        /* Line start offsets */
    DWORD dwLineCount;          /* Number of lines */
    BOOL bIndexReady;           /* Index built flag */
    HANDLE hBuildThread;        /* Background build thread */
    volatile BOOL bBuilding;    /* Currently building */
    volatile BOOL bCancelBuild; /* Cancel build flag */
    HWND hwndNotify;            /* Window to notify on completion */
    CRITICAL_SECTION cs;        /* Thread safety */
    BOOL bInitialized;          /* Initialization flag */
} SearchIndex;

/* Boyer-Moore search state */
typedef struct {
    int badChar[65536];         /* Bad character table (Unicode) */
    int* goodSuffix;            /* Good suffix table */
    WCHAR* pPattern;            /* Search pattern */
    DWORD dwPatternLen;         /* Pattern length */
    BOOL bCaseSensitive;        /* Case sensitive search */
} BoyerMooreState;

/* Search functions */
BOOL SearchIndex_Initialize(SearchIndex* pIndex);
void SearchIndex_Shutdown(SearchIndex* pIndex);
BOOL SearchIndex_Build(SearchIndex* pIndex, const WCHAR* pText, DWORD dwLen, HWND hwndNotify);
BOOL SearchIndex_BuildAsync(SearchIndex* pIndex, const WCHAR* pText, DWORD dwLen, HWND hwndNotify);
BOOL SearchIndex_Update(SearchIndex* pIndex, DWORD dwStart, DWORD dwOldLen, const WCHAR* pNewText, DWORD dwNewLen);
SearchMatch* SearchIndex_Find(SearchIndex* pIndex, const WCHAR* pPattern, BOOL bCaseSensitive, DWORD* pdwCount);
void SearchIndex_FreeResults(SearchMatch* pMatches);
DWORD SearchIndex_GetLineFromPos(SearchIndex* pIndex, DWORD dwPos);

/* Boyer-Moore functions */
void BoyerMoore_Init(BoyerMooreState* pState, const WCHAR* pPattern, BOOL bCaseSensitive);
void BoyerMoore_Shutdown(BoyerMooreState* pState);
int BoyerMoore_Search(BoyerMooreState* pState, const WCHAR* pText, DWORD dwTextLen, DWORD dwStartPos);


/*============================================================================
 * Incremental Syntax Highlighter
 *============================================================================*/

/* Syntax highlight style flags */
#define HL_STYLE_NORMAL     0x00
#define HL_STYLE_BOLD       0x01
#define HL_STYLE_ITALIC     0x02
#define HL_STYLE_UNDERLINE  0x04

/* Syntax highlight range */
typedef struct {
    DWORD dwStart;              /* Start offset */
    DWORD dwEnd;                /* End offset */
    COLORREF crColor;           /* Text color */
    DWORD dwStyle;              /* Font style (bold, italic) */
} HighlightRange;

/* Incremental highlighter state */
typedef struct {
    HighlightRange* pRanges;    /* Highlight ranges */
    DWORD dwRangeCount;         /* Number of ranges */
    DWORD dwRangeCapacity;      /* Allocated capacity */
    DWORD dwFirstDirtyLine;     /* First line needing update */
    DWORD dwLastDirtyLine;      /* Last line needing update */
    BOOL bFullRefreshNeeded;    /* Full refresh flag */
    HANDLE hWorkerThread;       /* Background worker thread */
    HANDLE hWorkEvent;          /* Work available event */
    HANDLE hStopEvent;          /* Stop event for shutdown */
    volatile BOOL bShutdown;    /* Shutdown flag */
    volatile BOOL bProcessing;  /* Currently processing */
    HWND hwndEdit;              /* Associated edit control */
    CRITICAL_SECTION cs;        /* Thread safety */
    BOOL bInitialized;          /* Initialization flag */
    DWORD dwLastProcessTime;    /* Last processing time in ms */
} IncrementalHighlighter;

/* Highlighter functions */
BOOL Highlighter_Initialize(IncrementalHighlighter* pH, HWND hwndEdit);
void Highlighter_Shutdown(IncrementalHighlighter* pH);
void Highlighter_MarkDirty(IncrementalHighlighter* pH, DWORD dwLine);
void Highlighter_MarkRangeDirty(IncrementalHighlighter* pH, DWORD dwStartLine, DWORD dwEndLine);
BOOL Highlighter_ProcessVisible(IncrementalHighlighter* pH, DWORD dwFirstVisible, DWORD dwLastVisible);
void Highlighter_TriggerUpdate(IncrementalHighlighter* pH);
DWORD Highlighter_GetLastProcessTime(IncrementalHighlighter* pH);
DWORD WINAPI Highlighter_WorkerThread(LPVOID lpParam);

/*============================================================================
 * Smooth Scroll Manager
 *============================================================================*/

/* Scroll animation state */
typedef struct {
    int nTargetPos;             /* Target scroll position */
    int nStartPos;              /* Start scroll position */
    int nCurrentPos;            /* Current scroll position */
    float fVelocity;            /* Current velocity */
    DWORD dwStartTime;          /* Animation start time */
    DWORD dwDuration;           /* Animation duration */
    BOOL bAnimating;            /* Animation in progress */
} ScrollAnimation;

/* Smooth scroll manager */
typedef struct {
    ScrollAnimation vertical;    /* Vertical scroll state */
    ScrollAnimation horizontal;  /* Horizontal scroll state */
    BOOL bHardwareAccelerated;  /* Using D2D */
    BOOL bEnabled;              /* Smooth scroll enabled */
    int nPredictedLines;        /* Pre-rendered lines */
    HWND hwnd;                  /* Associated window */
    UINT_PTR nTimerId;          /* Animation timer ID */
    BOOL bInitialized;          /* Initialization flag */
} SmoothScrollManager;

/* Easing function type */
typedef float (*EasingFunc)(float t);

/* Smooth scroll functions */
BOOL SmoothScroll_Initialize(SmoothScrollManager* pMgr, HWND hwnd);
void SmoothScroll_Shutdown(SmoothScrollManager* pMgr);
void SmoothScroll_ScrollTo(SmoothScrollManager* pMgr, int nTargetY, BOOL bAnimate);
void SmoothScroll_ScrollBy(SmoothScrollManager* pMgr, int nDeltaY, BOOL bAnimate);
void SmoothScroll_Update(SmoothScrollManager* pMgr);
BOOL SmoothScroll_IsAnimating(SmoothScrollManager* pMgr);
void SmoothScroll_Stop(SmoothScrollManager* pMgr);
void SmoothScroll_SetEnabled(SmoothScrollManager* pMgr, BOOL bEnabled);

/* Easing functions */
float Easing_Linear(float t);
float Easing_EaseOutCubic(float t);
float Easing_EaseOutQuad(float t);
float Easing_EaseInOutCubic(float t);

/*============================================================================
 * Auto-Save Manager
 *============================================================================*/

/* Auto-save status */
typedef enum {
    AUTOSAVE_IDLE = 0,
    AUTOSAVE_PENDING,
    AUTOSAVE_SAVING,
    AUTOSAVE_SUCCESS,
    AUTOSAVE_FAILED
} AutoSaveStatus;

/* Auto-save state */
typedef struct {
    HANDLE hThread;             /* Background save thread */
    HANDLE hWakeEvent;          /* Wake event for save */
    HANDLE hStopEvent;          /* Stop event for shutdown */
    DWORD dwIntervalMs;         /* Save interval (60000ms) */
    DWORD dwDelayAfterTypeMs;   /* Delay after typing (3000ms) */
    DWORD dwLastKeystroke;      /* Last keystroke time */
    volatile BOOL bPendingSave; /* Save pending flag */
    volatile BOOL bSaving;      /* Currently saving flag */
    volatile AutoSaveStatus status; /* Current status */
    HWND hwndNotify;            /* Window to notify */
    TCHAR szLastError[256];     /* Last error message */
    CRITICAL_SECTION cs;        /* Thread safety */
    BOOL bEnabled;              /* Auto-save enabled */
    BOOL bInitialized;          /* Initialization flag */
} AutoSaveManager;

/* Auto-save functions */
BOOL AutoSave_Initialize(AutoSaveManager* pMgr, HWND hwndNotify);
void AutoSave_Shutdown(AutoSaveManager* pMgr);
void AutoSave_OnKeystroke(AutoSaveManager* pMgr);
void AutoSave_TriggerNow(AutoSaveManager* pMgr);
void AutoSave_SetEnabled(AutoSaveManager* pMgr, BOOL bEnabled);
AutoSaveStatus AutoSave_GetStatus(AutoSaveManager* pMgr);
const TCHAR* AutoSave_GetLastError(AutoSaveManager* pMgr);
DWORD WINAPI AutoSave_WorkerThread(LPVOID lpParam);

/*============================================================================
 * Performance Monitor
 *============================================================================*/

/* Performance metrics */
typedef struct {
    DWORD dwFrameCount;         /* Frames rendered */
    DWORD dwLastFPSUpdate;      /* Last FPS calculation time */
    float fCurrentFPS;          /* Current FPS */
    float fAverageFPS;          /* Average FPS */
    DWORD dwMemoryUsage;        /* Current memory usage */
    DWORD dwPeakMemory;         /* Peak memory usage */
    BOOL bDebugMode;            /* Debug mode enabled */
    BOOL bShowFPS;              /* Show FPS in status bar */
    BOOL bShowMemory;           /* Show memory in status bar */
    DWORD dwSlowOpCount;        /* Count of slow operations */
    HANDLE hLogFile;            /* Debug log file handle */
    CRITICAL_SECTION cs;        /* Thread safety */
    BOOL bInitialized;          /* Initialization flag */
} PerfMonitor;

/* Performance monitor functions */
void PerfMon_Initialize(PerfMonitor* pMon);
void PerfMon_Shutdown(PerfMonitor* pMon);
void PerfMon_BeginFrame(PerfMonitor* pMon);
void PerfMon_EndFrame(PerfMonitor* pMon);
void PerfMon_LogSlowOp(PerfMonitor* pMon, const char* szOpName, DWORD dwMs);
void PerfMon_UpdateMemory(PerfMonitor* pMon);
void PerfMon_GetStatusText(PerfMonitor* pMon, TCHAR* szBuffer, DWORD dwSize);
float PerfMon_GetFPS(PerfMonitor* pMon);
DWORD PerfMon_GetMemoryUsage(PerfMonitor* pMon);
void PerfMon_SetDebugMode(PerfMonitor* pMon, BOOL bDebug);
void PerfMon_Log(PerfMonitor* pMon, const char* szFormat, ...);

/* Timing helper macros */
#define PERF_BEGIN_OP(name) DWORD _perf_start_##name = GetTickCount()
#define PERF_END_OP(pMon, name) do { \
    DWORD _perf_elapsed = GetTickCount() - _perf_start_##name; \
    if (_perf_elapsed > PERF_SLOW_OP_MS) PerfMon_LogSlowOp(pMon, #name, _perf_elapsed); \
} while(0)

/*============================================================================
 * Global Performance State
 *============================================================================*/

/* Extended application state for performance features */
typedef struct {
    /* Performance components */
    IOCPManager iocp;           /* Async I/O manager */
    MemoryPool memPool;         /* Memory pool */
    AutoSaveManager autoSave;   /* Auto-save manager */
    PerfMonitor perfMon;        /* Performance monitor */
    SmoothScrollManager scroll; /* Smooth scroll manager */
    
    /* Rendering */
    DWriteRenderer* pDWrite;    /* DirectWrite renderer (NULL if unavailable) */
    BOOL bUseHardwareAccel;     /* Hardware acceleration enabled */
    
    /* Settings */
    BOOL bAutoSaveEnabled;      /* Auto-save enabled */
    BOOL bSmoothScrollEnabled;  /* Smooth scroll enabled */
    BOOL bDebugMode;            /* Debug mode enabled */
    
    /* Initialization */
    BOOL bInitialized;          /* Full initialization flag */
} PerfState;

/* Global performance state */
extern PerfState g_PerfState;

/*============================================================================
 * High-Level Performance API
 *============================================================================*/

/* Initialize all performance components */
BOOL Perf_Initialize(HWND hwndMain);

/* Shutdown all performance components */
void Perf_Shutdown(void);

/* Check if performance features are available */
BOOL Perf_IsAvailable(void);

/* Get global performance state */
PerfState* Perf_GetState(void);

/* Async file operations */
BOOL Perf_LoadFileAsync(const TCHAR* szFileName, HWND hwndNotify, void* pUserData);
BOOL Perf_SaveFileAsync(const TCHAR* szFileName, const WCHAR* pContent, DWORD dwLen, HWND hwndNotify, void* pUserData);

/* Memory allocation through pool */
LPVOID Perf_Alloc(DWORD dwSize);
void Perf_Free(LPVOID pMem);

/* Search operations */
SearchMatch* Perf_Search(const WCHAR* pText, DWORD dwTextLen, const WCHAR* pPattern, BOOL bCaseSensitive, DWORD* pdwCount);

/* Fallback handlers */
void Perf_HandleDWriteError(HRESULT hr);
void Perf_HandleIOCPError(DWORD dwError);
void Perf_HandleMemPoolError(void);
void Perf_HandleAutoSaveError(DWORD dwError);

#ifdef __cplusplus
}
#endif

#endif /* PERFORMANCE_H */
