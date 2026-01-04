#!/bin/bash
# Convenient wrapper to regenerate digit bitmaps from fonts

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VENV_PATH="$PROJECT_ROOT/font_env"

# Ensure virtual environment exists
if [ ! -d "$VENV_PATH" ]; then
    echo "Creating Python virtual environment..."
    python3 -m venv "$VENV_PATH"
fi

# Install dependencies
echo "Installing dependencies (Pillow + fontTools)..."
source "$VENV_PATH/bin/activate"
pip install -q Pillow fonttools

# Run the generator
echo ""
echo "Generating bitmaps from font..."
python3 "$PROJECT_ROOT/scripts/generate_digits_from_font.py"

echo ""
echo "✓ Bitmaps generated successfully!"
echo ""
echo "Next steps:"
echo "  1. Build: pio run -e esp32c3"
echo "  2. Upload: pio run -t upload -e esp32c3"
echo "  3. Monitor: pio device monitor"
