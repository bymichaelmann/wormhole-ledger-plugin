#include "plugin.h"

void handle_init_contract(ethPluginInitContract_t *msg) {
    context_t *context = (context_t *) msg->pluginContext;

    // Make sure we are running a compatible version.
    if (msg->interfaceVersion != ETH_PLUGIN_INTERFACE_VERSION_LATEST) {
        msg->result = ETH_PLUGIN_RESULT_UNAVAILABLE;
        return;
    }

    // Validate that the context fits in the provided arena.
    if (msg->pluginContextLength < sizeof(context_t)) {
        msg->result = ETH_PLUGIN_RESULT_ERROR;
        return;
    }

    // Reset context.
    memset(context, 0, sizeof(*context));

    // Find which selector was invoked.
    size_t index;
    if (!find_selector(U4BE(msg->selector, 0), SELECTORS, SELECTOR_COUNT, &index)) {
        msg->result = ETH_PLUGIN_RESULT_UNAVAILABLE;
        return;
    }
    context->selectorIndex = (selector_t) index;

    // Set the first parameter to expect during handle_provide_parameter.
    switch (context->selectorIndex) {
        case SEL_WRAP_AND_TRANSFER_ETH:
            context->next_param = RECIPIENT_CHAIN;
            break;

        case SEL_WRAP_AND_TRANSFER_ETH_WITH_PAYLOAD:
            // WITH_PAYLOAD: recipientChain, recipient, nonce (no arbiterFee)
            context->next_param = RECIPIENT_CHAIN;
            break;

        case SEL_TRANSFER_TOKENS:
            context->next_param = TOKEN;
            break;

        case SEL_TRANSFER_TOKENS_WITH_PAYLOAD:
            // WITH_PAYLOAD: token, amount, recipientChain, recipient, nonce (no arbiterFee)
            context->next_param = TOKEN;
            break;

        case SEL_ATTEST_TOKEN:
            context->next_param = TOKEN_ADDRESS;
            break;

        // All remaining selectors (COMPLETE_*, CREATE_WRAPPED, etc.)
        // have either an encodedVM that we do not display, or are
        // non-display functions.  We mark ENCODED_VM and do no parsing.
        default:
            context->next_param = ENCODED_VM;
            break;
    }

    msg->result = ETH_PLUGIN_RESULT_OK;
}
