# Board side

`app.c` is the application: it owns the server, debounces the button and drives
the LED. It has no HAL in it, so it compiles on a laptop and the rules can be
tested without hardware.

`port_hal.c` is the only file that knows about the board. It provides the four
functions in `port.h` and wires the UART receive interrupt to the parser.

## Building it

1. New STM32CubeIDE project for the STM32F051 on the course board.
2. Enable the USART the STM32USB2UART adapter reaches, asynchronous, 115200,
   8 data bits, no parity, 1 stop bit, no flow control. Confirm the TX and RX
   pins against the board pinout before generating code.
3. Enable the USART global interrupt in the NVIC tab.
4. Copy `server/core/*.c`, `server/core/*.h`, `app.c`, `app.h`, `port.h` and
   `port_hal.c` into `Core/Src` and `Core/Inc`.
5. In the generated `main.c`, between the USER CODE markers:

```c
app_init();
HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
while (1)
{
    app_poll();
}
```

The button and LED pins at the top of `port_hal.c` are the ones the practicals
used. Check them against this board before the first run.

## Hardware

The adapter board, its 2 x 5 header and the soldering are the course UART
tutorial's job. 115200 8N1, adapter TX to board RX and adapter RX to board TX,
grounds together, and the UART cable is a second USB connection separate from
the programming one.
