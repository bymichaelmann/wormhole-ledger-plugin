#include "plugin.h"

void handle_query_contract_id(ethQueryContractID_t *msg) {
    // Plugin name and version shown on the Ledger device before signing.
    strlcpy(msg->name, "Wormhole Portal", msg->nameLength);
    strlcpy(msg->version, "1.0.0", msg->versionLength);

    msg->result = ETH_PLUGIN_RESULT_OK;
}
