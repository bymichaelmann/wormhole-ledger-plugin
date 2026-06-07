/**
 * fuzz_plugin.c — libFuzzer harness for the Wormhole Portal plugin.
 *
 * This harness fuzzes the parameter parsing logic by feeding arbitrary
 * byte sequences into re-implemented parsing routines.  The goal is to
 * catch memory errors, out-of-bounds accesses, and logic bugs when
 * processing malformed ABI-encoded parameters.
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

/* ------------------------------------------------------------------ */
/*  SDK macro emulation                                                */
/* ------------------------------------------------------------------ */

#define PARAMETER_LENGTH 32

#define U2BE(buf, off) ((uint16_t)(((buf)[off] << 8) | ((buf)[off + 1])))
#define U4BE(buf, off) ((uint32_t)(((buf)[off] << 24) |   \
                                   ((buf)[off + 1] << 16) | \
                                   ((buf)[off + 2] << 8) |  \
                                   ((buf)[off + 3])))

static void copy_parameter(uint8_t *dst, const uint8_t *src, size_t len) {
    memcpy(dst, src, len);
}

/* ------------------------------------------------------------------ */
/*  Minimal data structures matching the plugin's expectations         */
/* ------------------------------------------------------------------ */

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

typedef enum selector_e {
    SEL_WRAP_AND_TRANSFER_ETH = 0,
    SEL_WRAP_AND_TRANSFER_ETH_WITH_PAYLOAD,
    SEL_TRANSFER_TOKENS,
    SEL_TRANSFER_TOKENS_WITH_PAYLOAD,
    SEL_ATTEST_TOKEN,
    SEL_COMPLETE_TRANSFER,
    /* remaining selectors not needed for fuzzing */
} selector_t;

typedef struct {
    uint8_t *parameter;
    size_t   parameterOffset;
    uint8_t *pluginContext;
    size_t   pluginContextLength;
    int      result;
} ethPluginProvideParameter_t;

/* Context struct mirroring the plugin's context_t (simplified). */
typedef struct {
    selector_t selectorIndex;
    uint8_t    next_param;
    uint16_t   offset;
    int        go_to_offset;   /* bool in C — use int for ABI compat */
    union {
        struct {
            uint16_t recipient_chain;
            uint8_t  recipient[32];
            uint8_t  arbiter_fee[32];
            uint32_t nonce;
        } wrap;
        struct {
            uint8_t  token_address[32];
            uint32_t nonce;
        } attest;
        struct {
            uint8_t  token[32];
            uint8_t  amount[32];
            uint16_t recipient_chain;
            uint8_t  recipient[32];
            uint8_t  arbiter_fee[32];
            uint32_t nonce;
        } transfer;
    };
} context_t;

/* ------------------------------------------------------------------ */
/*  Inline the parsing logic (mirrors handle_provide_parameter.c)      */
/* ------------------------------------------------------------------ */

static void handle_wrap_and_transfer_eth(ethPluginProvideParameter_t *msg,
                                         context_t *context) {
    switch (context->next_param) {
        case RECIPIENT_CHAIN:
            context->wrap.recipient_chain =
                U2BE(msg->parameter, PARAMETER_LENGTH - 2);
            context->next_param = RECIPIENT;
            break;
        case RECIPIENT:
            copy_parameter(context->wrap.recipient,
                           msg->parameter, PARAMETER_LENGTH);
            context->next_param = ARBITER_FEE;
            break;
        case ARBITER_FEE:
            copy_parameter(context->wrap.arbiter_fee,
                           msg->parameter, PARAMETER_LENGTH);
            context->next_param = NONCE;
            break;
        case NONCE:
            context->wrap.nonce =
                U4BE(msg->parameter, PARAMETER_LENGTH - 4);
            context->next_param = UNEXPECTED_PARAMETER;
            break;
        default:
            break;
    }
}

static void handle_transfer_tokens(ethPluginProvideParameter_t *msg,
                                   context_t *context) {
    switch (context->next_param) {
        case TOKEN:
            copy_parameter(context->transfer.token,
                           msg->parameter, PARAMETER_LENGTH);
            context->next_param = AMOUNT;
            break;
        case AMOUNT:
            copy_parameter(context->transfer.amount,
                           msg->parameter, PARAMETER_LENGTH);
            context->next_param = RECIPIENT_CHAIN;
            break;
        case RECIPIENT_CHAIN:
            context->transfer.recipient_chain =
                U2BE(msg->parameter, PARAMETER_LENGTH - 2);
            context->next_param = RECIPIENT;
            break;
        case RECIPIENT:
            copy_parameter(context->transfer.recipient,
                           msg->parameter, PARAMETER_LENGTH);
            context->next_param = ARBITER_FEE;
            break;
        case ARBITER_FEE:
            copy_parameter(context->transfer.arbiter_fee,
                           msg->parameter, PARAMETER_LENGTH);
            context->next_param = NONCE;
            break;
        case NONCE:
            context->transfer.nonce =
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
            copy_parameter(context->attest.token_address,
                           msg->parameter, PARAMETER_LENGTH);
            context->next_param = NONCE;
            break;
        case NONCE:
            context->attest.nonce =
                U4BE(msg->parameter, PARAMETER_LENGTH - 4);
            context->next_param = UNEXPECTED_PARAMETER;
            break;
        default:
            break;
    }
}

static void handle_provide_parameter_fuzz(ethPluginProvideParameter_t *msg,
                                          context_t *context) {
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
            context->next_param = UNEXPECTED_PARAMETER;
            break;
    }
}

/* ------------------------------------------------------------------ */
/*  Fuzz entry point                                                   */
/* ------------------------------------------------------------------ */

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    /* Need at least 1 byte to pick a selector + 1 byte of data. */
    if (size < 2) return 0;

    /* Use first byte to select a function to fuzz. */
    int sel_idx = data[0] % 4;

    /* Allocate and zero context. */
    context_t *ctx = (context_t *) calloc(1, sizeof(context_t));
    if (!ctx) return 0;

    /* Parameter data.  We use up to 32 bytes (one ABI word). */
    size_t param_len = size - 1;
    if (param_len > PARAMETER_LENGTH) param_len = PARAMETER_LENGTH;

    /* Wire up the message. */
    ethPluginProvideParameter_t msg;
    msg.parameter = (uint8_t *) (data + 1);
    msg.parameterOffset = 0;
    msg.pluginContext = (uint8_t *) ctx;
    msg.pluginContextLength = sizeof(context_t);

    /* Set selector. */
    switch (sel_idx) {
        case 0:
            ctx->selectorIndex = SEL_WRAP_AND_TRANSFER_ETH;
            ctx->next_param = RECIPIENT_CHAIN;
            break;
        case 1:
            ctx->selectorIndex = SEL_TRANSFER_TOKENS;
            ctx->next_param = TOKEN;
            break;
        case 2:
            ctx->selectorIndex = SEL_ATTEST_TOKEN;
            ctx->next_param = TOKEN_ADDRESS;
            break;
        case 3:
            ctx->selectorIndex = SEL_COMPLETE_TRANSFER;
            ctx->next_param = ENCODED_VM;
            break;
        default:
            break;
    }

    /* Feed exactly one parameter word to the handler. */
    handle_provide_parameter_fuzz(&msg, ctx);

    free(ctx);
    return 0;
}
