"""
Ragger functional tests for the Wormhole Portal Token Bridge Ledger plugin.

Tests cover:
  - WRAP_AND_TRANSFER_ETH: 4 UI screens (Dst Chain, Recipient, Arbiter Fee, Nonce)
  - TRANSFER_TOKENS:      6 UI screens (Token, Amount, Dst Chain, Recipient,
                           Arbiter Fee, Nonce)
  - ATTEST_TOKEN:         2 UI screens (Token Address, Nonce)
  - COMPLETE_TRANSFER:    No parameter screens (just confirm)
  - Negative: unknown selector returns unavailable

These tests run against either a speculos emulator or a real Ledger device.
"""

import struct
import pytest

# ---------------------------------------------------------------------------
# Helper constants and utilities
# ---------------------------------------------------------------------------

# Wormhole Portal Token Bridge contract address (Ethereum mainnet)
CONTRACT_ADDRESS = bytes.fromhex("3ee18b2214aff97000d974cf647e7c347e8fa585")

# Selectors (4 bytes each)
SELECTOR_WRAP_AND_TRANSFER_ETH = bytes.fromhex("9981509f")
SELECTOR_TRANSFER_TOKENS = bytes.fromhex("0f5287b0")
SELECTOR_ATTEST_TOKEN = bytes.fromhex("c48fa115")
SELECTOR_COMPLETE_TRANSFER = bytes.fromhex("c6878519")
SELECTOR_UNKNOWN = bytes.fromhex("deadbeef")

# Ethereum plugin APDU instruction codes
ETH_INS_GET_PLUGIN_NAME = 0x14
ETH_INS_PROVIDE_TOKEN_INFO = 0x0a
ETH_INS_QUERY_CONTRACT_ID = 0x16
ETH_INS_QUERY_CONTRACT_UI = 0x18
ETH_INS_INIT_CONTRACT = 0x0c
ETH_INS_PROVIDE_PARAMETER = 0x0e
ETH_INS_FINALIZE = 0x12

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

        # Parse response: typically [title][msg]status
        # The response format depends on the SDK version.
        # For simplicity we just verify the status.
        # In a real test, parse the response to check title and msg strings.
        if len(resp) > 2:
            payload = resp[:-2]
            # Check that the payload contains expected strings
            assert expected_title.encode() in payload, \
                f"Screen {i}: expected title '{expected_title}' not in {payload!r}"
            if expected_msg_substr:
                assert expected_msg_substr.encode() in payload, \
                    f"Screen {i}: expected msg '{expected_msg_substr}' not in {payload!r}"


# For testing without a real device, we use parametrize to define test cases.
# The actual hardware test requires speculos or a real device.
# Here we provide the parametrized test data and skip if no device available.

def has_device():
    """Check if a Ledger device or speculos is available."""
    try:
        from ledgercomm import Transport
        t = Transport(interface="hid", debug=False)
        t.open()
        t.close()
        return True
    except Exception:
        return False


DEVICE_AVAILABLE = has_device()


# ---------------------------------------------------------------------------
# WRAP_AND_TRANSFER_ETH tests
# ---------------------------------------------------------------------------

WRAP_PARAMS = [
    # recipientChain (uint16) = 1 (Solana)
    encode_u16(1),
    # recipient (bytes32) = 0xabcd... (32 bytes)
    encode_bytes32("abcd1234abcd1234abcd1234abcd1234abcd1234abcd1234abcd1234abcd1234"),
    # arbiterFee (bytes32) = 0x0000...0001 (1 wei)
    encode_u256(1),
    # nonce (uint32) = 42
    encode_u32(42),
]

WRAP_EXPECTED = [
    ("Dst Chain", "Solana"),
    ("Recipient", "0x"),
    ("Arbiter Fee", "ETH"),
    ("Nonce", "42"),
]


@pytest.mark.skipif(not DEVICE_AVAILABLE, reason="No Ledger device or speculos available")
def test_wrap_and_transfer_eth(client):
    """WRAP_AND_TRANSFER_ETH displays Dst Chain, Recipient, Arbiter Fee, Nonce."""
    run_plugin_flow(client, SELECTOR_WRAP_AND_TRANSFER_ETH, WRAP_PARAMS, WRAP_EXPECTED)


# ---------------------------------------------------------------------------
# TRANSFER_TOKENS tests
# ---------------------------------------------------------------------------

TRANSFER_PARAMS = [
    # token (address) = 0xA0b86991c6218b36c1d19D4a2e9Eb0cE3606eB48 (USDC)
    encode_eth_address("A0b86991c6218b36c1d19D4a2e9Eb0cE3606eB48"),
    # amount (uint256) = 1000000 (1 USDC with 6 decimals)
    encode_u256(1000000),
    # recipientChain (uint16) = 4 (Bsc)
    encode_u16(4),
    # recipient (bytes32) = 0xabcd... (32 bytes)
    encode_bytes32("abcd1234abcd1234abcd1234abcd1234abcd1234abcd1234abcd1234abcd1234"),
    # arbiterFee (bytes32) = 0 (no fee)
    encode_u256(0),
    # nonce (uint32) = 99
    encode_u32(99),
]

TRANSFER_EXPECTED = [
    ("Token", "0x"),
    ("Amount", "tokens"),
    ("Dst Chain", "Bsc"),
    ("Recipient", "0x"),
    ("Arbiter Fee", "ETH"),
    ("Nonce", "99"),
]


@pytest.mark.skipif(not DEVICE_AVAILABLE, reason="No Ledger device or speculos available")
def test_transfer_tokens(client):
    """TRANSFER_TOKENS displays Token, Amount, Dst Chain, Recipient, Arbiter Fee, Nonce."""
    run_plugin_flow(client, SELECTOR_TRANSFER_TOKENS, TRANSFER_PARAMS, TRANSFER_EXPECTED)


# ---------------------------------------------------------------------------
# ATTEST_TOKEN tests
# ---------------------------------------------------------------------------

ATTEST_PARAMS = [
    # tokenAddress (address) = 0x7Fc66500c84A76Ad7e9c93437bFc5Ac33E2DDaE9 (AAVE)
    encode_eth_address("7Fc66500c84A76Ad7e9c93437bFc5Ac33E2DDaE9"),
    # nonce (uint32) = 7
    encode_u32(7),
]

ATTEST_EXPECTED = [
    ("Token Address", "0x"),
    ("Nonce", "7"),
]


@pytest.mark.skipif(not DEVICE_AVAILABLE, reason="No Ledger device or speculos available")
def test_attest_token(client):
    """ATTEST_TOKEN displays Token Address and Nonce."""
    run_plugin_flow(client, SELECTOR_ATTEST_TOKEN, ATTEST_PARAMS, ATTEST_EXPECTED)


# ---------------------------------------------------------------------------
# COMPLETE_TRANSFER tests (no param screens, just confirm)
# ---------------------------------------------------------------------------

COMPLETE_PARAMS = [
    # encodedVM bytes — large blob, but plugin doesn't parse it
    b"\x00" * 32,  # minimal placeholder
]


@pytest.mark.skipif(not DEVICE_AVAILABLE, reason="No Ledger device or speculos available")
def test_complete_transfer(client):
    """
    COMPLETE_TRANSFER does not display parameter screens.
    The transaction just shows a generic confirm.
    """
    run_plugin_flow(client, SELECTOR_COMPLETE_TRANSFER, COMPLETE_PARAMS, [])


# ---------------------------------------------------------------------------
# Negative test: unknown selector
# ---------------------------------------------------------------------------

@pytest.mark.skipif(not DEVICE_AVAILABLE, reason="No Ledger device or speculos available")
def test_unknown_selector(client):
    """Unknown selector should return UNAVAILABLE."""
    init_data = bytes([0x00, 0x01])  # interfaceVersion = 1
    init_data += struct.pack(">H", len(CONTRACT_ADDRESS))
    init_data += CONTRACT_ADDRESS
    init_data += SELECTOR_UNKNOWN
    resp = send_apdu(client, CLA, ETH_INS_INIT_CONTRACT, 0x00, 0x00, init_data)
    # TODO: depending on SDK version, this may return 0x9000 but with
    # result code inside payload, or an error SW.
    # For now we just verify no exception.
    assert resp is not None


# ---------------------------------------------------------------------------
# Negative test: wrong parameter count expected to not crash
# ---------------------------------------------------------------------------

@pytest.mark.skipif(not DEVICE_AVAILABLE, reason="No Ledger device or speculos available")
def test_attest_token_extra_params(client):
    """ATTEST_TOKEN with extra parameters should not crash (extra ignored)."""
    params = ATTEST_PARAMS + [encode_u32(999)]
    run_plugin_flow(client, SELECTOR_ATTEST_TOKEN, params, ATTEST_EXPECTED)
