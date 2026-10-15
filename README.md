# Serial Game Server

EEE3095S CS Project 2026. An STM32F051 acts as the referee for tic-tac-toe over
a UART link; a Python GUI renders the board and forwards clicks. The board owns
the game state and the client is never trusted with a rule.

## Layout

| Path | What it holds |
|---|---|
| `docs/` | protocol definition and the design diagrams |
| `server/core/` | the rules and the frame parser, plain C, no hardware |
| `server/stm32/` | board application and the port layer: UART, button, LED |
| `board/` | CubeIDE project for the STM32F051, builds the two above |
| `sim/` | host build of the same core, speaks the protocol over stdin and stdout |
| `client/` | Tkinter GUI |
| `tests/` | pytest harness, runs against the simulator or a serial port |
| `report/` | the report, LaTeX on the course template |

The same `server/core/` sources build for the board and for the host, so a rule
bug is found on a laptop and not in the lab.

## Setup

```
pip install -r requirements.txt
```

`pytest` is needed for the test harness and `pyserial` only to talk to the
board. The simulator and the firmware need no Python packages.

## Running without a board

```
make -C sim
python -m pytest tests
python client/gui.py --sim
```

## Running against the board

```
make -C board CONFIG=Release
st-flash write board/build/Release/GameServer.bin 0x08000000
python -m pytest tests --port /dev/ttyACM0
python client/gui.py --port /dev/ttyACM0
```

One USB cable carries both the flashing and the serial link; see
`board/README.md`.

## Building the report

```
make -C report          # builds report/main.pdf
make -C report check    # lists anything still unwritten
```

`IEEEtran.cls` and `IEEEtran.bst` are vendored in `report/`, so a plain TeX Live
installation is enough and `texlive-publishers` is not required.
