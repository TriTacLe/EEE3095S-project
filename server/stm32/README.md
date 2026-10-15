# Board side

`app.c` is the application: it owns the server, debounces the button and drives
the LED. It has no HAL in it, so it compiles on a laptop and the rules can be
tested without hardware.

`port_stm32.c` is the only file that knows about the board. It provides the
functions in `port.h` and feeds the USART receive interrupt to the parser.

The CubeIDE project that builds these lives in `board/`. It links to this
directory and to `server/core/` rather than copying them, so there is one copy
of every source and the host simulator in `sim/` compiles the same files. See
`board/README.md` for how to build and flash.

## Pins

Button0 on PA0, behind a pull-up and so active low. LED0 on PB0: solid while X
is to move, off while O is, blinking whenever no game is running. Both taken
from the Practical 1A and 1B projects that ran on this board, where the buttons
sit on PA0 to PA3 and the LEDs on PB0 to PB7.

## Hardware

No external adapter. USART1 (PA9 TX, PA10 RX) is wired on-board to the ST-LINK,
which doubles as a USB virtual COM port, so the cable you flash with also gives
the PC a serial port. 115200 8N1. The PC side appears as /dev/ttyACM0 on Linux.

The UART adapter boards handed out earlier are for the older dev boards and the
course has asked for them back; do not use one here.
