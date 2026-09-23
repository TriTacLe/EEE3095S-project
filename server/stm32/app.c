#include "app.h"
#include "port.h"
#include "protocol.h"

#define DEBOUNCE_MS   20u
#define BLINK_MS     400u

static server_t  server;
static int       btn_stable;
static int       btn_last_raw;
static unsigned  btn_changed_ms;
static unsigned  blink_ms;
static int       blink_on;

void app_init(void)
{
    server_init(&server);
    btn_stable   = 0;
    btn_last_raw = 0;
    blink_on     = 0;
}

/* Interrupt context: parse the byte, answer if the line is complete. Responses
 * are at most 32 bytes, so sending here is cheap enough to keep the protocol
 * strictly request-response. */
void app_rx_byte(char c)
{
    char out[PROTO_MAX_RESPONSE];

    if (server_rx_byte(&server, c, port_now_ms(), out, sizeof(out)))
    {
        port_tx(out);
    }
}

/* The button asks for a new game, the same thing the client asks for with NEW,
 * so it goes through the server rather than touching the board directly. */
static void poll_button(unsigned now)
{
    int raw = port_button_down();

    if (raw != btn_last_raw)
    {
        btn_last_raw   = raw;
        btn_changed_ms = now;
        return;
    }
    if (raw == btn_stable || (unsigned)(now - btn_changed_ms) < DEBOUNCE_MS)
    {
        return;
    }

    btn_stable = raw;
    if (raw)
    {
        char out[PROTO_MAX_RESPONSE];
        const char *cmd = "NEW\n";
        int i;
        for (i = 0; cmd[i] != '\0'; i++)
        {
            if (server_rx_byte(&server, cmd[i], now, out, sizeof(out)))
            {
                port_tx(out);   /* the client sees the new board too */
            }
        }
    }
}

/* Solid while X is to move, off while O is, blinking once the game is over.
 * A glance at the board says whose turn it is without the GUI. */
static void poll_led(unsigned now)
{
    char turn = server_turn(&server);

    if (turn == 'X')
    {
        port_led(1);
    }
    else if (turn == 'O')
    {
        port_led(0);
    }
    else if ((unsigned)(now - blink_ms) >= BLINK_MS)
    {
        blink_ms = now;
        blink_on = !blink_on;
        port_led(blink_on);
    }
}

void app_poll(void)
{
    unsigned now = port_now_ms();

    server_tick(&server, now);
    poll_button(now);
    poll_led(now);
}
