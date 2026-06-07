# Build: Ledger Plugin for Wormhole Portal Bridge

## What to Build
A Ledger Ethereum plugin (C) that parses and displays Wormhole Portal Token Bridge transactions on the Ledger Nano S/X/Stax device. Users bridging assets through Portal Bridge should see clear, human-readable transfer details on their Ledger screen.

**Contract address (Ethereum mainnet):** `0x3ee18b2214aff97000d974cf647e7c347e8fa585`

## Fork This Boilerplate Structure
Use the exact structure from LedgerHQ/app-plugin-boilerplate:
```
app-plugin-boilerplate/        # root dir
├── src/
│   ├── plugin.c              # SELECTORS array + #include "plugin.h"
│   ├── plugin.h              # SELECTORS_LIST, context_t struct, parameters enum
│   ├── handle_init_contract.c       # selector dispatch + next_param setup
│   ├── handle_provide_parameter.c   # per-selector parameter parsing
│   ├── handle_finalize.c            # no-op (set ETH_PLUGIN_RESULT_OK)
│   ├── handle_provide_token.c       # pass-through
│   ├── handle_query_contract_id.c   # return contract address
│   └── handle_query_contract_ui.c   # per-selector UI screens
├── tests/                           # Ragger functional tests
├── icons/                           # app icon (PNG, 14x14)
├── glyphs/                          # display glyphs
├── fuzzing/                         # fuzz harness
├── .clusterfuzzlite/                # fuzz CI config
├── .github/workflows/               # CI pipelines
├── Makefile                         # build (uses ethereum-plugin-sdk submodule)
├── PLUGIN_SPECIFICATION.md          # mandatory doc
├── README.md
├── ledger_app.toml
├── .clang-format
├── .gitignore
├── .gitmodules
├── LICENSE.md
```

## Supported Functions (SELECTORS)
Use the exact 4-byte selectors from the Wormhole Token Bridge contract:

| Selector name | 4-byte hex | Params to display |
|---|---|---|
| ATTEST_TOKEN | 0xc48fa115 | tokenAddress, nonce |
| WRAP_AND_TRANSFER_ETH | 0x9981509f | recipientChain, recipient, arbiterFee, nonce |
| WRAP_AND_TRANSFER_ETH_WITH_PAYLOAD | 0xbee9cdfc | recipientChain, recipient, arbiterFee, nonce |
| TRANSFER_TOKENS | 0x0f5287b0 | token, amount, recipientChain, recipient, arbiterFee, nonce |
| TRANSFER_TOKENS_WITH_PAYLOAD | 0xc5a5ebda | token, amount, recipientChain, recipient, arbiterFee, nonce |
| COMPLETE_TRANSFER | 0xc6878519 | encodedVM (no display — just confirm) |
| COMPLETE_TRANSFER_WITH_PAYLOAD | 0xc3f511c1 | encodedVM (no display — just confirm) |
| COMPLETE_TRANSFER_AND_UNWRAP_ETH | 0xff200cde | encodedVM (no display — just confirm) |
| COMPLETE_TRANSFER_AND_UNWRAP_ETH_WITH_PAYLOAD | 0x1c8475e4 | encodedVM (no display — just confirm) |
| ATTEST_TOKEN | 0xc48fa115 | tokenAddress, nonce |
| CREATE_WRAPPED | 0xe8059810 | encodedVM (no display) |
| UPDATE_WRAPPED | 0xf768441f | encodedVM (no display) |
| REGISTER_CHAIN | 0xa5799f93 | encodedVM (no display) |
| SUBMIT_RECOVER_CHAIN_ID | 0x178149e7 | encodedVM (no display) |
| UPGRADE | 0x25394645 | encodedVM (no display) |

For the MVP, implement these **display-focused** functions:
1. **WRAP_AND_TRANSFER_ETH** (0x9981509f) — most common user action
2. **TRANSFER_TOKENS** (0x0f5287b0) — most common user action
3. **ATTEST_TOKEN** (0xc48fa115) — token registration
4. **COMPLETE_TRANSFER** (0xc6878519) — complete a transfer (no param parsing, just confirm)

Mark the others as UNEXPECTED_PARAMETER (skip parsing, just confirm the tx).

## file-by-file instructions

### src/plugin.h
- Define SELECTORS_LIST with all 14 selectors from the table above.
- Parameter enum: ENCODED_VM, TOKEN_ADDRESS, NONCE, RECIPIENT_CHAIN, RECIPIENT, ARBITER_FEE, PAYLOAD, TOKEN, AMOUNT, ENCODED_VM, UNEXPECTED_PARAMETER.
- context_t struct with union of:
  - handle_wrap_and_transfer_e_t_h_data: {recipient_chain(uint16_t), recipient(uint8_t[32]), arbiter_fee(uint8_t[32]), nonce(uint32_t)}
  - handle_attest_token_data: {token_address(uint8_t[32]), nonce(uint32_t)}
  - handle_transfer_tokens_data: {token(uint8_t[32]), amount(uint8_t[32]), recipient_chain(uint16_t), recipient(uint8_t[32]), arbiter_fee(uint8_t[32]), nonce(uint32_t)}
- Plus: next_param (uint8_t), offset (uint16_t), go_to_offset (bool), selectorIndex (selector_t).
- ASSERT_SIZEOF_PLUGIN_CONTEXT at bottom.

### src/plugin.c
- Just the SELECTORS array definition: `const uint32_t SELECTORS[SELECTOR_COUNT] = {SELECTORS_LIST(TO_VALUE)};`

### src/handle_init_contract.c
- Check interfaceVersion, pluginContextLength.
- Find selector via find_selector().
- Set context->selectorIndex.
- Switch on selectorIndex to set context->next_param:
  - WRAP_AND_TRANSFER_ETH → RECIPIENT_CHAIN
  - TRANSFER_TOKENS → TOKEN
  - ATTEST_TOKEN → TOKEN_ADDRESS
  - COMPLETE_* and remaining → ENCODED_VM
- Return ETH_PLUGIN_RESULT_OK.

### src/handle_provide_parameter.c
For each display function, parse the parameters in order:

**handle_wrap_and_transfer_eth:**
- RECIPIENT_CHAIN: U2BE from last 2 bytes of msg->parameter
- RECIPIENT: copy_parameter to context->data...recipient (32 bytes)
- ARBITER_FEE: copy_parameter to context->data...arbiter_fee (32 bytes)
- NONCE: U4BE from last 4 bytes of msg->parameter

**handle_attest_token:**
- TOKEN_ADDRESS: copy_parameter to context->data.handle_attest_token_data.token_address (but it stores in recipient field — note the union naming)
- NONCE: U4BE from last 4 bytes

**handle_transfer_tokens:**
- TOKEN: copy_parameter to token field (32 bytes)
- AMOUNT: copy_parameter to amount field (32 bytes)
- RECIPIENT_CHAIN: U2BE from last 2 bytes
- RECIPIENT: copy_parameter (32 bytes)
- ARBITER_FEE: copy_parameter (32 bytes)
- NONCE: U4BE from last 4 bytes

For COMPLETE_* and other non-display selectors, just set next_param = UNEXPECTED_PARAMETER.
Return ETH_PLUGIN_RESULT_OK.

### src/handle_finalize.c
Just return ETH_PLUGIN_RESULT_OK. No additional processing needed.

### src/handle_provide_token.c
Pass-through: set msg->result = ETH_PLUGIN_RESULT_OK.
Optionally fill token_found and ticker from plugin shared memory.

### src/handle_query_contract_id.c
Return the contract address bytes. Use the Wormhole Portal Token Bridge address:
`0x3ee18b2214aff97000d974cf647e7c347e8fa585`

### src/handle_query_contract_ui.c
**Chain ID → name mapping** (critical for user experience):
```c
static const char *const chain_names1[] = {
    "UNUSED", "Solana", "Ethereum", "Terra Classic", "Bsc", "Polygon",
    "Avalanche", "Oasis", "Algorand", "Aurora", "Fantom", "Karura", "Acala",
    "Klaytn", "Celo", "Near", "Moonbeam", "Neon", "Terra2", "Injective",
    "Osmosis", "Sui", "Aptos", "Arbitrum", "Optimism", "Gnosis", "Pythnet",
    "UNUSED", "XPLA", "Bitcoin", "Base", "UNUSED", "Sei", "Rootstock",
    "Scroll", "Mantle"
};
static const char *const chain_names2[] = {
    "Cosmos Hub", "Evmos", "Kujira", "Neutron", "Celestia"
};
static const char *const chain_names3[] = {
    "Sepolia", "Arbitrum Sepolia", "Base Sepolia", "Optimism Sepolia", "Holesky"
};
```
Special: ID 3104 → "Wormhole".
Chain 4000-4004 → chain_names2; 10002-10006 → chain_names3.

**Helper functions (set_):**
- set_recipient_chain(msg, context) → title: "Dst Chain", value: chain_name
- set_recipient(msg, context) → title: "Recipient", value: 0x + hex address (20 bytes from 32-byte field)
- set_arbiter_fee(msg, context) → title: "Arbiter Fee", value: amount in ETH with "ETH" suffix
- set_nonce(msg, context) → title: "Nonce", value: decimal uint32

**Per-selector display:**
- WRAP_AND_TRANSFER_ETH: screen 0=Dst Chain, 1=Recipient, 2=Arbiter Fee, 3=None
- TRANSFER_TOKENS: screen 0=Token, 1=Amount, 2=Dst Chain, 3=Recipient, 4=Arbiter Fee, 5=Nonce
- ATTEST_TOKEN: screen 0=Token Address, 1=Nonce
- COMPLETE_*: just set result OK, no screens

### Makefile
Standard boilerplate Makefile that expects `ethereum-plugin-sdk/` as submodule. Set BOLOS_SDK or ETHEREUM_PLUGIN_SDK path. Has targets: clean, build, all (build for nanos, nanosp, nanox, stax).

### PLUGIN_SPECIFICATION.md
Required Ledger doc listing all supported functions and their parameters. Describe the Wormhole Portal Token Bridge integration.

### ledger_app.toml
Minimal TOML with app metadata.

### tests/
Ragger functional tests (Python with pytest) covering:
- wrap_and_transfer_eth: correct parameter display
- transfer_tokens: correct parameter display
- attest_token: correct parameter display
- complete_transfer: just confirms
- Negative: unknown selector returns unavailable

## Acceptance Criteria
1. Complete Ledger plugin with source + tests + CI config
2. Correct function selectors for all Wormhole Portal Bridge functions
3. Correct parameter parsing for WRAP_AND_TRANSFER_ETH, TRANSFER_TOKENS, ATTEST_TOKEN
4. Chain ID → human-readable name mapping for 30+ chains
5. Ragger test suite covering at least 3 positive flows + 1 negative
6. PLUGIN_SPECIFICATION.md documenting all supported functions
7. README.md with build & test instructions
8. User flow: "Jim bridges assets through Portal Bridge → sees transfer details on Ledger"
