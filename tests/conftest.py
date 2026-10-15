import pathlib
import subprocess
import sys

import pytest

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))

from link import SerialTransport, SimTransport  # noqa: E402

SIM = ROOT / "sim" / "server_sim"


def pytest_addoption(parser):
    parser.addoption(
        "--port",
        default=None,
        help="serial port of the board, e.g. /dev/ttyACM0. Without it the "
        "host simulator is used.",
    )


@pytest.fixture(scope="session")
def simulator_binary():
    subprocess.run(["make", "-s"], cwd=ROOT / "sim", check=True)
    return SIM


@pytest.fixture
def server(request, simulator_binary):
    port = request.config.getoption("--port")
    link = SerialTransport(port) if port else SimTransport(simulator_binary)
    link.reset()
    try:
        yield link
    finally:
        link.close()


@pytest.fixture
def game(server):
    """A server with a session open and a fresh game running."""
    assert server.ask("CONNECT") == "OK"
    assert server.ask("NEW") == "OK:BOARD:........."
    return server
