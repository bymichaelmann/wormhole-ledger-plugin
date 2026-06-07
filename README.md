# Wormhole Portal Token Bridge — Ledger Plugin

A [Ledger Ethereum plugin](https://github.com/LedgerHQ/ethereum-plugin-sdk) for the
**Wormhole Portal Token Bridge** contract. When a user signs a Portal Bridge
transaction on their Ledger device, this plugin decodes the function parameters
and displays them as clear, human-readable screens — enabling safe verification
before approval.

## Supported Devices

- Ledger Nano S
- Ledger Nano S Plus
- Ledger Nano X
- Ledger Stax

## Contract

| Network | Address |
|---------|---------|
| Ethereum Mainnet | [`0x3ee18b2214aff97000d974cf647e7c347e8fa585`](https://etherscan.io/address/0x3ee18b2214aff97000d974cf647e7c347e8fa585) |

## Supported Functions

| Function | Selector | Screens |
|----------|----------|---------|
| `WRAP_AND_TRANSFER_ETH` | `0x9981509f` | 4 (Dst Chain, Recipient, Arbiter Fee, Nonce) |
| `WRAP_AND_TRANSFER_ETH_WITH_PAYLOAD` | `0xbee9cdfc` | 4 |
| `TRANSFER_TOKENS` | `0x0f5287b0` | 6 (Token, Amount, Dst Chain, Recipient, Arbiter Fee, Nonce) |
| `TRANSFER_TOKENS_WITH_PAYLOAD` | `0xc5a5ebda` | 6 |
| `ATTEST_TOKEN` | `0xc48fa115` | 2 (Token Address, Nonce) |
| `COMPLETE_TRANSFER` | `0xc6878519` | 0 (confirm only) |
| *(and 8 more admin functions)* | | confirm only |

See [PLUGIN_SPECIFICATION.md](PLUGIN_SPECIFICATION.md) for the full list.

## Building

### Prerequisites

- [Ledger Ethereum Plugin SDK](https://github.com/LedgerHQ/ethereum-plugin-sdk)
- `clang` / `gcc` (for the SDK)
- GNU Make

### Setup

```bash
# Clone the SDK submodule
git submodule update --init

# Or point to a local SDK checkout
export ETHEREUM_PLUGIN_SDK=/path/to/ethereum-plugin-sdk
```

### Build

```bash
make all
```

This builds for all devices (nanos, nanosp, nanox, stax). Output goes to
`build/` or per-device directories.

### Build for a single device

```bash
make nanos
# or: make nanox
# or: make stax
```

## Testing

### Unit tests (via Ragger)

```bash
cd tests
pip install -r requirements.txt
pytest -v
```

### Fuzzing

```bash
cd fuzzing
make
./run_fuzzer
```

## Project Structure

```
.
├── src/                       # Plugin C source
│   ├── plugin.h               # Selectors, enums, context struct
│   ├── plugin.c               # SELECTORS array
│   ├── handle_init_contract.c          # Selector dispatch
│   ├── handle_provide_parameter.c      # Parameter parsing
│   ├── handle_finalize.c               # Finalization (no-op)
│   ├── handle_provide_token.c          # Token info pass-through
│   ├── handle_query_contract_id.c      # Contract identity
│   └── handle_query_contract_ui.c      # UI screen rendering
├── tests/                     # Ragger functional tests
├── icons/                     # Device app icon
├── glyphs/                    # Display glyphs
├── fuzzing/                   # Fuzz harness
├── .github/workflows/         # CI pipelines
├── Makefile
├── PLUGIN_SPECIFICATION.md
├── README.md
└── ledger_app.toml
```

## User Flow

1. User connects Ledger and opens Ethereum app
2. User visits [Portal Bridge](https://portalbridge.com) web interface
3. User initiates a bridge transfer (e.g., ETH → Solana)
4. Ledger shows:
   - **"Wormhole Portal"** as the plugin name
   - Destination chain (human-readable name)
   - Recipient address
   - Arbiter fee
   - Nonce
5. User verifies details and approves on-device

## License

Apache 2.0 — see [LICENSE.md](LICENSE.md)
