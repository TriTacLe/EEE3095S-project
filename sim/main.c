/* Host build of the server. Same core as the board; stdin and stdout stand in
 * for the UART, so the tests and the GUI run with no hardware present. */
#include <stdio.h>
#include <time.h>
#include "protocol.h"

static unsigned now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (unsigned)(ts.tv_sec * 1000u + ts.tv_nsec / 1000000u);
}

int main(void)
{
    server_t s;
    char     out[PROTO_MAX_RESPONSE];
    int      c;

    server_init(&s);
    setvbuf(stdout, NULL, _IONBF, 0);

    while ((c = fgetc(stdin)) != EOF)
    {
        if (server_rx_byte(&s, (char)c, now_ms(), out, sizeof(out)))
        {
            printf("%s\n", out);
        }
    }
    return 0;
}
