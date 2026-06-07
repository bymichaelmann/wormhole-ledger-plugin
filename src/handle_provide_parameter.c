#include "plugin.h"

static void handle_wrap_and_transfer_eth(ethPluginProvideParameter_t *msg,
                                         context_t *context) {
    switch (context->next_param) {
        case RECIPIENT_CHAIN:
            // recipientChain is a uint16 at the end of the 32-byte word.
            context->data.handle_wrap_and_transfer_eth_data.recipient_chain =
                U2BE(msg->parameter, PARAMETER_LENGTH - 2);
            context->next_param = RECIPIENT;
            break;

        case RECIPIENT:
            copy_parameter(context->data.handle_wrap_and_transfer_eth_data.recipient,
                           msg->parameter, PARAMETER_LENGTH);
            context->next_param = ARBITER_FEE;
            break;

        case ARBITER_FEE:
            copy_parameter(context->data.handle_wrap_and_transfer_eth_data.arbiter_fee,
                           msg->parameter, PARAMETER_LENGTH);
            context->next_param = NONCE;
            break;

        case NONCE:
            context->data.handle_wrap_and_transfer_eth_data.nonce =
                U4BE(msg->parameter, PARAMETER_LENGTH - 4);
            context->next_param = UNEXPECTED_PARAMETER;
            break;

        default:
            // No more parameters to parse.
            break;
    }
}

static void handle_transfer_tokens(ethPluginProvideParameter_t *msg,
                                   context_t *context) {
    switch (context->next_param) {
        case TOKEN:
            copy_parameter(context->data.handle_transfer_tokens_data.token,
                           msg->parameter, PARAMETER_LENGTH);
            context->next_param = AMOUNT;
            break;

        case AMOUNT:
            copy_parameter(context->data.handle_transfer_tokens_data.amount,
                           msg->parameter, PARAMETER_LENGTH);
            context->next_param = RECIPIENT_CHAIN;
            break;

        case RECIPIENT_CHAIN:
            context->data.handle_transfer_tokens_data.recipient_chain =
                U2BE(msg->parameter, PARAMETER_LENGTH - 2);
            context->next_param = RECIPIENT;
            break;

        case RECIPIENT:
            copy_parameter(context->data.handle_transfer_tokens_data.recipient,
                           msg->parameter, PARAMETER_LENGTH);
            context->next_param = ARBITER_FEE;
            break;

        case ARBITER_FEE:
            copy_parameter(context->data.handle_transfer_tokens_data.arbiter_fee,
                           msg->parameter, PARAMETER_LENGTH);
            context->next_param = NONCE;
            break;

        case NONCE:
            context->data.handle_transfer_tokens_data.nonce =
                U4BE(msg->parameter, PARAMETER_LENGTH - 4);
            context->next_param = UNEXPECTED_PARAMETER;
            break;

        default:
            break;
    }
}

static void handle_attest_token(ethPluginProvideParameter_t *msg,
                                context_t *context) {
    switch (context->next_param) {
        case TOKEN_ADDRESS:
            copy_parameter(context->data.handle_attest_token_data.token_address,
                           msg->parameter, PARAMETER_LENGTH);
            context->next_param = NONCE;
            break;

        case NONCE:
            context->data.handle_attest_token_data.nonce =
                U4BE(msg->parameter, PARAMETER_LENGTH - 4);
            context->next_param = UNEXPECTED_PARAMETER;
            break;

        default:
            break;
    }
}

void handle_provide_parameter(ethPluginProvideParameter_t *msg) {
    context_t *context = (context_t *) msg->pluginContext;

    // For non-display selectors, skip parsing entirely.
    switch (context->selectorIndex) {
        case SEL_WRAP_AND_TRANSFER_ETH:
        case SEL_WRAP_AND_TRANSFER_ETH_WITH_PAYLOAD:
            handle_wrap_and_transfer_eth(msg, context);
            break;

        case SEL_TRANSFER_TOKENS:
        case SEL_TRANSFER_TOKENS_WITH_PAYLOAD:
            handle_transfer_tokens(msg, context);
            break;

        case SEL_ATTEST_TOKEN:
            handle_attest_token(msg, context);
            break;

        default:
            // COMPLETE_* and admin functions: do nothing.
            context->next_param = UNEXPECTED_PARAMETER;
            break;
    }

    msg->result = ETH_PLUGIN_RESULT_OK;
}
