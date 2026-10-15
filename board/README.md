# Board project

STM32CubeIDE project for the STM32F051C6 on the UCT dev board. It builds the
game core in `../server/core` and the board application in `../server/stm32`,
reached through the linked folders `GameCore` and `BoardApp` so there is one
copy of each source and the host simulator compiles the same files.

## Build without the IDE

```
make                  # Debug
make CONFIG=Release
make clean
```

`arm-none-eabi-gcc` on PATH is enough; failing that the Makefile looks in the
usual CubeIDE toolchain locations. The Release build is about 5 kB of flash and
2.7 kB of RAM, well inside the 32 kB and 8 kB on this part.

## Build in the IDE

File, Import, Existing Projects into Workspace, and pick this directory. The
managed Debug and Release configurations in `.cproject` carry the same include
paths as the Makefile.

Only the clock and the two GPIO pins come from `GameServer.ioc`. The USART is
configured from registers in `../server/stm32/port_stm32.c` because the HAL
vendored with the course projects has no UART driver, which is the same split
the practicals use for their own peripherals. Opening the `.ioc` and
regenerating is therefore not needed, and regenerating will not produce the
USART setup.

## Flash

Any of the usual routes: the IDE's run button, `st-flash write
build/Release/GameServer.bin 0x08000000`, or `openocd` with the stlink
interface. One USB cable does both the flashing and the serial link.

## What runs

`main` brings up the 48 MHz clock, the button and the LED, then the USART, then
hands over to `app_poll` in a bare loop. Received bytes arrive through
`USART1_IRQHandler` and go straight into the frame parser, so nothing in the
main loop blocks.
