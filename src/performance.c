/**
 * performance.c - Advanced Performance Optimization for XNote
 */

#include "performance.h"
#include "notepad.h"
#include <stdio.h>
#include <stdarg.h>
#include <math.h>

PerfState g_PerfState = {0};

BOOL MemPool_Initialize(MemoryPool* pPool) {
    if (!pPool) return FALSE;
    ZeroMemory(pPool, sizeof(MemoryPool));
    InitializeCriticalSection(&pPool->cs);
    pPool->pPoolBase = VirtualAlloc(NULL, MEMPOOL_INITIAL_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!pPool->pPoolBase) { DeleteCriticalSection(&pPool->cs); return FALSE; }
    pPool->dwPoolSize = MEMPOOL_INITIAL_SIZE;
    pPool->dwMaxSize = MEMPOOL_MAX_SIZE;
    pPool->bInitialized = TRUE;
    return TRUE;
}

void MemPool_Shutdown(MemoryPool* pPool) {
    if (!pPool || !pPool->bInitialized) return;
    EnterCriticalSection(&pPool->cs);
    if (pPool->pPoolBase) { VirtualFree(pPool->pPoolBase, 0, MEM_RELEASE); pPool->pPoolBase = NULL; }
    pPool->bInitialized = FALSE;
    LeaveCriticalSection(&pPool->cs);
    DeleteCriticalSection(&pPool->cs);
}

LPVOID MemPool_Alloc(MemoryPool* pPool, DWORD dwSize) {
    if (!pPool || !pPool->bInitialized || dwSize == 0) return NULL;
    return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);
}

void MemPool_Free(MemoryPool* pPool, LPVOID pMem) {
    (void)pPool;
    if (pMem) HeapFree(GetProcessHeap(), 0, pMem);
}

DWORD MemPool_GetUsage(MemoryPool* pPool) { return pPool && pPool->bInitialized ? pPool->dwUsedSize : 0; }
BOOL MemPool_IsNearLimit(MemoryPool* pPool) { return pPool && pPool->bInitialized && pPool->dwUsedSize >= MEMPOOL_WARN_THRESHOLD; }
void MemPool_GetStats(MemoryPool* pPool, DWORD* u, DWORD* t, DWORD* c) {
    if (!pPool || !pPool->bInitialized) {
        if (u) *u = 0;
        if (t) *t = 0;
        if (c) *c = 0;
        return;
    }

    if (u) *u = pPool->dwUsedSize;
    if (t) *t = pPool->dwPoolSize;
    if (c) *c = pPool->dwAllocCount;
}

BOOL IOCP_Initialize(IOCPManager* pMgr) {
    int i;
    if (!pMgr) return FALSE;
    ZeroMemory(pMgr, sizeof(IOCPManager));
    InitializeCriticalSection(&pMgr->cs);
    pMgr->hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, IOCP_THREAD_COUNT);
    if (!pMgr->hIOCP) { DeleteCriticalSection(&pMgr->cs); return FALSE; }
    for (i = 0; i < IOCP_THREAD_COUNT; i++) {
        pMgr->hThreadPool[i] = CreateThread(NULL, 0, IOCP_WorkerThread, pMgr, 0, NULL);
        if (!pMgr->hThreadPool[i]) { pMgr->bShutdown = TRUE; CloseHandle(pMgr->hIOCP); DeleteCriticalSection(&pMgr->cs); return FALSE; }
    }
    pMgr->bInitialized = TRUE;
    return TRUE;
}

void IOCP_Shutdown(IOCPManager* pMgr) {
    int i;
    if (!pMgr || !pMgr->bInitialized) return;
    pMgr->bShutdown = TRUE;
    for (i = 0; i < IOCP_THREAD_COUNT; i++) PostQueuedCompletionStatus(pMgr->hIOCP, 0, 0, NULL);
    for (i = 0; i < IOCP_THREAD_COUNT; i++) { if (pMgr->hThreadPool[i]) { WaitForSingleObject(pMgr->hThreadPool[i], 5000); CloseHandle(pMgr->hThreadPool[i]); } }
    if (pMgr->hIOCP) CloseHandle(pMgr->hIOCP);
    pMgr->bInitialized = FALSE;
    DeleteCriticalSection(&pMgr->cs);
}

DWORD WINAPI IOCP_WorkerThread(LPVOID lpParam) {
    IOCPManager* pMgr = (IOCPManager*)lpParam;
    DWORD dwBytes; ULONG_PTR ulKey; LPOVERLAPPED pOv; AsyncFileOp* pOp;
    while (!pMgr->bShutdown) {
        BOOL bResult = GetQueuedCompletionStatus(pMgr->hIOCP, &dwBytes, &ulKey, &pOv, INFINITE);
        if (pMgr->bShutdown || !pOv) break;
        pOp = (AsyncFileOp*)pOv;
        pOp->dwBytesTransferred = dwBytes;
        pOp->bSuccess = bResult;
        if (!bResult) pOp->dwError = GetLastError();
        if (pOp->hwndNotify && pOp->uMsgComplete) PostMessage(pOp->hwndNotify, pOp->uMsgComplete, (WPARAM)pOp->bSuccess, (LPARAM)pOp);
    }
    return 0;
}

BOOL IOCP_QueueRead(IOCPManager* pMgr, AsyncFileOp* pOp) { (void)pMgr; (void)pOp; return FALSE; }
BOOL IOCP_QueueWrite(IOCPManager* pMgr, AsyncFileOp* pOp) { (void)pMgr; (void)pOp; return FALSE; }

AsyncFileOp* IOCP_CreateOp(const TCHAR* szFileName, HWND hwndNotify, UINT uMsg, void* pUserData) {
    AsyncFileOp* pOp = (AsyncFileOp*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(AsyncFileOp));
    if (!pOp) return NULL;
    if (szFileName) _tcsncpy(pOp->szFileName, szFileName, MAX_PATH - 1);
    pOp->hwndNotify = hwndNotify;
    pOp->uMsgComplete = uMsg;
    pOp->pUserData = pUserData;
    return pOp;
}

void IOCP_FreeOp(AsyncFileOp* pOp) {
    if (!pOp) return;
    if (pOp->hFile && pOp->hFile != INVALID_HANDLE_VALUE) CloseHandle(pOp->hFile);
    if (pOp->pBuffer) VirtualFree(pOp->pBuffer, 0, MEM_RELEASE);
    HeapFree(GetProcessHeap(), 0, pOp);
}

BOOL DWrite_IsAvailable(void) { return FALSE; }
BOOL DWrite_Initialize(DWriteRenderer* p, HWND h) { (void)p; (void)h; return FALSE; }
void DWrite_Shutdown(DWriteRenderer* p) { if (p) p->bInitialized = FALSE; }
BOOL DWrite_SetFont(DWriteRenderer* p, const WCHAR* n, float s) { (void)p; (void)n; (void)s; return FALSE; }
void DWrite_RenderText(DWriteRenderer* p, const WCHAR* t, DWORD l, RECT* r) { (void)p; (void)t; (void)l; (void)r; }
void DWrite_RenderLine(DWriteRenderer* p, const WCHAR* t, DWORD l, int x, int y) { (void)p; (void)t; (void)l; (void)x; (void)y; }
void DWrite_Resize(DWriteRenderer* p, int w, int h) { (void)p; (void)w; (void)h; }
void DWrite_SetTextColor(DWriteRenderer* p, COLORREF c) { (void)p; (void)c; }
void DWrite_SetBgColor(DWriteRenderer* p, COLORREF c) { (void)p; (void)c; }

void BoyerMoore_Init(BoyerMooreState* p, const WCHAR* pat, BOOL cs) { (void)p; (void)pat; (void)cs; }
void BoyerMoore_Shutdown(BoyerMooreState* p) { (void)p; }
int BoyerMoore_Search(BoyerMooreState* p, const WCHAR* t, DWORD l, DWORD s) { (void)p; (void)t; (void)l; (void)s; return -1; }

BOOL SearchIndex_Initialize(SearchIndex* p) { if (!p) return FALSE; ZeroMemory(p, sizeof(SearchIndex)); InitializeCriticalSection(&p->cs); p->bInitialized = TRUE; return TRUE; }
void SearchIndex_Shutdown(SearchIndex* p) { if (!p || !p->bInitialized) return; p->bInitialized = FALSE; DeleteCriticalSection(&p->cs); }
BOOL SearchIndex_Build(SearchIndex* p, const WCHAR* t, DWORD l, HWND h) { (void)p; (void)t; (void)l; (void)h; return TRUE; }
BOOL SearchIndex_BuildAsync(SearchIndex* p, const WCHAR* t, DWORD l, HWND h) { (void)p; (void)t; (void)l; (void)h; return TRUE; }
BOOL SearchIndex_Update(SearchIndex* p, DWORD s, DWORD o, const WCHAR* t, DWORD n) { (void)p; (void)s; (void)o; (void)t; (void)n; return TRUE; }
SearchMatch* SearchIndex_Find(SearchIndex* p, const WCHAR* pat, BOOL cs, DWORD* c) { (void)p; (void)pat; (void)cs; if (c) *c = 0; return NULL; }
void SearchIndex_FreeResults(SearchMatch* p) { if (p) HeapFree(GetProcessHeap(), 0, p); }
DWORD SearchIndex_GetLineFromPos(SearchIndex* p, DWORD pos) { (void)p; (void)pos; return 1; }

BOOL Highlighter_Initialize(IncrementalHighlighter* p, HWND h) { if (!p) return FALSE; ZeroMemory(p, sizeof(IncrementalHighlighter)); p->hwndEdit = h; InitializeCriticalSection(&p->cs); p->bInitialized = TRUE; return TRUE; }
void Highlighter_Shutdown(IncrementalHighlighter* p) { if (!p || !p->bInitialized) return; p->bInitialized = FALSE; DeleteCriticalSection(&p->cs); }
void Highlighter_MarkDirty(IncrementalHighlighter* p, DWORD l) { (void)p; (void)l; }
void Highlighter_MarkRangeDirty(IncrementalHighlighter* p, DWORD s, DWORD e) { (void)p; (void)s; (void)e; }
BOOL Highlighter_ProcessVisible(IncrementalHighlighter* p, DWORD f, DWORD l) { (void)p; (void)f; (void)l; return TRUE; }
void Highlighter_TriggerUpdate(IncrementalHighlighter* p) { (void)p; }
DWORD Highlighter_GetLastProcessTime(IncrementalHighlighter* p) { (void)p; return 0; }
DWORD WINAPI Highlighter_WorkerThread(LPVOID p) { (void)p; return 0; }

float Easing_Linear(float t) { return t; }
float Easing_EaseOutCubic(float t) { t = t - 1.0f; return t * t * t + 1.0f; }
float Easing_EaseOutQuad(float t) { return t * (2.0f - t); }
float Easing_EaseInOutCubic(float t) { if (t < 0.5f) return 4.0f * t * t * t; float f = 2.0f * t - 2.0f; return 0.5f * f * f * f + 1.0f; }

BOOL SmoothScroll_Initialize(SmoothScrollManager* p, HWND h) { if (!p) return FALSE; ZeroMemory(p, sizeof(SmoothScrollManager)); p->hwnd = h; p->bEnabled = TRUE; p->bInitialized = TRUE; return TRUE; }
void SmoothScroll_Shutdown(SmoothScrollManager* p) { if (!p || !p->bInitialized) return; if (p->nTimerId) KillTimer(p->hwnd, p->nTimerId); p->bInitialized = FALSE; }
void SmoothScroll_ScrollTo(SmoothScrollManager* p, int y, BOOL a) { (void)p; (void)y; (void)a; }
void SmoothScroll_ScrollBy(SmoothScrollManager* p, int d, BOOL a) { (void)p; (void)d; (void)a; }
void SmoothScroll_Update(SmoothScrollManager* p) { (void)p; }
BOOL SmoothScroll_IsAnimating(SmoothScrollManager* p) { (void)p; return FALSE; }
void SmoothScroll_Stop(SmoothScrollManager* p) { (void)p; }
void SmoothScroll_SetEnabled(SmoothScrollManager* p, BOOL e) { if (p) p->bEnabled = e; }

BOOL AutoSave_Initialize(AutoSaveManager* p, HWND h) { if (!p) return FALSE; ZeroMemory(p, sizeof(AutoSaveManager)); p->hwndNotify = h; p->dwIntervalMs = AUTOSAVE_INTERVAL_MS; p->dwDelayAfterTypeMs = AUTOSAVE_DELAY_MS; p->bEnabled = TRUE; InitializeCriticalSection(&p->cs); p->bInitialized = TRUE; return TRUE; }
void AutoSave_Shutdown(AutoSaveManager* p) { if (!p || !p->bInitialized) return; p->bInitialized = FALSE; DeleteCriticalSection(&p->cs); }
void AutoSave_OnKeystroke(AutoSaveManager* p) { if (p && p->bInitialized && p->bEnabled) { EnterCriticalSection(&p->cs); p->dwLastKeystroke = GetTickCount(); p->bPendingSave = TRUE; LeaveCriticalSection(&p->cs); } }
void AutoSave_TriggerNow(AutoSaveManager* p) { (void)p; }
void AutoSave_SetEnabled(AutoSaveManager* p, BOOL e) { if (p && p->bInitialized) { EnterCriticalSection(&p->cs); p->bEnabled = e; LeaveCriticalSection(&p->cs); } }
AutoSaveStatus AutoSave_GetStatus(AutoSaveManager* p) { return p && p->bInitialized ? p->status : AUTOSAVE_IDLE; }
const TCHAR* AutoSave_GetLastError(AutoSaveManager* p) { return p && p->bInitialized ? p->szLastError : TEXT(""); }
DWORD WINAPI AutoSave_WorkerThread(LPVOID p) { (void)p; return 0; }

void PerfMon_Initialize(PerfMonitor* p) { if (!p) return; ZeroMemory(p, sizeof(PerfMonitor)); InitializeCriticalSection(&p->cs); p->dwLastFPSUpdate = GetTickCount(); p->bInitialized = TRUE; }
void PerfMon_Shutdown(PerfMonitor* p) { if (!p || !p->bInitialized) return; if (p->hLogFile && p->hLogFile != INVALID_HANDLE_VALUE) CloseHandle(p->hLogFile); p->bInitialized = FALSE; DeleteCriticalSection(&p->cs); }
void PerfMon_BeginFrame(PerfMonitor* p) { (void)p; }
void PerfMon_EndFrame(PerfMonitor* p) { if (!p || !p->bInitialized) return; EnterCriticalSection(&p->cs); p->dwFrameCount++; DWORD now = GetTickCount(); if (now - p->dwLastFPSUpdate >= 1000) { p->fCurrentFPS = (float)p->dwFrameCount * 1000.0f / (float)(now - p->dwLastFPSUpdate); p->dwFrameCount = 0; p->dwLastFPSUpdate = now; } LeaveCriticalSection(&p->cs); }
void PerfMon_LogSlowOp(PerfMonitor* p, const char* n, DWORD m) { if (p && p->bInitialized && p->bDebugMode) { char buf[256]; sprintf(buf, "[PERF] Slow: %s %lu ms\n", n, m); OutputDebugStringA(buf); } }
void PerfMon_UpdateMemory(PerfMonitor* p) { PROCESS_MEMORY_COUNTERS pmc; if (p && p->bInitialized && GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) { EnterCriticalSection(&p->cs); p->dwMemoryUsage = (DWORD)(pmc.WorkingSetSize / (1024 * 1024)); if (p->dwMemoryUsage > p->dwPeakMemory) p->dwPeakMemory = p->dwMemoryUsage; LeaveCriticalSection(&p->cs); } }
void PerfMon_GetStatusText(PerfMonitor* p, TCHAR* b, DWORD s) { if (!p || !p->bInitialized || !b || s == 0) return; EnterCriticalSection(&p->cs); if (p->bShowFPS && p->bShowMemory) _sntprintf(b, s, TEXT("FPS: %.1f | Mem: %lu MB"), p->fCurrentFPS, p->dwMemoryUsage); else if (p->bShowFPS) _sntprintf(b, s, TEXT("FPS: %.1f"), p->fCurrentFPS); else if (p->bShowMemory) _sntprintf(b, s, TEXT("Mem: %lu MB"), p->dwMemoryUsage); else b[0] = 0; LeaveCriticalSection(&p->cs); }
float PerfMon_GetFPS(PerfMonitor* p) { float f = 0; if (p && p->bInitialized) { EnterCriticalSection(&p->cs); f = p->fCurrentFPS; LeaveCriticalSection(&p->cs); } return f; }
DWORD PerfMon_GetMemoryUsage(PerfMonitor* p) { DWORD u = 0; if (p && p->bInitialized) { EnterCriticalSection(&p->cs); u = p->dwMemoryUsage; LeaveCriticalSection(&p->cs); } return u; }
void PerfMon_SetDebugMode(PerfMonitor* p, BOOL d) { if (p && p->bInitialized) { EnterCriticalSection(&p->cs); p->bDebugMode = d; p->bShowFPS = d; p->bShowMemory = d; LeaveCriticalSection(&p->cs); } }
void PerfMon_Log(PerfMonitor* p, const char* f, ...) { if (!p || !p->bInitialized || !p->bDebugMode) return; char buf[512]; va_list args; va_start(args, f); vsnprintf(buf, sizeof(buf), f, args); va_end(args); OutputDebugStringA(buf); }

BOOL Perf_Initialize(HWND hwndMain) {
    ZeroMemory(&g_PerfState, sizeof(PerfState));
    if (!MemPool_Initialize(&g_PerfState.memPool)) OutputDebugString(TEXT("[PERF] MemPool init failed\n"));
    if (!IOCP_Initialize(&g_PerfState.iocp)) OutputDebugString(TEXT("[PERF] IOCP init failed\n"));
    if (!AutoSave_Initialize(&g_PerfState.autoSave, hwndMain)) OutputDebugString(TEXT("[PERF] AutoSave init failed\n"));
    PerfMon_Initialize(&g_PerfState.perfMon);
    if (!SmoothScroll_Initialize(&g_PerfState.scroll, hwndMain)) OutputDebugString(TEXT("[PERF] SmoothScroll init failed\n"));
    g_PerfState.bAutoSaveEnabled = TRUE;
    g_PerfState.bSmoothScrollEnabled = TRUE;
    g_PerfState.bInitialized = TRUE;
    return TRUE;
}

void Perf_Shutdown(void) {
    if (!g_PerfState.bInitialized) return;
    if (g_PerfState.pDWrite) { DWrite_Shutdown(g_PerfState.pDWrite); HeapFree(GetProcessHeap(), 0, g_PerfState.pDWrite); g_PerfState.pDWrite = NULL; }
    SmoothScroll_Shutdown(&g_PerfState.scroll);
    PerfMon_Shutdown(&g_PerfState.perfMon);
    AutoSave_Shutdown(&g_PerfState.autoSave);
    IOCP_Shutdown(&g_PerfState.iocp);
    MemPool_Shutdown(&g_PerfState.memPool);
    g_PerfState.bInitialized = FALSE;
}

BOOL Perf_IsAvailable(void) { return g_PerfState.bInitialized; }
PerfState* Perf_GetState(void) { return &g_PerfState; }

BOOL Perf_LoadFileAsync(const TCHAR* f, HWND h, void* u) { (void)f; (void)h; (void)u; return FALSE; }
BOOL Perf_SaveFileAsync(const TCHAR* f, const WCHAR* c, DWORD l, HWND h, void* u) { (void)f; (void)c; (void)l; (void)h; (void)u; return FALSE; }

LPVOID Perf_Alloc(DWORD s) { return g_PerfState.bInitialized && g_PerfState.memPool.bInitialized ? MemPool_Alloc(&g_PerfState.memPool, s) : HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, s); }
void Perf_Free(LPVOID p) { if (!p) return; if (g_PerfState.bInitialized && g_PerfState.memPool.bInitialized) MemPool_Free(&g_PerfState.memPool, p); else HeapFree(GetProcessHeap(), 0, p); }

SearchMatch* Perf_Search(const WCHAR* t, DWORD tl, const WCHAR* p, BOOL cs, DWORD* c) { (void)t; (void)tl; (void)p; (void)cs; if (c) *c = 0; return NULL; }

void Perf_HandleDWriteError(HRESULT hr) { TCHAR m[256]; _sntprintf(m, 256, TEXT("[PERF] DWrite error: 0x%08X\n"), hr); OutputDebugString(m); g_PerfState.bUseHardwareAccel = FALSE; }
void Perf_HandleIOCPError(DWORD e) { TCHAR m[256]; _sntprintf(m, 256, TEXT("[PERF] IOCP error: %lu\n"), e); OutputDebugString(m); }
void Perf_HandleMemPoolError(void) { OutputDebugString(TEXT("[PERF] MemPool exhausted\n")); }
void Perf_HandleAutoSaveError(DWORD e) { TCHAR m[256]; _sntprintf(m, 256, TEXT("[PERF] AutoSave error: %lu\n"), e); OutputDebugString(m); }
