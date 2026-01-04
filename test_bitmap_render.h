// Test to verify bitmap rendering compiles and basic logic works
#include "digit_bitmaps.h"

void testBitmapLookup()
{
    // Test that all digits can be looked up
    for (char c = '0'; c <= '9'; c++)
    {
        const unsigned char *bitmap = getDigitBitmap(c);
        if (bitmap == NULL)
        {
            // Error: bitmap lookup failed
            return;
        }
    }

    // Test colon
    const unsigned char *colonBitmap = getDigitBitmap(':');
    if (colonBitmap == NULL)
    {
        // Error: colon bitmap lookup failed
        return;
    }

    // All tests passed
}
