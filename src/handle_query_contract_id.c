#include "plugin.h"

// Wormhole Portal Token Bridge address (Ethereum mainnet).
static const uint8_t WORMHOLE_TOKEN_BRIDGE[ADDRESS_LENGTH] = {
    0x3e, 0xe1, 0x8b, 0x22, 0x14, 0xaf, 0xf9, 0x70,
    0x00, 0xd9, 0x74, 0xcf, 0x64, 0x7e, 0x7c, 0x34,
    0x7e, 0x8f, 0xa5, 0x85
};

void handle_query_contract_id(ethQueryContractID_t *msg) {
    // Copy the plugin contract address.
    memcpy(msg->pluginContractAddress, WORMHOLE_TOKEN_BRIDGE, ADDRESS_LENGTH);

    // Set a human-readable plugin name shown on the device.
    strlcpy(msg->pluginName, "Wormhole Portal", sizeof(msg->pluginName));

    msg->result = ETH_PLUGIN_RESULT_OK;
}
