#!/bin/bash
################################################################################
# VisionBox Git Hooks Setup
# Installs pre-commit and pre-push Git hooks for code quality
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

echo -e "${GREEN}VisionBox Git Hooks Setup${NC}"
echo "=========================="
echo ""

# Git hooks directory
GIT_HOOKS_DIR="$PROJECT_ROOT/.git/hooks"

# Check if we're in a git repository
if [ ! -d "$GIT_HOOKS_DIR" ]; then
    echo -e "${RED}Error: Not in a Git repository${NC}"
    echo "Please initialize a Git repository first:"
    echo "  git init"
    exit 1
fi

echo -e "${YELLOW}Installing Git hooks...${NC}"
echo ""

# Create pre-commit hook
cat > "$GIT_HOOKS_DIR/pre-commit" << 'EOF'
#!/bin/bash
# VisionBox Pre-Commit Hook
# Runs code formatting check before committing

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

echo "Running pre-commit checks..."

# Run format check
if ! "$PROJECT_ROOT/scripts/check_format.sh"; then
    echo ""
    echo "Pre-commit check failed!"
    echo "Run './scripts/format_code.sh' to fix formatting issues"
    echo "Or commit with --no-verify to skip this check"
    exit 1
fi

echo "Pre-commit checks passed!"
exit 0
EOF

chmod +x "$GIT_HOOKS_DIR/pre-commit"
echo -e "  ${GREEN}Installed${NC}: pre-commit hook"

# Create pre-push hook
cat > "$GIT_HOOKS_DIR/pre-push" << 'EOF'
#!/bin/bash
# VisionBox Pre-Push Hook
# Runs static analysis before pushing to remote

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

echo "Running pre-push checks..."

# Check if we want to skip this hook
if [ "$SKIP_TIDY" = "1" ]; then
    echo "Skipping clang-tidy (SKIP_TIDY=1)"
    exit 0
fi

# Check if clang-tidy is available
if ! command -v clang-tidy &> /dev/null; then
    echo "Warning: clang-tidy not found, skipping static analysis"
    exit 0
fi

# Run clang-tidy (non-blocking, just warn)
echo "Running clang-tidy static analysis..."
if ! "$PROJECT_ROOT/scripts/run_tidy.sh"; then
    echo ""
    echo "Pre-push check found issues!"
    echo "Review the output above and fix critical issues"
    echo "Or push with SKIP_TIDY=1 git push to skip this check"
    read -p "Continue anyway? (y/N) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

echo "Pre-push checks passed!"
exit 0
EOF

chmod +x "$GIT_HOOKS_DIR/pre-push"
echo -e "  ${GREEN}Installed${NC}: pre-push hook"

echo ""
echo "=========================="
echo -e "${GREEN}Git hooks installed successfully!${NC}"
echo ""
echo "Installed hooks:"
echo "  • pre-commit  - Checks code formatting before commit"
echo "  • pre-push    - Runs static analysis before push"
echo ""
echo "To bypass hooks temporarily:"
echo "  git commit --no-verify"
echo "  SKIP_TIDY=1 git push"
echo ""
exit 0
