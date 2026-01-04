#!/usr/bin/env python3
"""
Generate digit bitmaps from actual fonts using PIL/Pillow.
Produces crisp, professional-quality digit glyphs.

REQUIREMENTS:
- Python virtual environment with PIL/Pillow and fontTools
- Must run in the font_env virtual environment

SETUP (one time):
    python3 -m venv font_env
    source font_env/bin/activate
    pip install Pillow fontTools

USAGE:
    source font_env/bin/activate
    python3 scripts/generate_digits_from_font.py
    
    (Optional: modify font path and size in the script or hardcoded defaults)

OUTPUT:
    Generates: src/digit_bitmaps.h
    Contains: 12 bitmap glyphs (0-9, :, °) at 120pt from Courier Bold font
    Format: 60x100 pixel monochrome bitmaps for e-ink display
    Rebuild firmware with: pio run -e esp32c3
"""

import os
import sys
from PIL import Image, ImageDraw, ImageFont

def find_helvetica_bold_font():
    """Find SF Mono Heavy font on the system."""
    # macOS and other systems
    fonts_to_try = [
        "/System/Library/Fonts/SF-Mono-Heavy.otf",
        "/System/Library/Fonts/SF-Mono.ttc",  # Index 5 for Heavy
        "/Library/Fonts/SF-Mono-Heavy.otf",
        "/System/Library/Fonts/Courier.ttc",  # Fallback
        "/Library/Fonts/Courier New Bold.ttf",
        "/Library/Fonts/Courier Bold.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Bold.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf",
    ]
    
    for font_path in fonts_to_try:
        if os.path.exists(font_path):
            print(f"Using font: {font_path}")
            return font_path
    
    # Fallback to default
    print("Warning: Could not find SF Mono Heavy, using default PIL font")
    return None

def generate_digit_bitmap(digit_char, font_path=None, font_size=120, width=70, height=110):
    """
    Generate a bitmap for a digit using a real font.
    """
    # Create image
    img = Image.new('1', (width, height), color=0)  # 1-bit white background
    draw = ImageDraw.Draw(img)
    
    # Load font
    if font_path:
        try:
            # Try to load bold/heavy variant (index 1 or 5) from .ttc files, or regular if .ttf
            if font_path.endswith('.ttc'):
                # For .ttc (TrueType Collection), try various indices for Heavy/Bold variants
                font = None
                for index in [5, 1, 0]:  # Try Heavy (5), Bold (1), Regular (0)
                    try:
                        font = ImageFont.truetype(font_path, font_size, index=index)
                        print(f"  Loaded font variant (index={index}) from {font_path}")
                        break
                    except:
                        continue
                if not font:
                    font = ImageFont.truetype(font_path, font_size, index=0)
            else:
                font = ImageFont.truetype(font_path, font_size)
        except Exception as e:
            print(f"Warning: Could not load font {font_path}: {e}")
            font = ImageFont.load_default()
    else:
        font = ImageFont.load_default()
    
    # Get text bounding box to center it
    bbox = draw.textbbox((0, 0), digit_char, font=font)
    text_width = bbox[2] - bbox[0]
    text_height = bbox[3] - bbox[1]
    
    # Center the digit
    x = (width - text_width) // 2 - bbox[0]
    y = (height - text_height) // 2 - bbox[1]
    
    # Draw the digit in black
    draw.text((x, y), digit_char, font=font, fill=1)
    
    # Convert to bytes (1 bit per pixel, packed)
    bytes_per_row = (width + 7) // 8
    bitmap = bytearray(bytes_per_row * height)
    
    pixels = img.load()
    for row in range(height):
        for col in range(width):
            if pixels[col, row]:  # If pixel is black (1)
                byte_idx = row * bytes_per_row + col // 8
                bit_idx = 7 - (col % 8)
                bitmap[byte_idx] |= (1 << bit_idx)
    
    return bytes(bitmap)

def generate_cpp_header(output_file, font_path=None, font_size=120):
    """Generate C++ header with bitmap data."""
    digits = '0123456789:°'
    bitmaps = {}
    
    print("\nGenerating bitmaps from font...")
    for digit in digits:
        bitmap = generate_digit_bitmap(digit, font_path, font_size)
        bitmaps[digit] = bitmap
        print(f"  Generated '{digit}' bitmap ({len(bitmap)} bytes)")
    
    # Write header file
    with open(output_file, 'w') as f:
        f.write("""#ifndef DIGIT_BITMAPS_H
#define DIGIT_BITMAPS_H

#include <Arduino.h>

// Custom bitmap digits rendered from TrueType font
// 60x100 pixels each (monochrome, 1 bit per pixel)
// Auto-generated - do not edit manually
// Use scripts/generate_digits_from_font.py to regenerate

#define DIGIT_WIDTH 70
#define DIGIT_HEIGHT 110
#define DIGIT_BYTES ((DIGIT_WIDTH + 7) / 8 * DIGIT_HEIGHT)  // 1100 bytes per digit

""")
        
        # Write bitmap arrays
        for digit in digits:
            bitmap = bitmaps[digit]
            if digit == ':':
                array_name = "COLON_BITMAP"
            elif digit == '°':
                array_name = "DEGREE_BITMAP"
            else:
                array_name = f"DIGIT_{digit}_BITMAP"
            
            f.write(f"static const unsigned char {array_name}[DIGIT_BYTES] PROGMEM = {{\n")
            
            # Write bitmap data as hex bytes
            for i, byte in enumerate(bitmap):
                if i % 16 == 0:
                    f.write("    ")
                f.write(f"0x{byte:02x}")
                if i < len(bitmap) - 1:
                    f.write(", ")
                if (i + 1) % 16 == 0:
                    f.write("\n")
            
            f.write("\n};\n\n")
        
        # Write lookup function
        f.write("""// Lookup function to get bitmap for a digit
static inline const unsigned char* getDigitBitmap(char digit) {
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wswitch"
    switch (digit) {
""")
        
        for digit in digits:
            if digit == ':':
                f.write(f"        case ':': return COLON_BITMAP;\n")
            elif digit == '°':
                f.write(f"        case '°': return DEGREE_BITMAP;\n")
            else:
                f.write(f"        case '{digit}': return DIGIT_{digit}_BITMAP;\n")
        
        f.write("""        default: return DIGIT_0_BITMAP;
    }
    #pragma GCC diagnostic pop
}

#endif // DIGIT_BITMAPS_H
""")
    
    print(f"\nWrote {output_file}")
    print(f"Header contains {len(digits)} bitmaps, 1100 bytes each = {len(digits) * 1100} bytes total")

if __name__ == "__main__":
    # Determine paths
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    output_file = os.path.join(project_root, "src", "digit_bitmaps.h")
    
    print("Digital Bitmap Generator (From Font)")
    print("=" * 50)
    
    # Find Helvetica Bold font
    font_path = find_helvetica_bold_font()
    
    # Generate with 120pt font size for excellent quality
    print(f"Output: {output_file}")
    generate_cpp_header(output_file, font_path, font_size=120)
    
    print("\nDone! Rebuild with: pio run -e esp32c3")
