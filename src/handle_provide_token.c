#include "plugin.h"

void handle_provide_token(ethPluginProvideToken_t *msg) {
    // Pass-through: accept whatever token info the SDK provides.
    // Optionally token_found and ticker could be filled from shared memory.
    msg->result = ETH_PLUGIN_RESULT_OK;
}
