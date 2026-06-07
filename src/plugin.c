#include "plugin.h"

/* The array of 4-byte selectors used by the SDK to dispatch calls. */
const uint32_t SELECTORS[SELECTOR_COUNT] = {
    SELECTORS_LIST(TO_VALUE)
};
