#include "plugin.h"

// ---------------------------------------------------------------------------
// Chain ID to human-readable name mapping (Wormhole chain IDs).
// chain_names1 covers IDs 1..35 (index offset by 1).
// chain_names2 covers IDs 4000..4004.
// chain_names3 covers IDs 10002..10006.
// ---------------------------------------------------------------------------

static const char *const chain_names1[] = {
    "UNUSED",            // 0  (placeholder, never used)
    "Solana",            // 1
    "Ethereum",          // 2
    "Terra Classic",     // 3
    "Bsc",               // 4
    "Polygon",           // 5
    "Avalanche",         // 6
    "Oasis",             // 7
    "Algorand",          // 8
    "Aurora",            // 9
    "Fantom",            // 10
    "Karura",            // 11
    "Acala",             // 12
    "Klaytn",            // 13
    "Celo",              // 14
    "Near",              // 15
    "Moonbeam",          // 16
    "Neon",              // 17
    "Terra2",            // 18
    "Injective",         // 19
    "Osmosis",           // 20
    "Sui",               // 21
    "Aptos",             // 22
    "Arbitrum",          // 23
    "Optimism",          // 24
    "Gnosis",            // 25
    "Pythnet",           // 26
    "UNUSED",            // 27
    "XPLA",              // 28
    "Bitcoin",           // 29
    "Base",              // 30
    "UNUSED",            // 31
    "Sei",               // 32
    "Rootstock",         // 33
    "Scroll",            // 34
    "Mantle",            // 35
};

static const char *const chain_names2[] = {
    "Cosmos Hub",        // 4000
    "Evmos",             // 4001
    "Kujira",            // 4002
    "Neutron",           // 4003
    "Celestia",          // 4004
};

static const char *const chain_names3[] = {
    "Sepolia",           // 10002
    "Arbitrum Sepolia",  // 10003
    "Base Sepolia",      // 10004
    "Optimism Sepolia",  // 10005
    "Holesky",           // 10006
};

// ---------------------------------------------------------------------------
// Helper: resolve a Wormhole chain ID to a display string.
// Returns a pointer to a static string.
// ---------------------------------------------------------------------------
static const char *get_chain_name(uint16_t chain_id) {
    if (chain_id == 3104) {
        return "Wormhole";
    }
    if (chain_id >= 4000 && chain_id <= 4004) {
        return chain_names2[chain_id - 4000];
    }
    if (chain_id >= 10002 && chain_id <= 10006) {
        return chain_names3[chain_id - 10002];
    }
    if (chain_id < sizeof(chain_names1) / sizeof(chain_names1[0])) {
        return chain_names1[chain_id];
    }
    return "Unknown";
}

// ---------------------------------------------------------------------------
// Helper: format a 32-byte parameter as a 0x-prefixed hex string.
// Shows all 32 bytes for non-EVM destinations, last 20 bytes for EVM chains.
// ---------------------------------------------------------------------------
static bool is_evm_chain(uint16_t chain_id) {
    switch (chain_id) {
        case 2:   // Ethereum
        case 5:   // Polygon (EVM)
        case 6:   // Avalanche C-Chain
        case 9:   // Aurora (NEAR EVM)
        case 10:  // Fantom
        case 11:  // Karura EVM
        case 12:  // Acala EVM
        case 13:  // Klaytn
        case 14:  // Celo
        case 16:  // Moonbeam
        case 17:  // Neon (Solana EVM)
        case 23:  // Arbitrum
        case 24:  // Optimism
        case 25:  // Gnosis
        case 30:  // Base
        case 32:  // Sei EVM
        case 33:  // Rootstock
        case 34:  // Scroll
        case 35:  // Mantle
            return true;
        default:
            return false;
    }
}

static void format_address(const uint8_t *in, uint16_t chain_id, char *out, size_t out_len) {
    snprintf(out, out_len, "0x");
    int start = is_evm_chain(chain_id) ? 12 : 0;  // EVM: last 20 bytes, non-EVM: all 32
    for (int i = start; i < 32; i++) {
        char buf[3];
        snprintf(buf, sizeof(buf), "%02x", in[i]);
        strlcat(out, buf, out_len);
    }
}

// ---------------------------------------------------------------------------
// Helper: format a 32-byte big-endian uint256 as a decimal string.
// ---------------------------------------------------------------------------
static void format_amount(const uint8_t *amount, char *out, size_t out_len) {
    uint256_to_decimal(amount, 32, out, out_len);
}

// ---------------------------------------------------------------------------
// Helper: format a nonce (uint32) as decimal.
// ---------------------------------------------------------------------------
static void format_nonce(uint32_t nonce, char *out, size_t out_len) {
    snprintf(out, out_len, "%u", (unsigned int) nonce);
}

// ---------------------------------------------------------------------------
// UI screen setters
// ---------------------------------------------------------------------------

static void set_dst_chain(ethQueryContractUI_t *msg, context_t *context) {
    uint16_t chain_id;
    switch (context->selectorIndex) {
        case SEL_WRAP_AND_TRANSFER_ETH:
            chain_id = context->data.handle_wrap_and_transfer_eth_data.recipient_chain;
            break;
        case SEL_WRAP_AND_TRANSFER_ETH_WITH_PAYLOAD:
            chain_id = context->data.handle_wrap_and_transfer_eth_with_payload_data.recipient_chain;
            break;
        case SEL_TRANSFER_TOKENS:
            chain_id = context->data.handle_transfer_tokens_data.recipient_chain;
            break;
        case SEL_TRANSFER_TOKENS_WITH_PAYLOAD:
            chain_id = context->data.handle_transfer_tokens_with_payload_data.recipient_chain;
            break;
        default:
            chain_id = 0;
            break;
    }
    const char *name = get_chain_name(chain_id);
    strlcpy(msg->title, "Dst Chain", msg->titleLength);
    strlcpy(msg->msg, name, msg->msgLength);
}

static void set_recipient(ethQueryContractUI_t *msg, context_t *context) {
    uint8_t *rec;
    uint16_t chain_id = 0;
    switch (context->selectorIndex) {
        case SEL_WRAP_AND_TRANSFER_ETH:
            rec = context->data.handle_wrap_and_transfer_eth_data.recipient;
            chain_id = context->data.handle_wrap_and_transfer_eth_data.recipient_chain;
            break;
        case SEL_WRAP_AND_TRANSFER_ETH_WITH_PAYLOAD:
            rec = context->data.handle_wrap_and_transfer_eth_with_payload_data.recipient;
            chain_id = context->data.handle_wrap_and_transfer_eth_with_payload_data.recipient_chain;
            break;
        case SEL_TRANSFER_TOKENS:
            rec = context->data.handle_transfer_tokens_data.recipient;
            chain_id = context->data.handle_transfer_tokens_data.recipient_chain;
            break;
        case SEL_TRANSFER_TOKENS_WITH_PAYLOAD:
            rec = context->data.handle_transfer_tokens_with_payload_data.recipient;
            chain_id = context->data.handle_transfer_tokens_with_payload_data.recipient_chain;
            break;
        default:
            rec = context->data.handle_wrap_and_transfer_eth_data.recipient;
            break;
    }
    strlcpy(msg->title, "Recipient", msg->titleLength);
    format_address(rec, chain_id, msg->msg, msg->msgLength);
}

static void set_arbiter_fee(ethQueryContractUI_t *msg, context_t *context) {
    uint8_t *fee;
    switch (context->selectorIndex) {
        case SEL_WRAP_AND_TRANSFER_ETH:
            fee = context->data.handle_wrap_and_transfer_eth_data.arbiter_fee;
            break;
        case SEL_TRANSFER_TOKENS:
            fee = context->data.handle_transfer_tokens_data.arbiter_fee;
            break;
        default:
            fee = context->data.handle_wrap_and_transfer_eth_data.arbiter_fee;
            break;
    }
    strlcpy(msg->title, "Arbiter Fee", msg->titleLength);
    format_amount(fee, msg->msg, msg->msgLength);
    strlcat(msg->msg, " wei", msg->msgLength);
}

static void set_nonce(ethQueryContractUI_t *msg, context_t *context) {
    uint32_t nonce;
    switch (context->selectorIndex) {
        case SEL_WRAP_AND_TRANSFER_ETH:
            nonce = context->data.handle_wrap_and_transfer_eth_data.nonce;
            break;
        case SEL_WRAP_AND_TRANSFER_ETH_WITH_PAYLOAD:
            nonce = context->data.handle_wrap_and_transfer_eth_with_payload_data.nonce;
            break;
        case SEL_TRANSFER_TOKENS:
            nonce = context->data.handle_transfer_tokens_data.nonce;
            break;
        case SEL_TRANSFER_TOKENS_WITH_PAYLOAD:
            nonce = context->data.handle_transfer_tokens_with_payload_data.nonce;
            break;
        case SEL_ATTEST_TOKEN:
            nonce = context->data.handle_attest_token_data.nonce;
            break;
        default:
            nonce = 0;
            break;
    }
    strlcpy(msg->title, "Nonce", msg->titleLength);
    format_nonce(nonce, msg->msg, msg->msgLength);
}

static void set_token_address(ethQueryContractUI_t *msg, context_t *context) {
    strlcpy(msg->title, "Token Address", msg->titleLength);
    // token_address is ABI-encoded: 12 zero bytes + 20-byte EVM address
    format_address(context->data.handle_attest_token_data.token_address,
                   2, msg->msg, msg->msgLength);
}

static void set_token(ethQueryContractUI_t *msg, context_t *context) {
    strlcpy(msg->title, "Token", msg->titleLength);
    // token is ABI-encoded: 12 zero bytes + 20-byte EVM address
    format_address(context->data.handle_transfer_tokens_data.token,
                   2, msg->msg, msg->msgLength);
}

static void set_amount(ethQueryContractUI_t *msg, context_t *context) {
    strlcpy(msg->title, "Amount", msg->titleLength);
    format_amount(context->data.handle_transfer_tokens_data.amount,
                  msg->msg, msg->msgLength);
    strlcat(msg->msg, " tokens", msg->msgLength);
}

// ---------------------------------------------------------------------------
// Main UI query handler
// ---------------------------------------------------------------------------

void handle_query_contract_ui(ethQueryContractUI_t *msg) {
    context_t *context = (context_t *) msg->pluginContext;

    // Reset UI fields.
    memset(msg->title, 0, msg->titleLength);
    memset(msg->msg, 0, msg->msgLength);

    switch (context->selectorIndex) {
        // ---------------------------------------------------------------
        // WRAP_AND_TRANSFER_ETH
        // Screens: 0=Dst Chain, 1=Recipient, 2=Arbiter Fee, 3=Nonce
        // ---------------------------------------------------------------
        case SEL_WRAP_AND_TRANSFER_ETH:
            switch (msg->screenIndex) {
                case 0:
                    set_dst_chain(msg, context);
                    break;
                case 1:
                    set_recipient(msg, context);
                    break;
                case 2:
                    set_arbiter_fee(msg, context);
                    break;
                case 3:
                    set_nonce(msg, context);
                    break;
                default:
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
            }
            break;

        // ---------------------------------------------------------------
        // WRAP_AND_TRANSFER_ETH_WITH_PAYLOAD
        // Screens: 0=Dst Chain, 1=Recipient, 2=Nonce (NO Arbiter Fee)
        // ---------------------------------------------------------------
        case SEL_WRAP_AND_TRANSFER_ETH_WITH_PAYLOAD:
            switch (msg->screenIndex) {
                case 0:
                    set_dst_chain(msg, context);
                    break;
                case 1:
                    set_recipient(msg, context);
                    break;
                case 2:
                    set_nonce(msg, context);
                    break;
                default:
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
            }
            break;

        // ---------------------------------------------------------------
        // TRANSFER_TOKENS
        // Screens: 0=Token, 1=Amount, 2=Dst Chain, 3=Recipient,
        //          4=Arbiter Fee, 5=Nonce
        // ---------------------------------------------------------------
        case SEL_TRANSFER_TOKENS:
            switch (msg->screenIndex) {
                case 0:
                    set_token(msg, context);
                    break;
                case 1:
                    set_amount(msg, context);
                    break;
                case 2:
                    set_dst_chain(msg, context);
                    break;
                case 3:
                    set_recipient(msg, context);
                    break;
                case 4:
                    set_arbiter_fee(msg, context);
                    break;
                case 5:
                    set_nonce(msg, context);
                    break;
                default:
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
            }
            break;

        // ---------------------------------------------------------------
        // TRANSFER_TOKENS_WITH_PAYLOAD
        // Screens: 0=Token, 1=Amount, 2=Dst Chain, 3=Recipient,
        //          4=Nonce (NO Arbiter Fee)
        // ---------------------------------------------------------------
        case SEL_TRANSFER_TOKENS_WITH_PAYLOAD:
            switch (msg->screenIndex) {
                case 0:
                    set_token(msg, context);
                    break;
                case 1:
                    set_amount(msg, context);
                    break;
                case 2:
                    set_dst_chain(msg, context);
                    break;
                case 3:
                    set_recipient(msg, context);
                    break;
                case 4:
                    set_nonce(msg, context);
                    break;
                default:
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
            }
            break;

        // ---------------------------------------------------------------
        // ATTEST_TOKEN
        // Screens: 0=Token Address, 1=Nonce
        // ---------------------------------------------------------------
        case SEL_ATTEST_TOKEN:
            switch (msg->screenIndex) {
                case 0:
                    set_token_address(msg, context);
                    break;
                case 1:
                    set_nonce(msg, context);
                    break;
                default:
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
            }
            break;

        // ---------------------------------------------------------------
        // All other selectors (COMPLETE_*, admin functions):
        // no screens — just confirm.
        // ---------------------------------------------------------------
        default:
            // No screens to display; the SDK will show a generic confirm.
            msg->result = ETH_PLUGIN_RESULT_OK;
            return;
    }

    msg->result = ETH_PLUGIN_RESULT_OK;
}
