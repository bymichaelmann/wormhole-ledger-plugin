import pytest
from ragger.backend import SpeculosBackend
from ragger.firmware import Firmware
from ragger.navigator import NavInsID


@pytest.fixture(params=[Firmware.NANOS, Firmware.NANOSP, Firmware.NANOX])
def firmware(request):
    return request.param


@pytest.fixture
def backend(firmware):
    """Create a speculos backend with the Wormhole plugin loaded."""
    with SpeculosBackend(
        firmware=firmware,
        app_name="wormhole-portal",
        args=["--plugin", "wormhole-portal,0x3ee18b2214aff97000d974cf647e7c347e8fa585"],
    ) as b:
        yield b


@pytest.fixture
def client(backend):
    """Return the APDU client from the backend."""
    return backend.client
