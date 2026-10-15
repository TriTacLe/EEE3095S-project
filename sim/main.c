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
