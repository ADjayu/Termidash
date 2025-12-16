#!/bin/bash
# =============================================================================
# Termidash Build Script for macOS and Linux
# =============================================================================
# Usage:
#   ./build.sh [command]
#
# Commands:
#   clean      - Remove build directory
#   configure  - Run CMake configuration
#   build      - Build the project (implies configure)
#   test       - Run tests (implies build)
#   package    - Create distribution package (implies build)
#   all        - Run clean, build, test, and package
#   help       - Show this help message
#
# Examples:
#   ./build.sh build      # Configure and build
#   ./build.sh test       # Build and run tests
#   ./build.sh package    # Build and create package
#   ./build.sh all        # Full pipeline
# =============================================================================

set -e  # Exit on error

# Configuration
BUILD_DIR="build"
BUILD_TYPE="Release"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Helper functions
print_header() {
    echo -e "${BLUE}=============================================${NC}"
    echo -e "${BLUE}  $1${NC}"
    echo -e "${BLUE}=============================================${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

print_info() {
    echo -e "${YELLOW}→ $1${NC}"
}

# Detect OS
detect_os() {
    if [[ "$OSTYPE" == "darwin"* ]]; then
        echo "macos"
    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        echo "linux"
    else
        echo "unknown"
    fi
}

# Clean build directory
do_clean() {
    print_header "Cleaning Build Directory"
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
        print_success "Removed $BUILD_DIR directory"
    else
        print_info "Build directory does not exist, nothing to clean"
    fi
}

# Configure with CMake
do_configure() {
    print_header "Configuring CMake"
    
    mkdir -p "$BUILD_DIR"
    
    print_info "Build type: $BUILD_TYPE"
    print_info "Build directory: $BUILD_DIR"
    
    cmake -B "$BUILD_DIR" \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DBUILD_TESTING=ON
    
    print_success "CMake configuration complete"
}

# Build the project
do_build() {
    print_header "Building Project"
    
    # Configure if needed
    if [ ! -f "$BUILD_DIR/CMakeCache.txt" ]; then
        print_info "Build not configured, running configure first..."
        do_configure
    fi
    
    # Determine number of parallel jobs
    if [[ "$(detect_os)" == "macos" ]]; then
        JOBS=$(sysctl -n hw.ncpu)
    else
        JOBS=$(nproc)
    fi
    
    print_info "Building with $JOBS parallel jobs"
    
    cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" -j "$JOBS"
    
    print_success "Build complete"
}

# Run tests
do_test() {
    print_header "Running Tests"
    
    # Build if needed
    if [ ! -f "$BUILD_DIR/termidash_tests" ] && [ ! -f "$BUILD_DIR/Debug/termidash_tests" ]; then
        print_info "Tests not built, running build first..."
        do_build
    fi
    
    cd "$BUILD_DIR"
    ctest --output-on-failure --build-config "$BUILD_TYPE"
    cd "$SCRIPT_DIR"
    
    print_success "All tests passed"
}

# Create package
do_package() {
    print_header "Creating Package"
    
    # Build if needed
    do_build
    
    cd "$BUILD_DIR"
    
    OS=$(detect_os)
    
    if [[ "$OS" == "macos" ]]; then
        print_info "Creating macOS DMG package..."
        cpack -G DragNDrop
    elif [[ "$OS" == "linux" ]]; then
        print_info "Creating Linux packages (DEB, RPM)..."
        
        # Check for dpkg-deb
        if command -v dpkg-deb &> /dev/null; then
            cpack -G DEB
            print_success "DEB package created"
        else
            print_info "dpkg-deb not found, skipping DEB package"
        fi
        
        # Check for rpmbuild
        if command -v rpmbuild &> /dev/null; then
            cpack -G RPM
            print_success "RPM package created"
        else
            print_info "rpmbuild not found, skipping RPM package"
        fi
        
        # Always create TGZ as fallback
        cpack -G TGZ
        print_success "TGZ package created"
    fi
    
    cd "$SCRIPT_DIR"
    
    print_success "Package creation complete"
    print_info "Packages are in: $BUILD_DIR/"
    ls -la "$BUILD_DIR"/*.{dmg,deb,rpm,tar.gz} 2>/dev/null || true
}

# Run all steps
do_all() {
    do_clean
    do_build
    do_test
    do_package
}

# Show help
show_help() {
    echo "Termidash Build Script"
    echo ""
    echo "Usage: $0 [command]"
    echo ""
    echo "Commands:"
    echo "  clean      Remove build directory"
    echo "  configure  Run CMake configuration"
    echo "  build      Build the project (implies configure)"
    echo "  test       Run tests (implies build)"
    echo "  package    Create distribution package (implies build)"
    echo "  all        Run clean, build, test, and package"
    echo "  help       Show this help message"
    echo ""
    echo "Environment variables:"
    echo "  BUILD_TYPE   Set to Debug or Release (default: Release)"
    echo ""
    echo "Examples:"
    echo "  $0 build              # Configure and build"
    echo "  $0 test               # Build and run tests"
    echo "  BUILD_TYPE=Debug $0 build  # Debug build"
}

# Check for BUILD_TYPE override
if [ -n "$BUILD_TYPE_OVERRIDE" ]; then
    BUILD_TYPE="$BUILD_TYPE_OVERRIDE"
fi

# Main entry point
cd "$SCRIPT_DIR"

case "${1:-help}" in
    clean)
        do_clean
        ;;
    configure)
        do_configure
        ;;
    build)
        do_build
        ;;
    test)
        do_test
        ;;
    package)
        do_package
        ;;
    all)
        do_all
        ;;
    help|--help|-h)
        show_help
        ;;
    *)
        print_error "Unknown command: $1"
        echo ""
        show_help
        exit 1
        ;;
esac
