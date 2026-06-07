import pytest
from ledgercomm import Transport


@pytest.fixture(scope="session")
def client():
    """Create a Ledger transport client (speculos or real device)."""
    transport = Transport(interface="hid", debug=True)
    transport.open()
    yield transport
    transport.close()
