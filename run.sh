#!/bin/bash

# Script to run the 3D processing application
# This should be run from a terminal with access to your graphical session

cd "$(dirname "$0")/build"

if [ ! -f "./processing3d" ]; then
    echo "Error: processing3d executable not found. Please build the project first."
    echo "Run: cd build && cmake .. && make"
    exit 1
fi

echo "Starting 3D Grid Navigation..."
echo "Controls:"
echo "  W/A/S/D - Move"
echo "  Mouse   - Look around"
echo "  Space   - Jump"
echo "  ESC     - Exit"
echo ""

./processing3d

exit_code=$?
if [ $exit_code -ne 0 ]; then
    echo ""
    echo "Program exited with error code: $exit_code"
    echo ""
    echo "If you see Wayland/display errors:"
    echo "1. Open a native terminal (Konsole, GNOME Terminal, etc.)"
    echo "2. Navigate to: $(pwd)"
    echo "3. Run: ./processing3d"
    echo ""
    echo "Or run this script from a native terminal:"
    echo "bash $(dirname "$0")/run.sh"
fi
