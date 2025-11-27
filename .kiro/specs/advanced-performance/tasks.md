# Implementation Plan

## Advanced Performance Optimization untuk XNote

- [x] 1. Setup infrastruktur dan header files




  - [x] 1.1 Buat file `src/performance.h` dengan semua struktur data dan deklarasi fungsi

    - Definisikan IOCPManager, DWriteRenderer, MemoryPool, SearchIndex, dll

    - Definisikan konstanta performa (MEMPOOL_MAX_SIZE, SCROLL_FPS_TARGET, dll)
    - _Requirements: 1.1, 2.1, 4.1, 5.1, 6.1_


  - [x] 1.2 Buat file `src/performance.c` dengan stub functions

    - Implementasi skeleton untuk semua fungsi
    - _Requirements: 1.1, 2.1, 4.1, 5.1, 6.1_



  - [ ] 1.3 Update Makefile untuk compile performance module
    - Tambahkan link ke d2d1.lib, dwrite.lib
    - _Requirements: 2.1, 6.1_


- [ ] 2. Implementasi Memory Pool Allocator
  - [x] 2.1 Implementasi `MemPool_Initialize()` dan `MemPool_Shutdown()`

    - Pre-allocate 64MB initial pool
    - Setup free list dan critical section
    - _Requirements: 4.1_
  - [ ] 2.2 Implementasi `MemPool_Alloc()` dan `MemPool_Free()`
    - Alokasi dengan 16-byte alignment
    - Return memory ke free list saat free



    - _Requirements: 4.1, 4.2_
  - [ ] 2.3 Implementasi `MemPool_GetUsage()` dan `MemPool_IsNearLimit()`
    - Track memory usage

    - Warning saat mendekati 500MB limit
    - _Requirements: 4.3, 4.4_
  - [x]* 2.4 Write property test untuk Memory Pool Bounds

    - **Property 4: Memory Pool Bounds**
    - **Validates: Requirements 4.1, 4.2, 4.3**



- [ ] 3. Implementasi Asynchronous I/O dengan IOCP
  - [ ] 3.1 Implementasi `IOCP_Initialize()` dan `IOCP_Shutdown()`
    - Buat I/O Completion Port
    - Buat thread pool (4 worker threads)
    - _Requirements: 1.1, 1.3_
  - [ ] 3.2 Implementasi `IOCP_WorkerThread()`
    - Loop GetQueuedCompletionStatus
    - Post completion message ke UI thread
    - _Requirements: 1.2, 1.4_

  - [ ] 3.3 Implementasi `IOCP_QueueRead()` dan `IOCP_QueueWrite()`
    - Setup OVERLAPPED structure
    - Queue async operation
    - _Requirements: 1.1_

  - [ ] 3.4 Integrasi async loading ke file_ops.c
    - Modifikasi `OpenFile()` untuk menggunakan async I/O
    - Handle completion message di main window proc

    - _Requirements: 1.1, 1.2, 1.4_
  - [ ]* 3.5 Write property test untuk Async I/O Responsiveness
    - **Property 1: Async I/O Responsiveness**
    - **Validates: Requirements 1.1, 1.2, 1.4**

- [ ] 4. Checkpoint - Pastikan semua tests passing
  - Ensure all tests pass, ask the user if questions arise.

- [ ] 5. Implementasi Boyer-Moore Search dan Background Indexing
  - [ ] 5.1 Implementasi `BoyerMoore_Init()` dan `BoyerMoore_Search()`
    - Build bad character table

    - Build good suffix table
    - Implementasi search algorithm
    - _Requirements: 5.4_

  - [ ] 5.2 Implementasi `SearchIndex_Initialize()` dan `SearchIndex_Build()`
    - Build line offset table

    - Run di background thread
    - _Requirements: 5.1_
  - [ ] 5.3 Implementasi `SearchIndex_Find()` dan `SearchIndex_Update()`
    - Gunakan Boyer-Moore untuk search
    - Incremental update saat file dimodifikasi
    - _Requirements: 5.2, 5.3_
  - [ ] 5.4 Integrasi search index ke Find dialog
    - Gunakan indexed search untuk file yang sudah di-index
    - _Requirements: 5.2_
  - [ ]* 5.5 Write property test untuk Search Index Performance
    - **Property 5: Search Index Performance**
    - **Validates: Requirements 5.1, 5.2, 5.3**

- [x] 6. Implementasi Incremental Syntax Highlighting

  - [x] 6.1 Implementasi `Highlighter_Initialize()` dan `Highlighter_Shutdown()`

    - Setup worker thread dan event
    - Initialize dirty line tracking
    - _Requirements: 3.1_
  - [x] 6.2 Implementasi `Highlighter_MarkDirty()` dan `Highlighter_MarkRangeDirty()`

    - Track baris yang perlu di-highlight ulang
    - _Requirements: 3.1_
  - [x] 6.3 Implementasi `Highlighter_ProcessVisible()` dan worker thread

    - Hanya proses visible area
    - Background processing untuk area lain
    - _Requirements: 3.3, 3.4_
  - [x] 6.4 Integrasi incremental highlighter ke syntax.c

    - Hook ke text change notifications
    - Prioritaskan visible area

    - _Requirements: 3.1, 3.2_
  - [ ]* 6.5 Write property test untuk Incremental Syntax Highlighting Performance
    - **Property 3: Incremental Syntax Highlighting Performance**

    - **Validates: Requirements 3.1, 3.2, 3.3, 3.4**

- [ ] 7. Checkpoint - Pastikan semua tests passing
  - Ensure all tests pass, ask the user if questions arise.

- [ ] 8. Implementasi DirectWrite Text Renderer (Optional)
  - [ ] 8.1 Implementasi `DWrite_Initialize()` dan `DWrite_Shutdown()`
    - Buat D2D1Factory dan DWriteFactory
    - Buat render target
    - Handle fallback ke GDI jika gagal
    - _Requirements: 2.1, 2.4_
  - [ ] 8.2 Implementasi `DWrite_SetFont()` dan `DWrite_RenderText()`
    - Buat text format
    - Render text dengan hardware acceleration

    - _Requirements: 2.2_
  - [ ] 8.3 Implementasi `DWrite_IsAvailable()` dan fallback logic
    - Check DirectWrite availability

    - Graceful fallback ke GDI
    - _Requirements: 2.4_


- [ ] 9. Implementasi Smooth Scroll Manager
  - [ ] 9.1 Implementasi `SmoothScroll_Initialize()` dan `SmoothScroll_Shutdown()`
    - Setup animation state
    - _Requirements: 6.1_
  - [ ] 9.2 Implementasi `SmoothScroll_ScrollTo()` dan `SmoothScroll_Update()`
    - Implementasi scroll animation
    - Implementasi ease-out-cubic easing
    - _Requirements: 6.2_
  - [ ] 9.3 Implementasi scroll prediction dan pre-rendering
    - Pre-render 10 lines ahead
    - _Requirements: 6.3_
  - [ ] 9.4 Integrasi smooth scroll ke main window
    - Hook ke WM_MOUSEWHEEL
    - Timer-based animation update
    - _Requirements: 6.2, 6.3, 6.4_

  - [ ]* 9.5 Write property test untuk Smooth Scroll Easing
    - **Property 6: Smooth Scroll Easing**
    - **Validates: Requirements 6.2, 6.3**

  - [ ]* 9.6 Write property test untuk Scroll Frame Rate
    - **Property 2: Scroll Frame Rate**
    - **Validates: Requirements 2.3**



- [ ] 10. Implementasi Auto-Save Manager
  - [ ] 10.1 Implementasi `AutoSave_Initialize()` dan `AutoSave_Shutdown()`
    - Buat background thread
    - Setup events untuk wake dan stop
    - _Requirements: 9.1_
  - [ ] 10.2 Implementasi `AutoSave_WorkerThread()`
    - Wait dengan timeout 60 detik
    - Save file di background
    - _Requirements: 9.1, 9.2_
  - [ ] 10.3 Implementasi `AutoSave_OnKeystroke()` dan delay logic
    - Reset timer saat keystroke
    - Delay 3 detik setelah typing berhenti
    - _Requirements: 9.4_
  - [ ] 10.4 Integrasi auto-save ke main window dan status bar
    - Hook ke WM_CHAR
    - Show notification di status bar saat gagal
    - _Requirements: 9.2, 9.3_
  - [ ]* 10.5 Write property test untuk Background Auto-Save
    - **Property 9: Background Auto-Save**
    - **Validates: Requirements 9.1, 9.2, 9.4**

- [ ] 11. Checkpoint - Pastikan semua tests passing
  - Ensure all tests pass, ask the user if questions arise.

- [ ] 12. Implementasi Performance Monitor
  - [ ] 12.1 Implementasi `PerfMon_Initialize()` dan frame tracking
    - Setup FPS counter
    - Track memory usage
    - _Requirements: 10.1_
  - [ ] 12.2 Implementasi `PerfMon_LogSlowOp()` dan `PerfMon_UpdateMemory()`
    - Log operasi > 100ms ke debug log
    - Log memory warnings
    - _Requirements: 10.2, 10.3_
  - [ ] 12.3 Implementasi `PerfMon_GetStatusText()` dan status bar integration
    - Format FPS dan memory untuk status bar
    - _Requirements: 10.1, 10.4_
  - [ ]* 12.4 Write property test untuk Performance Logging
    - **Property 10: Performance Logging**
    - **Validates: Requirements 10.2, 10.3**

- [ ] 13. Implementasi Fast Startup dan Lazy Initialization
  - [ ] 13.1 Implementasi lazy initialization untuk komponen non-essential
    - Delay DirectWrite init sampai pertama kali dibutuhkan
    - Delay search index build
    - _Requirements: 7.2_
  - [ ] 13.2 Implementasi background session restore
    - Load file terakhir di background setelah UI siap
    - _Requirements: 7.3_
  - [ ] 13.3 Konfigurasi delay-loaded DLLs di Makefile
    - d2d1.dll dan dwrite.dll sebagai delay-loaded
    - _Requirements: 7.4_
  - [ ]* 13.4 Write property test untuk Fast Startup
    - **Property 7: Fast Startup**
    - **Validates: Requirements 7.1, 7.2, 7.3**

- [ ] 14. Optimasi Undo/Redo
  - [ ] 14.1 Implementasi incremental undo data storage
    - Simpan delta bukan full state
    - _Requirements: 8.2_
  - [ ] 14.2 Implementasi undo buffer memory management
    - Track memory usage per tab
    - Auto-remove oldest saat limit tercapai
    - _Requirements: 8.3_
  - [ ] 14.3 Integrasi dengan partial loading mode
    - Batasi undo ke loaded content
    - _Requirements: 8.4_
  - [ ]* 14.4 Write property test untuk Undo Performance
    - **Property 8: Undo Performance**
    - **Validates: Requirements 8.1, 8.2, 8.3, 8.4**

- [ ] 15. Final Checkpoint - Pastikan semua tests passing
  - Ensure all tests pass, ask the user if questions arise.

- [ ] 16. Integrasi dan Testing Akhir
  - [ ] 16.1 Integrasi semua komponen ke AppStateEx
    - Initialize semua manager di WM_CREATE
    - Shutdown di WM_DESTROY
    - _Requirements: All_
  - [ ] 16.2 Update settings untuk enable/disable fitur performa
    - Tambah settings untuk auto-save, smooth scroll, debug mode
    - _Requirements: 9.1, 6.1, 10.1_
  - [ ] 16.3 Update status bar untuk menampilkan info performa
    - FPS counter (debug mode)
    - Memory usage
    - Auto-save status
    - _Requirements: 10.1, 10.4, 9.3_
  - [ ]* 16.4 Write unit tests untuk error handling dan fallback
    - Test DirectWrite fallback ke GDI
    - Test IOCP fallback ke sync I/O
    - Test memory pool fallback ke heap
    - _Requirements: 2.4, 6.4_
