#!/bin/bash
################################################################################
# VisionBox Code Formatter
# Automatically formats all C++ source files using clang-format
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

echo -e "${GREEN}VisionBox Code Formatter${NC}"
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
echo -e "${YELLOW}Finding source files...${NC}"

# Create a temporary file with files to format
FILES_TO_FORMAT=$(mktemp)

# Find files but ignore patterns from .clang-format-ignore
find "$PROJECT_ROOT" \
    -type f \( -name "*.h" -o -name "*.cpp" \) \
    ! -path "*/build/*" \
    ! -path "*/external/*" \
    ! -path "*/CMakeFiles/*" \
    ! -name "moc_*" \
    ! -name "qrc_*" \
    ! -name "ui_*" \
    -print > "$FILES_TO_FORMAT"

FILE_COUNT=$(wc -l < "$FILES_TO_FORMAT")
echo "Found $FILE_COUNT files to format"
echo ""

# Format files
if [ "$FILE_COUNT" -eq 0 ]; then
    echo -e "${YELLOW}No files to format${NC}"
    rm -f "$FILES_TO_FORMAT"
    exit 0
fi

echo -e "${YELLOW}Formatting files...${NC}"

# Count of files that were modified
MODIFIED=0
TOTAL=0

while IFS= read -r file; do
    TOTAL=$((TOTAL + 1))
    if [ -f "$file" ]; then
        # Create a backup for comparison
        BACKUP=$(mktemp)
        cp "$file" "$BACKUP"

        # Format the file
        clang-format -i "$file" 2>/dev/null || true

        # Check if file was modified
        if ! cmp -s "$file" "$BACKUP"; then
            echo -e "  ${GREEN}Formatted${NC}: ${file#$PROJECT_ROOT/}"
            MODIFIED=$((MODIFIED + 1))
        fi

        rm -f "$BACKUP"
    fi
done < "$FILES_TO_FORMAT"

rm -f "$FILES_TO_FORMAT"

echo ""
echo "=========================="
if [ "$MODIFIED" -eq 0 ]; then
    echo -e "${GREEN}All files are already formatted!${NC}"
else
    echo -e "${GREEN}Formatted $MODIFIED of $TOTAL files${NC}"
fi

exit 0
