#pragma once

#include "eth_plugin_interface.h"

/******************************************************************************
 * SELECTORS_LIST — every Wormhole Portal Token Bridge function selector.
 * Expandable: the 4-byte selector constant is produced via TO_VALUE.
 ******************************************************************************/
#define SELECTORS_LIST(X)                                    \
    X(ATTEST_TOKEN, 0xc48fa115)                              \
    X(WRAP_AND_TRANSFER_ETH, 0x9981509f)                     \
    X(WRAP_AND_TRANSFER_ETH_WITH_PAYLOAD, 0xbee9cdfc)        \
    X(TRANSFER_TOKENS, 0x0f5287b0)                           \
    X(TRANSFER_TOKENS_WITH_PAYLOAD, 0xc5a5ebda)              \
    X(COMPLETE_TRANSFER, 0xc6878519)                         \
    X(COMPLETE_TRANSFER_WITH_PAYLOAD, 0xc3f511c1)            \
    X(COMPLETE_TRANSFER_AND_UNWRAP_ETH, 0xff200cde)          \
    X(COMPLETE_TRANSFER_AND_UNWRAP_ETH_WITH_PAYLOAD, 0x1c8475e4) \
    X(CREATE_WRAPPED, 0xe8059810)                            \
    X(UPDATE_WRAPPED, 0xf768441f)                            \
    X(REGISTER_CHAIN, 0xa5799f93)                            \
    X(SUBMIT_RECOVER_CHAIN_ID, 0x178149e7)                   \
    X(UPGRADE, 0x25394645)

/* Convenience macros to expand the list into an enum of selector indices. */
#define TO_ENUM(name, id) SEL_##name,
#define TO_VALUE(name, id) id,

typedef enum selector_e {
    SELECTORS_LIST(TO_ENUM)
    SELECTOR_COUNT,
} selector_t;

/* The array of 4-byte selectors defined in plugin.c. */
extern const uint32_t SELECTORS[SELECTOR_COUNT];

/******************************************************************************
 * Parameter enum — corresponds to the order each function expects its
 * ABI-encoded parameters.
 ******************************************************************************/
typedef enum parameter_e {
    RECIPIENT_CHAIN,
    RECIPIENT,
    ARBITER_FEE,
    NONCE,
    TOKEN_ADDRESS,
    TOKEN,
    AMOUNT,
    ENCODED_VM,
    PAYLOAD,
    UNEXPECTED_PARAMETER,
} parameter_t;

/******************************************************************************
 * Per-function data structures stored inside context_t.
 ******************************************************************************/
typedef struct handle_wrap_and_transfer_eth_data_s {
    uint16_t recipient_chain;
    uint8_t recipient[32];
    uint8_t arbiter_fee[32];
    uint32_t nonce;
} handle_wrap_and_transfer_eth_data_t;

typedef struct handle_wrap_and_transfer_eth_with_payload_data_s {
    uint16_t recipient_chain;
    uint8_t recipient[32];
    uint32_t nonce;
} handle_wrap_and_transfer_eth_with_payload_data_t;

typedef struct handle_attest_token_data_s {
    uint8_t token_address[32];
    uint32_t nonce;
} handle_attest_token_data_t;

typedef struct handle_transfer_tokens_data_s {
    uint8_t token[32];
    uint8_t amount[32];
    uint16_t recipient_chain;
    uint8_t recipient[32];
    uint8_t arbiter_fee[32];
    uint32_t nonce;
} handle_transfer_tokens_data_t;

typedef struct handle_transfer_tokens_with_payload_data_s {
    uint8_t token[32];
    uint8_t amount[32];
    uint16_t recipient_chain;
    uint8_t recipient[32];
    uint32_t nonce;
} handle_transfer_tokens_with_payload_data_t;

/******************************************************************************
 * context_t — plugin wide context, shared across all handler calls.
 * Uses a union to keep total size compact.
 ******************************************************************************/
typedef struct context_s {
    /* Selector that was matched during handle_init_contract. */
    selector_t selectorIndex;

    /* Which parameter we expect next in handle_provide_parameter. */
    uint8_t next_param;

    /* For parameter copy helpers. */
    uint16_t offset;
    bool go_to_offset;

    /* Union of per-function data (largest member determines total size). */
    union {
        handle_wrap_and_transfer_eth_data_t handle_wrap_and_transfer_eth_data;
        handle_wrap_and_transfer_eth_with_payload_data_t handle_wrap_and_transfer_eth_with_payload_data;
        handle_attest_token_data_t handle_attest_token_data;
        handle_transfer_tokens_data_t handle_transfer_tokens_data;
        handle_transfer_tokens_with_payload_data_t handle_transfer_tokens_with_payload_data;
    } data;
} context_t;

/* Compile-time assertion that the context fits within the plugin arena. */
ASSERT_SIZEOF_PLUGIN_CONTEXT(context_t);
