#!/bin/bash
################################################################################
# VisionBox Format Checker
# Checks if all C++ source files conform to clang-format style
################################################################################

set -e

# Script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}VisionBox Format Checker${NC}"
echo "=========================="
echo ""

# Check if clang-format is available
if ! command -v clang-format &> /dev/null; then
    echo -e "${RED}Error: clang-format not found${NC}"
    echo "Please install clang-format:"
    echo "  Ubuntu/Debian: sudo apt-get install clang-format"
    echo "  Fedora/RHEL:   sudo dnf install clang-tools-extra"
    exit 1
fi

# Display clang-format version
CLANG_VERSION=$(clang-format --version)
echo "Using: $CLANG_VERSION"
echo ""

# Find all .h and .cpp files, excluding generated files and external/
echo -e "${YELLOW}Checking source files...${NC}"
echo ""

# Create a temporary file with files to check
FILES_TO_CHECK=$(mktemp)

find "$PROJECT_ROOT" \
    -type f \( -name "*.h" -o -name "*.cpp" \) \
    ! -path "*/build/*" \
    ! -path "*/external/*" \
    ! -path "*/CMakeFiles/*" \
    ! -name "moc_*" \
    ! -name "qrc_*" \
    ! -name "ui_*" \
    -print > "$FILES_TO_CHECK"

FILE_COUNT=$(wc -l < "$FILES_TO_CHECK")

if [ "$FILE_COUNT" -eq 0 ]; then
    echo -e "${YELLOW}No files to check${NC}"
    rm -f "$FILES_TO_CHECK"
    exit 0
fi

# Check formatting
FAILED=0
TOTAL=0

while IFS= read -r file; do
    TOTAL=$((TOTAL + 1))
    if [ -f "$file" ]; then
        # Check if file is properly formatted
        if ! clang-format "$file" | diff -q "$file" - > /dev/null 2>&1; then
            echo -e "  ${RED}FAIL${NC}: ${file#$PROJECT_ROOT/}"
            FAILED=$((FAILED + 1))
        fi
    fi
done < "$FILES_TO_CHECK"

rm -f "$FILES_TO_CHECK"

echo ""
echo "=========================="
if [ "$FAILED" -eq 0 ]; then
    echo -e "${GREEN}All $TOTAL files are properly formatted!${NC}"
    exit 0
else
    echo -e "${RED}Format check failed! $FAILED of $TOTAL files need formatting${NC}"
    echo ""
    echo "Run './scripts/format_code.sh' to fix formatting issues"
    exit 1
fi
