/*****************************************************************************
 * Wormhole Portal logo glyph (14x14 1-bit bitmap)
 * Displayed on Ledger device during transaction confirmation.
 *
 * This bitmap represents the Wormhole "W" icon in 1-bit format suitable
 * for the Ledger Nano S/X/Stax display.
 *****************************************************************************/

#ifndef WORMHOLE_GLYPH_H
#define WORMHOLE_GLYPH_H

#include "os.h"

/* 14x14 pixel monochrome bitmap (1 bit per pixel, packed row-wise).
   Each row is padded to the next byte boundary (2 bytes per row). */
static const unsigned char C_wormhole_logo[] = {
    /* Row 0 */ 0b00000000, 0b00000000,
    /* Row 1 */ 0b00000000, 0b00000000,
    /* Row 2 */ 0b00100100, 0b10000000,
    /* Row 3 */ 0b00111111, 0b10000000,
    /* Row 4 */ 0b00111111, 0b10000000,
    /* Row 5 */ 0b01100110, 0b11000000,
    /* Row 6 */ 0b01100110, 0b11000000,
    /* Row 7 */ 0b01100110, 0b11000000,
    /* Row 8 */ 0b01100110, 0b11000000,
    /* Row 9 */ 0b00111111, 0b10000000,
    /* Row 10 */ 0b00111111, 0b10000000,
    /* Row 11 */ 0b00011011, 0b00000000,
    /* Row 12 */ 0b00000000, 0b00000000,
    /* Row 13 */ 0b00000000, 0b00000000,
};

#endif  // WORMHOLE_GLYPH_H
