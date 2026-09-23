/* Host build of the board application. Same core, same app.c, same button and
 * LED logic as the firmware; stdin and stdout stand in for the UART, so the
 * tests and the GUI run with no hardware present.
 *
 * The button is never down and the LED goes to stderr, so a test run shows
 * turn changes without polluting the protocol stream. */
#include <stdio.h>
#include <time.h>

#include "app.h"
#include "port.h"

unsigned port_now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (unsigned)(ts.tv_sec * 1000u + ts.tv_nsec / 1000000u);
}

void port_tx(const char *line)
{
    printf("%s\n", line);
}

void port_led(int on)
{
    static int last = -1;
    if (on != last)
    {
        last = on;
        fprintf(stderr, "led %s\n", on ? "on" : "off");
    }
}

int port_button_down(void)
{
    return 0;
}

int main(void)
{
    int c;

    setvbuf(stdout, NULL, _IONBF, 0);
    app_init();

    while ((c = fgetc(stdin)) != EOF)
    {
        app_rx_byte((char)c);
        app_poll();
    }
    return 0;
}
