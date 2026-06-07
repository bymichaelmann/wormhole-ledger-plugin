/*****************************************************************************
 * Chain name glyphs and helpers for the Wormhole Portal plugin.
 * These provide bitmap representations for common chain logos.
 *
 * For MVP, we provide only the Wormhole logo glyph.
 * Chain names are rendered as text on the Ledger display.
 *****************************************************************************/

#ifndef CHAIN_NAMES_H
#define CHAIN_NAMES_H

#include "glyphs/wormhole_logo.h"

/* Chain ID to glyph mapping (for future use with chain-specific icons). */
static inline const unsigned char *get_chain_glyph(uint16_t chain_id) {
    (void) chain_id;
    return C_wormhole_logo;
}

#endif  // CHAIN_NAMES_H
