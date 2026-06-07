#include "plugin.h"

void handle_finalize(ethPluginFinalize_t *msg) {
    // No additional processing needed.
    msg->result = ETH_PLUGIN_RESULT_OK;
}
