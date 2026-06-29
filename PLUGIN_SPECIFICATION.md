# Wormhole Portal Token Bridge — Ledger Plugin Specification

## Overview

This Ledger Ethereum plugin parses and displays **Wormhole Portal Token Bridge** transactions
on Ledger Nano S, Nano S Plus, Nano X, and Stax devices. When a user interacts with the
Portal Bridge contract (`0x3ee18b2214aff97000d974cf647e7c347e8fa585`), the plugin presents
human-readable transfer details such as destination chain, recipient address, amount, arbiter
fee, and nonce — allowing users to verify and approve transactions with confidence.

## Supported Functions

### 1. WRAP_AND_TRANSFER_ETH

| Field | Selector |
|-------|----------|
| **Selector** | `0x9981509f` |
| **Signature** | `wrapAndTransferEth(uint16 recipientChain, bytes32 recipient, bytes32 arbiterFee, uint32 nonce)` |
| **UI Screens** | 4 |

| Screen | Title | Value |
|--------|-------|-------|
| 0 | Dst Chain | Human-readable chain name (e.g. "Solana", "Ethereum", "Polygon") |
| 1 | Recipient | `0x`-prefixed hex address (20 bytes) |
| 2 | Arbiter Fee | Amount in ETH with "ETH" suffix |
| 3 | Nonce | Decimal uint32 |

### 2. WRAP_AND_TRANSFER_ETH_WITH_PAYLOAD

| Field | Selector |
|-------|----------|
| **Selector** | `0xbee9cdfc` |
| **Signature** | `wrapAndTransferETHWithPayload(uint16 recipientChain, bytes32 recipient, uint32 nonce, bytes payload)` |
| **UI Screens** | 3 (no Arbiter Fee) |

| Screen | Title | Value |
|--------|-------|-------|
| 0 | Dst Chain | Human-readable chain name (e.g. "Solana", "Ethereum", "Polygon") |
| 1 | Recipient | `0x`-prefixed hex address |
| 2 | Nonce | Decimal uint32 |

Note: This variant has no `arbiterFee` parameter. The `payload` is a dynamic bytes parameter that follows the nonce; it is not displayed on the device.

### 3. TRANSFER_TOKENS

| Field | Selector |
|-------|----------|
| **Selector** | `0x0f5287b0` |
| **Signature** | `transferTokens(address token, uint256 amount, uint16 recipientChain, bytes32 recipient, bytes32 arbiterFee, uint32 nonce)` |
| **UI Screens** | 6 |

| Screen | Title | Value |
|--------|-------|-------|
| 0 | Token | `0x`-prefixed token contract address |
| 1 | Amount | Raw token amount |
| 2 | Dst Chain | Human-readable chain name |
| 3 | Recipient | `0x`-prefixed hex address |
| 4 | Arbiter Fee | Arbitrary fee (in ETH) |
| 5 | Nonce | Decimal uint32 |

### 4. TRANSFER_TOKENS_WITH_PAYLOAD

| Field | Selector |
|-------|----------|
| **Selector** | `0xc5a5ebda` |
| **Signature** | `transferTokensWithPayload(address token, uint256 amount, uint16 recipientChain, bytes32 recipient, uint32 nonce, bytes payload)` |
| **UI Screens** | 5 (no Arbiter Fee) |

| Screen | Title | Value |
|--------|-------|-------|
| 0 | Token | `0x`-prefixed token contract address |
| 1 | Amount | Raw token amount |
| 2 | Dst Chain | Human-readable chain name |
| 3 | Recipient | `0x`-prefixed hex address |
| 4 | Nonce | Decimal uint32 |

Note: This variant has no `arbiterFee` parameter. The `payload` is a dynamic bytes parameter; it is not displayed on the device.

### 5. ATTEST_TOKEN

| Field | Selector |
|-------|----------|
| **Selector** | `0xc48fa115` |
| **Signature** | `attestToken(address tokenAddress, uint32 nonce)` |
| **UI Screens** | 2 |

| Screen | Title | Value |
|--------|-------|-------|
| 0 | Token Address | `0x`-prefixed token address |
| 1 | Nonce | Decimal uint32 |

### 6. COMPLETE_TRANSFER

| Field | Selector |
|-------|----------|
| **Selector** | `0xc6878519` |
| **Signature** | `completeTransfer(bytes encodedVM)` |
| **UI Screens** | 0 (just confirm) |

No parameter parsing; the user only sees a confirmation screen.

### 7. COMPLETE_TRANSFER_WITH_PAYLOAD

| Field | Selector |
|-------|----------|
| **Selector** | `0xc3f511c1` |
| **Signature** | `completeTransferWithPayload(bytes encodedVM)` |
| **UI Screens** | 0 (just confirm) |

### 8. COMPLETE_TRANSFER_AND_UNWRAP_ETH

| Field | Selector |
|-------|----------|
| **Selector** | `0xff200cde` |
| **Signature** | `completeTransferAndUnwrapETH(bytes encodedVM)` |
| **UI Screens** | 0 (just confirm) |

### 9. COMPLETE_TRANSFER_AND_UNWRAP_ETH_WITH_PAYLOAD

| Field | Selector |
|-------|----------|
| **Selector** | `0x1c8475e4` |
| **Signature** | `completeTransferAndUnwrapETHWithPayload(bytes encodedVM)` |
| **UI Screens** | 0 (just confirm) |

### 10. CREATE_WRAPPED

| Field | Selector |
|-------|----------|
| **Selector** | `0xe8059810` |
| **Signature** | `createWrapped(bytes encodedVM)` |
| **UI Screens** | 0 (just confirm) |

### 11. UPDATE_WRAPPED

| Field | Selector |
|-------|----------|
| **Selector** | `0xf768441f` |
| **Signature** | `updateWrapped(bytes encodedVM)` |
| **UI Screens** | 0 (just confirm) |

### 12. REGISTER_CHAIN

| Field | Selector |
|-------|----------|
| **Selector** | `0xa5799f93` |
| **Signature** | `registerChain(bytes encodedVM)` |
| **UI Screens** | 0 (just confirm) |

### 13. SUBMIT_RECOVER_CHAIN_ID

| Field | Selector |
|-------|----------|
| **Selector** | `0x178149e7` |
| **Signature** | `submitRecoverChainId(bytes encodedVM)` |
| **UI Screens** | 0 (just confirm) |

### 14. UPGRADE

| Field | Selector |
|-------|----------|
| **Selector** | `0x25394645` |
| **Signature** | `upgrade(bytes encodedVM)` |
| **UI Screens** | 0 (just confirm) |

## Chain ID Resolution

The plugin maps Wormhole chain IDs to human-readable names:

| ID Range | Names |
|----------|-------|
| 1–35 | Solana, Ethereum, Terra Classic, Bsc, Polygon, Avalanche, Oasis, Algorand, Aurora, Fantom, Karura, Acala, Klaytn, Celo, Near, Moonbeam, Neon, Terra2, Injective, Osmosis, Sui, Aptos, Arbitrum, Optimism, Gnosis, Pythnet, XPLA, Bitcoin, Base, Sei, Rootstock, Scroll, Mantle |
| 3104 | Wormhole |
| 4000–4004 | Cosmos Hub, Evmos, Kujira, Neutron, Celestia |
| 10002–10006 | Sepolia, Arbitrum Sepolia, Base Sepolia, Optimism Sepolia, Holesky |

## Security Considerations

- The plugin does **not** parse `encodedVM` bytes — these are large blobs that
  cannot be meaningfully displayed on a Ledger screen. The user is asked to
  confirm the transaction based on context (the Portal Bridge UI shows full
  details off-device).
- Arbiter fee (when present) and amount are displayed with their respective units.
- For non-payload variants, arbiter fee is shown in ETH with 18 decimal precision.
- WITH_PAYLOAD variants do not have an arbiter fee parameter.
- All addresses are displayed as `0x`-prefixed hex strings. For EVM-compatible
  chains, the last 20 bytes are shown; for non-EVM chains (Solana, Sui, Aptos,
  etc.), the full 32 bytes are displayed.

## Dependencies

- [ethereum-plugin-sdk](https://github.com/LedgerHQ/ethereum-plugin-sdk)
- [app-plugin-boilerplate](https://github.com/LedgerHQ/app-plugin-boilerplate)
