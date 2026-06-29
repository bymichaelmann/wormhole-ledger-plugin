"""
Ragger functional tests for the Wormhole Portal Token Bridge Ledger plugin.

Tests cover all display functions:
  - WRAP_AND_TRANSFER_ETH:        4 UI screens (Dst Chain, Recipient, Arbiter Fee, Nonce)
  - WRAP_AND_TRANSFER_ETH_WITH_PAYLOAD: 3 UI screens (Dst Chain, Recipient, Nonce — NO Arbiter Fee)
  - TRANSFER_TOKENS:              6 UI screens (Token, Amount, Dst Chain, Recipient, Arbiter Fee, Nonce)
  - TRANSFER_TOKENS_WITH_PAYLOAD: 5 UI screens (Token, Amount, Dst Chain, Recipient, Nonce — NO Arbiter Fee)
  - ATTEST_TOKEN:                 2 UI screens (Token Address, Nonce)
  - COMPLETE_TRANSFER:            No parameter screens (just confirm)
  - Negative: unknown selector returns unavailable

All tests run against speculos emulator; no real device required.
"""

import struct

import pytest

from ragger.navigator import NavInsID

# ---------------------------------------------------------------------------
# Helper constants and utilities
# ---------------------------------------------------------------------------

# Wormhole Portal Token Bridge contract address (Ethereum mainnet)
CONTRACT_ADDRESS = bytes.fromhex("3ee18b2214aff97000d974cf647e7c347e8fa585")

# Selectors (4 bytes each)
SELECTORS = {
    "WRAP_AND_TRANSFER_ETH": bytes.fromhex("9981509f"),
    "WRAP_AND_TRANSFER_ETH_WITH_PAYLOAD": bytes.fromhex("bee9cdfc"),
    "TRANSFER_TOKENS": bytes.fromhex("0f5287b0"),
    "TRANSFER_TOKENS_WITH_PAYLOAD": bytes.fromhex("c5a5ebda"),
    "ATTEST_TOKEN": bytes.fromhex("c48fa115"),
    "COMPLETE_TRANSFER": bytes.fromhex("c6878519"),
}

# Ethereum plugin APDU instruction codes
ETH_INS_INIT_CONTRACT = 0x0c
ETH_INS_PROVIDE_PARAMETER = 0x0e
ETH_INS_FINALIZE = 0x12
ETH_INS_QUERY_CONTRACT_ID = 0x16
ETH_INS_QUERY_CONTRACT_UI = 0x18

CLA = 0xe0


def send_apdu(client, cla, ins, p1, p2, data=b""):
    """Send an APDU and return response."""
    apdu_bytes = bytes([cla, ins, p1, p2, len(data)]) + data
    return client.apdu_exchange(apdu_bytes)


def encode_eth_address(addr_hex: str) -> bytes:
    """Encode an Ethereum address as a left-padded 32-byte word."""
    raw = bytes.fromhex(addr_hex.replace("0x", ""))
    assert len(raw) <= 20
    return raw.rjust(32, b"\x00")


def encode_u256(value: int) -> bytes:
    """Encode a uint256 as a 32-byte big-endian word."""
    return value.to_bytes(32, byteorder="big")


def encode_u16(value: int) -> bytes:
    """Encode a uint16 right-aligned in a 32-byte word."""
    return b"\x00" * 30 + value.to_bytes(2, byteorder="big")


def encode_u32(value: int) -> bytes:
    """Encode a uint32 right-aligned in a 32-byte word."""
    return b"\x00" * 28 + value.to_bytes(4, byteorder="big")


def encode_bytes32(hex_str: str) -> bytes:
    """Encode a 32-byte (64 hex chars) value."""
    return bytes.fromhex(hex_str.replace("0x", ""))


def run_plugin_flow(client, selector, params, expected_screens):
    """
    Simulate the Ledger Ethereum plugin call flow:
      1. INIT_CONTRACT
      2. PROVIDE_PARAMETER for each param
      3. FINALIZE
      4. QUERY_CONTRACT_ID
      5. QUERY_CONTRACT_UI for each screen
    """
    # Step 1: Init contract
    init_data = bytes([0x00, 0x01])  # interfaceVersion = 1
    init_data += struct.pack(">H", len(CONTRACT_ADDRESS))
    init_data += CONTRACT_ADDRESS
    init_data += selector
    resp = send_apdu(client, CLA, ETH_INS_INIT_CONTRACT, 0x00, 0x00, init_data)
    assert resp == b"\x90\x00", f"INIT_CONTRACT failed: {resp.hex()}"

    # Step 2: Provide parameters
    for param in params:
        resp = send_apdu(client, CLA, ETH_INS_PROVIDE_PARAMETER, 0x00, 0x00, param)
        assert resp == b"\x90\x00", f"PROVIDE_PARAMETER failed: {resp.hex()}"

    # Step 3: Finalize
    resp = send_apdu(client, CLA, ETH_INS_FINALIZE, 0x00, 0x00)
    assert resp == b"\x90\x00", f"FINALIZE failed: {resp.hex()}"

    # Step 4: Query contract ID
    resp = send_apdu(client, CLA, ETH_INS_QUERY_CONTRACT_ID, 0x00, 0x00)
    assert resp == b"\x90\x00", f"QUERY_CONTRACT_ID failed: {resp.hex()}"

    # Step 5: Query contract UI for each expected screen
    for i, (expected_title, expected_msg_substr) in enumerate(expected_screens):
        resp = send_apdu(client, CLA, ETH_INS_QUERY_CONTRACT_UI, 0x00, i)
        assert resp[:2] == b"\x90\x00", f"QUERY_CONTRACT_UI screen {i} failed: {resp.hex()}"

        if len(resp) > 2:
            payload = resp[:-2]
            assert expected_title.encode() in payload, \
                f"Screen {i}: expected title '{expected_title}' not in {payload!r}"
            if expected_msg_substr:
                assert expected_msg_substr.encode() in payload, \
                    f"Screen {i}: expected msg '{expected_msg_substr}' not in {payload!r}"

    # Step 6: Verify no extra screens
    extra = send_apdu(client, CLA, ETH_INS_QUERY_CONTRACT_UI, 0x00, len(expected_screens))
    # Should return an error or a generic confirm screen
    assert extra[:2] != b"\x90\x00", \
        f"Unexpected extra screen at index {len(expected_screens)}: {extra.hex()}"


# ---------------------------------------------------------------------------
# WRAP_AND_TRANSFER_ETH tests
# ---------------------------------------------------------------------------

WRAP_PARAMS = [
    encode_u16(1),                                                         # recipientChain = 1 (Solana)
    encode_bytes32("abcd1234abcd1234abcd1234abcd1234abcd1234abcd1234abcd1234abcd1234"),  # recipient
    encode_u256(1),                                                        # arbiterFee = 1 wei
    encode_u32(42),                                                        # nonce = 42
]

WRAP_EXPECTED = [
    ("Dst Chain", "Solana"),
    ("Recipient", "0x"),
    ("Arbiter Fee", "ETH"),
    ("Nonce", "42"),
]


def test_wrap_and_transfer_eth(client):
    """WRAP_AND_TRANSFER_ETH displays Dst Chain, Recipient, Arbiter Fee, Nonce."""
    run_plugin_flow(client, SELECTORS["WRAP_AND_TRANSFER_ETH"], WRAP_PARAMS, WRAP_EXPECTED)


# ---------------------------------------------------------------------------
# WRAP_AND_TRANSFER_ETH_WITH_PAYLOAD tests (NO Arbiter Fee)
# ---------------------------------------------------------------------------

WRAP_PAYLOAD_PARAMS = [
    encode_u16(21),                                                        # recipientChain = 21 (Sui)
    encode_bytes32("deadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeef"),  # recipient (32 bytes)
    encode_u32(7),                                                         # nonce = 7
    # payload bytes: dynamic (offset, length, data) — we send minimal
    encode_u256(32),                                                       # offset to payload
    encode_u256(4),                                                        # length = 4
    encode_bytes32("00000000000000000000000000000000000000000000000000000000ca1ebabe"),  # actual payload data
]

WRAP_PAYLOAD_EXPECTED = [
    ("Dst Chain", "Sui"),
    ("Recipient", "0x"),
    ("Nonce", "7"),
]


def test_wrap_and_transfer_eth_with_payload(client):
    """WRAP_AND_TRANSFER_ETH_WITH_PAYLOAD shows 3 screens: Dst Chain, Recipient, Nonce (NO Arbiter Fee)."""
    run_plugin_flow(client, SELECTORS["WRAP_AND_TRANSFER_ETH_WITH_PAYLOAD"],
                    WRAP_PAYLOAD_PARAMS, WRAP_PAYLOAD_EXPECTED)


# ---------------------------------------------------------------------------
# TRANSFER_TOKENS tests
# ---------------------------------------------------------------------------

TRANSFER_PARAMS = [
    encode_eth_address("A0b86991c6218b36c1d19D4a2e9Eb0cE3606eB48"),  # token = USDC
    encode_u256(1000000),                                          # amount = 1000000
    encode_u16(4),                                                 # recipientChain = 4 (Bsc)
    encode_bytes32("abcd1234abcd1234abcd1234abcd1234abcd1234abcd1234abcd1234abcd1234"),  # recipient
    encode_u256(0),                                                # arbiterFee = 0
    encode_u32(99),                                                # nonce = 99
]

TRANSFER_EXPECTED = [
    ("Token", "0x"),
    ("Amount", "tokens"),
    ("Dst Chain", "Bsc"),
    ("Recipient", "0x"),
    ("Arbiter Fee", "ETH"),
    ("Nonce", "99"),
]


def test_transfer_tokens(client):
    """TRANSFER_TOKENS displays Token, Amount, Dst Chain, Recipient, Arbiter Fee, Nonce."""
    run_plugin_flow(client, SELECTORS["TRANSFER_TOKENS"], TRANSFER_PARAMS, TRANSFER_EXPECTED)


# ---------------------------------------------------------------------------
# TRANSFER_TOKENS_WITH_PAYLOAD tests (NO Arbiter Fee)
# ---------------------------------------------------------------------------

TRANSFER_PAYLOAD_PARAMS = [
    encode_eth_address("A0b86991c6218b36c1d19D4a2e9Eb0cE3606eB48"),  # token = USDC
    encode_u256(5000000),                                          # amount = 5000000
    encode_u16(22),                                                # recipientChain = 22 (Aptos)
    encode_bytes32("deadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeef"),  # recipient
    encode_u32(123),                                               # nonce = 123
    # payload bytes: dynamic
    encode_u256(32),
    encode_u256(8),
    encode_bytes32("00000000000000000000000000000000000000000000000000000000cafebabe"),
]

TRANSFER_PAYLOAD_EXPECTED = [
    ("Token", "0x"),
    ("Amount", "tokens"),
    ("Dst Chain", "Aptos"),
    ("Recipient", "0x"),
    ("Nonce", "123"),
]


def test_transfer_tokens_with_payload(client):
    """TRANSFER_TOKENS_WITH_PAYLOAD shows 5 screens: Token, Amount, Dst Chain, Recipient, Nonce (NO Arbiter Fee)."""
    run_plugin_flow(client, SELECTORS["TRANSFER_TOKENS_WITH_PAYLOAD"],
                    TRANSFER_PAYLOAD_PARAMS, TRANSFER_PAYLOAD_EXPECTED)


# ---------------------------------------------------------------------------
# ATTEST_TOKEN tests
# ---------------------------------------------------------------------------

ATTEST_PARAMS = [
    encode_eth_address("7Fc66500c84A76Ad7e9c93437bFc5Ac33E2DDaE9"),  # tokenAddress = AAVE
    encode_u32(7),                                                  # nonce = 7
]

ATTEST_EXPECTED = [
    ("Token Address", "0x"),
    ("Nonce", "7"),
]


def test_attest_token(client):
    """ATTEST_TOKEN displays Token Address and Nonce."""
    run_plugin_flow(client, SELECTORS["ATTEST_TOKEN"], ATTEST_PARAMS, ATTEST_EXPECTED)


# ---------------------------------------------------------------------------
# COMPLETE_TRANSFER tests (no param screens, just confirm)
# ---------------------------------------------------------------------------

COMPLETE_PARAMS = [
    b"\x00" * 32,  # minimal placeholder for encodedVM
]


def test_complete_transfer(client):
    """
    COMPLETE_TRANSFER does not display parameter screens.
    The transaction just shows a generic confirm.
    """
    run_plugin_flow(client, SELECTORS["COMPLETE_TRANSFER"], COMPLETE_PARAMS, [])


# ---------------------------------------------------------------------------
# Negative test: unknown selector
# ---------------------------------------------------------------------------

def test_unknown_selector(client):
    """Unknown selector should return UNAVAILABLE."""
    unknown_selector = bytes.fromhex("deadbeef")
    init_data = bytes([0x00, 0x01])  # interfaceVersion = 1
    init_data += struct.pack(">H", len(CONTRACT_ADDRESS))
    init_data += CONTRACT_ADDRESS
    init_data += unknown_selector
    resp = send_apdu(client, CLA, ETH_INS_INIT_CONTRACT, 0x00, 0x00, init_data)
    # Should return something other than 0x9000 (unavailable/error)
    assert resp != b"\x90\x00", f"Unknown selector should not succeed: {resp.hex()}"


# ---------------------------------------------------------------------------
# Negative test: extra parameters do not crash
# ---------------------------------------------------------------------------

def test_attest_token_extra_params(client):
    """ATTEST_TOKEN with extra parameters should not crash (extra ignored)."""
    params = ATTEST_PARAMS + [encode_u32(999)]
    run_plugin_flow(client, SELECTORS["ATTEST_TOKEN"], params, ATTEST_EXPECTED)


# ---------------------------------------------------------------------------
# Recipient display: full 32 bytes for non-EVM destination
# ---------------------------------------------------------------------------

FULL_RECIPIENT = "deadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeef"

WRAP_SOLANA_PARAMS = [
    encode_u16(1),               # recipientChain = 1 (Solana — non-EVM)
    encode_bytes32(FULL_RECIPIENT),
    encode_u256(0),
    encode_u32(1),
]


def test_recipient_full_32_bytes_solana(client):
    """Non-EVM destination (Solana) shows full 32-byte recipient."""
    resp = send_apdu(client, CLA, ETH_INS_INIT_CONTRACT, 0x00, 0x00,
                     bytes([0x00, 0x01]) + struct.pack(">H", len(CONTRACT_ADDRESS))
                     + CONTRACT_ADDRESS + SELECTORS["WRAP_AND_TRANSFER_ETH"])
    assert resp == b"\x90\x00"

    for param in WRAP_SOLANA_PARAMS:
        resp = send_apdu(client, CLA, ETH_INS_PROVIDE_PARAMETER, 0x00, 0x00, param)
        assert resp == b"\x90\x00"

    resp = send_apdu(client, CLA, ETH_INS_FINALIZE, 0x00, 0x00)
    assert resp == b"\x90\x00"

    resp = send_apdu(client, CLA, ETH_INS_QUERY_CONTRACT_ID, 0x00, 0x00)
    assert resp == b"\x90\x00"

    # Screen 1 = Recipient — should contain the full 32 bytes (64 hex chars after 0x)
    resp = send_apdu(client, CLA, ETH_INS_QUERY_CONTRACT_UI, 0x00, 1)
    assert resp[:2] == b"\x90\x00"
    payload = resp[:-2]
    # The full recipient should be visible in the payload
    assert FULL_RECIPIENT.encode() in payload, \
        f"Full 32-byte recipient '{FULL_RECIPIENT}' not found in {payload!r}"
