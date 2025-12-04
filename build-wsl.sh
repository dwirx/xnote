#!/bin/bash
# =============================================================================
# XNote WSL Build Script
# Build Windows executable from WSL (Windows Subsystem for Linux)
# =============================================================================

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Compiler settings for cross-compilation
CC="x86_64-w64-mingw32-gcc"
RC="x86_64-w64-mingw32-windres"

# Directories
SRC_DIR="src"
BUILD_DIR="build"

# Output
TARGET="xnote.exe"

# Compiler flags
CFLAGS="-Wall -Wextra -O3 -DUNICODE -D_UNICODE"
LDFLAGS="-mwindows -lcomctl32 -lcomdlg32 -lshell32 -lshlwapi -ld2d1 -ldwrite -lpsapi -lole32 -s"

# Source files
SRCS=(
    "main.c"
    "file_ops.c"
    "edit_ops.c"
    "dialogs.c"
    "line_numbers.c"
    "statusbar.c"
    "syntax.c"
    "vim_mode.c"
    "session.c"
    "theme.c"
    "json_format.c"
    "settings.c"
    "multi_cursor.c"
    "dragdrop.c"
    "performance.c"
    "sticky_notes.c"
)

# Resource file
RES_SRC="notepad.rc"

# Functions
print_header() {
    echo -e "${BLUE}=============================================${NC}"
    echo -e "${BLUE}  XNote WSL Build Script${NC}"
    echo -e "${BLUE}=============================================${NC}"
}

print_step() {
    echo -e "${YELLOW}>> $1${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

check_mingw() {
    print_step "Checking MinGW-w64 installation..."
    
    if ! command -v $CC &> /dev/null; then
        print_error "MinGW-w64 not found!"
        echo ""
        echo -e "${YELLOW}To install MinGW-w64 on Ubuntu/Debian WSL:${NC}"
        echo "  sudo apt update"
        echo "  sudo apt install mingw-w64"
        echo ""
        echo -e "${YELLOW}To install MinGW-w64 on Arch Linux:${NC}"
        echo "  sudo pacman -S mingw-w64-gcc"
        echo ""
        exit 1
    fi
    
    if ! command -v $RC &> /dev/null; then
        print_error "MinGW-w64 windres not found!"
        echo "Please install full mingw-w64 package"
        exit 1
    fi
    
    print_success "MinGW-w64 found: $($CC --version | head -1)"
}

clean_build() {
    print_step "Cleaning previous build..."
    rm -rf "$BUILD_DIR"
    rm -f "$TARGET"
    rm -f ${SRC_DIR}/*.o
    print_success "Clean complete"
}

compile_sources() {
    print_step "Compiling source files..."
    
    mkdir -p "$BUILD_DIR"
    
    local obj_files=()
    local total=${#SRCS[@]}
    local count=1
    
    for src in "${SRCS[@]}"; do
        local src_path="${SRC_DIR}/${src}"
        local obj_path="${BUILD_DIR}/${src%.c}.o"
        
        echo -e "  [${count}/${total}] Compiling ${src}..."
        
        # Run compiler - ignore warnings but catch errors
        if ! $CC $CFLAGS -c "$src_path" -o "$obj_path" 2>/dev/null; then
            # If failed, run again to show error
            print_error "Failed to compile $src"
            $CC $CFLAGS -c "$src_path" -o "$obj_path" 2>&1 || true
            exit 1
        fi
        
        obj_files+=("$obj_path")
        count=$((count + 1))
    done
    
    print_success "All source files compiled"
    
    # Store object files for linking
    OBJ_FILES="${obj_files[*]}"
}

compile_resources() {
    print_step "Compiling resources..."
    
    local res_path="${SRC_DIR}/${RES_SRC}"
    local res_obj="${BUILD_DIR}/notepad.res.o"
    
    if ! $RC "$res_path" -o "$res_obj"; then
        print_error "Failed to compile resources"
        exit 1
    fi
    
    print_success "Resources compiled"
    
    RES_OBJ="$res_obj"
}

link_executable() {
    print_step "Linking executable..."
    
    if ! $CC $OBJ_FILES $RES_OBJ -o "$TARGET" $LDFLAGS; then
        print_error "Failed to link executable"
        exit 1
    fi
    
    print_success "Executable created: $TARGET"
}

show_result() {
    echo ""
    echo -e "${GREEN}=============================================${NC}"
    echo -e "${GREEN}  Build Successful!${NC}"
    echo -e "${GREEN}=============================================${NC}"
    echo ""
    echo -e "Output: ${BLUE}$(pwd)/$TARGET${NC}"
    echo -e "Size:   ${BLUE}$(du -h "$TARGET" | cut -f1)${NC}"
    echo ""
    echo -e "${YELLOW}To run on Windows:${NC}"
    echo "  - Copy $TARGET to Windows"
    echo "  - Or run from WSL: cmd.exe /c $TARGET"
    echo "  - Or use: explorer.exe ."
    echo ""
}

install_mingw() {
    print_step "Installing MinGW-w64..."
    
    # Detect package manager
    if command -v apt &> /dev/null; then
        echo "Detected apt package manager (Ubuntu/Debian)"
        sudo apt update
        sudo apt install -y mingw-w64
    elif command -v pacman &> /dev/null; then
        echo "Detected pacman package manager (Arch)"
        sudo pacman -S --noconfirm mingw-w64-gcc
    elif command -v dnf &> /dev/null; then
        echo "Detected dnf package manager (Fedora)"
        sudo dnf install -y mingw64-gcc mingw64-winpthreads-static
    else
        print_error "Unknown package manager. Please install mingw-w64 manually."
        exit 1
    fi
    
    print_success "MinGW-w64 installed"
}

# Main script
main() {
    print_header
    
    case "${1:-build}" in
        build)
            check_mingw
            compile_sources
            compile_resources
            link_executable
            show_result
            ;;
        clean)
            clean_build
            ;;
        rebuild)
            clean_build
            check_mingw
            compile_sources
            compile_resources
            link_executable
            show_result
            ;;
        install-deps)
            install_mingw
            ;;
        run)
            if [[ -f "$TARGET" ]]; then
                print_step "Running $TARGET..."
                cmd.exe /c "$TARGET" 2>/dev/null || explorer.exe "$TARGET"
            else
                print_error "$TARGET not found. Build first with: ./build-wsl.sh build"
                exit 1
            fi
            ;;
        help|--help|-h)
            echo "Usage: $0 [command]"
            echo ""
            echo "Commands:"
            echo "  build        Build the project (default)"
            echo "  clean        Remove build artifacts"
            echo "  rebuild      Clean and rebuild"
            echo "  install-deps Install MinGW-w64"
            echo "  run          Run the built executable"
            echo "  help         Show this help message"
            echo ""
            ;;
        *)
            print_error "Unknown command: $1"
            echo "Use '$0 help' for usage information"
            exit 1
            ;;
    esac
}

# Run main with all arguments
main "$@"
