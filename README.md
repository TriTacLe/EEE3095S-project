# Serial Game Server

EEE3095S CS Project 2026. An STM32F051 acts as the referee for tic-tac-toe over
a UART link; a Python GUI renders the board and forwards clicks. The board owns
the game state and the client is never trusted with a rule.

## Layout

| Path | What it holds |
|---|---|
| `docs/` | protocol definition and the design diagrams |
| `server/core/` | the rules and the frame parser, plain C, no hardware |
| `server/stm32/` | CubeIDE project: UART, button, LED, main loop |
| `sim/` | host build of the same core, speaks the protocol over stdin and stdout |
| `client/` | Tkinter GUI |
| `tests/` | pytest harness, runs against the simulator or a serial port |
| `report/` | the report |

The same `server/core/` sources build for the board and for the host, so a rule
bug is found on a laptop and not in the lab.

## Running without a board

```
make -C sim
python -m pytest tests
python client/gui.py --sim
```

## Running against the board

```
python -m pytest tests --port /dev/ttyACM0
python client/gui.py --port /dev/ttyACM0
```
