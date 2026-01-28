#!/bin/bash
#
# VisionBox Launcher
#
# This script automatically detects the display server type and chooses the
# appropriate Qt backend for optimal compatibility.
#
# For Wayland sessions: Uses X11 backend (via XWayland) to avoid Qt 5.15
#                     dialog rendering issues.
# For X11 sessions:      Uses default backend.
#
# See docs/WAYLAND_COMPATIBILITY.md for details.

# Detect session type
SESSION_TYPE="${XDG_SESSION_TYPE:-}"
WAYLAND_DISPLAY="${WAYLAND_DISPLAY:-}"

# Function to show info message
show_info() {
    if [ -t 1 ]; then  # Only print if running in terminal
        echo "[VisionBox] $1"
    fi
}

# Check if we're in a Wayland session
if [ "$SESSION_TYPE" = "wayland" ] && [ -n "$WAYLAND_DISPLAY" ]; then
    # Wayland detected: use X11 backend for compatibility
    export QT_QPA_PLATFORM=xcb
    show_info "Wayland session detected, using X11 backend for compatibility"
    show_info "See docs/WAYLAND_COMPATIBILITY.md for more information"
else
    show_info "Using default Qt backend"
fi

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Launch VisionBox
exec "$SCRIPT_DIR/VisionBox" "$@"

