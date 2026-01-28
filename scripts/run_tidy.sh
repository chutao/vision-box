#!/bin/bash
################################################################################
# VisionBox Static Analysis
# Runs clang-tidy static analysis on all C++ source files
################################################################################

set -e

# Script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$PROJECT_ROOT/build}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${GREEN}VisionBox Static Analysis (clang-tidy)${NC}"
echo "======================================"
echo ""

# Check if clang-tidy is available
if ! command -v clang-tidy &> /dev/null; then
    echo -e "${RED}Error: clang-tidy not found${NC}"
    echo "Please install clang-tidy:"
    echo "  Ubuntu/Debian: sudo apt-get install clang-tidy"
    echo "  Fedora/RHEL:   sudo dnf install clang-tools-extra"
    exit 1
fi

# Display clang-tidy version
TIDY_VERSION=$(clang-tidy --version)
echo "Using: $TIDY_VERSION"
echo ""

# Check if build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${YELLOW}Build directory not found: $BUILD_DIR${NC}"
    echo "Creating build directory and running CMake..."
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    cmake ..
    cd "$PROJECT_ROOT"
fi

# Check for compile_commands.json
if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
    echo -e "${YELLOW}compile_commands.json not found${NC}"
    echo "Running CMake to generate compile_commands.json..."
    cd "$BUILD_DIR"
    cmake ..
    cd "$PROJECT_ROOT"
fi

echo -e "${BLUE}Build directory: $BUILD_DIR${NC}"
echo ""

# Find all source files to analyze
echo -e "${YELLOW}Finding source files...${NC}"

FILES_TO_ANALYZE=$(mktemp)

find "$PROJECT_ROOT/src" \
    -type f \( -name "*.cpp" \) \
    ! -name "moc_*" \
    -print > "$FILES_TO_ANALYZE"

FILE_COUNT=$(wc -l < "$FILES_TO_ANALYZE")

if [ "$FILE_COUNT" -eq 0 ]; then
    echo -e "${YELLOW}No source files to analyze${NC}"
    rm -f "$FILES_TO_ANALYZE"
    exit 0
fi

echo "Found $FILE_COUNT source files"
echo ""

# Run clang-tidy on each file
echo -e "${YELLOW}Running clang-tidy...${NC}"
echo ""

TOTAL=0
PASSED=0
FAILED=0

while IFS= read -r file; do
    TOTAL=$((TOTAL + 1))
    FILE_RELATIVE="${file#$PROJECT_ROOT/}"

    echo -e "${BLUE}Analyzing${NC}: $FILE_RELATIVE"

    # Run clang-tidy with fix mode if requested
    if [ "$FIX" = "1" ]; then
        if clang-tidy "$file" \
            -p "$BUILD_DIR" \
            -format-style=file \
            -fix \
            2>&1 | tee -a "$BUILD_DIR/clang-tidy.log"; then
            PASSED=$((PASSED + 1))
        else
            FAILED=$((FAILED + 1))
        fi
    else
        if clang-tidy "$file" \
            -p "$BUILD_DIR" \
            -format-style=file \
            2>&1 | tee -a "$BUILD_DIR/clang-tidy.log"; then
            PASSED=$((PASSED + 1))
        else
            FAILED=$((FAILED + 1))
        fi
    fi
    echo ""
done < "$FILES_TO_ANALYZE"

rm -f "$FILES_TO_ANALYZE"

echo "======================================"
echo "Analysis complete!"
echo "  Total:  $TOTAL"
echo -e "  ${GREEN}Passed: $PASSED${NC}"
if [ "$FAILED" -gt 0 ]; then
    echo -e "  ${RED}Failed: $FAILED${NC}"
    echo ""
    echo "Check $BUILD_DIR/clang-tidy.log for details"
    echo ""
    echo "To auto-fix issues, run: FIX=1 ./scripts/run_tidy.sh"
    exit 1
else
    echo ""
    echo -e "${GREEN}No issues found!${NC}"
    exit 0
fi
