#!/usr/bin/env python3
"""Convert PNG icons to C bitmap arrays for GxEPD2"""

from PIL import Image
import os
from pathlib import Path

def png_to_bitmap_c(png_path, output_size=32):
    """Convert PNG to monochrome bitmap C array"""
    img = Image.open(png_path)
    
    # Resize to target size if needed
    if img.size != (output_size, output_size):
        img = img.resize((output_size, output_size), Image.Resampling.LANCZOS)
    
    # Handle alpha channel - flatten to white background
    if img.mode == 'RGBA' or img.mode == 'LA' or 'transparency' in img.info:
        background = Image.new('RGB', img.size, (255, 255, 255))
        if img.mode == 'RGBA':
            background.paste(img, mask=img.split()[3])
        else:
            background.paste(img)
        img = background
    elif img.mode != 'RGB':
        img = img.convert('RGB')
    
    # Convert to monochrome (1-bit) - this will use threshold at 128
    img = img.convert('1')
    
    # Get pixel data
    pixels = list(img.getdata())
    
    # Convert to bytes (pack bits)
    bytes_list = []
    for i in range(0, len(pixels), 8):
        byte = 0
        for j in range(8):
            if i + j < len(pixels):
                # PIL mode '1': 0 = black, 255 = white
                # Set bit if pixel is BLACK (value 0)
                if pixels[i + j] == 0:  # Black pixel
                    byte |= (1 << (7 - j))
        bytes_list.append(byte)
    
    return bytes_list, output_size

def generate_c_code(png_folder, output_size=32):
    """Generate C header with all bitmap arrays"""
    
    png_files = sorted(Path(png_folder).glob(f'*-{output_size}.png'))
    
    c_code = f"""#ifndef WEATHER_BITMAPS_H
#define WEATHER_BITMAPS_H

// Auto-generated from PNG files - {output_size}x{output_size} monochrome bitmaps
// Each bitmap is {output_size*output_size//8} bytes

"""
    
    for png_file in png_files:
        name = png_file.stem.replace('-', '_').replace(f'_{output_size}', '')
        name = name.replace('icons8_', '')
        
        bytes_list, size = png_to_bitmap_c(str(png_file), output_size)
        
        c_code += f"const unsigned char {name}_{output_size}x{output_size}[] PROGMEM = {{\n"
        
        # Format bytes nicely
        for i, byte in enumerate(bytes_list):
            if i % 16 == 0:
                c_code += "  "
            c_code += f"0x{byte:02x}"
            if i < len(bytes_list) - 1:
                c_code += ", "
            if (i + 1) % 16 == 0:
                c_code += "\n"
        
        c_code += "\n};\n\n"
    
    c_code += "#endif // WEATHER_BITMAPS_H\n"
    
    return c_code

# Generate for 32x32
output_dir = "/Users/colin/Projects/trmnl-view/src"
output_size = 32

code = generate_c_code("/Users/colin/Projects/trmnl-view/weather-bitmaps", output_size)

output_file = os.path.join(output_dir, "weather_bitmaps.h")
with open(output_file, 'w') as f:
    f.write(code)

print(f"✓ Generated {output_file}")
print(f"  Size: {len(code)} bytes of C code")

# Also show file sizes
import subprocess
result = subprocess.run(['du', '-h', output_file], capture_output=True, text=True)
print(f"  File size: {result.stdout.strip()}")
