/* Framing, parsing and dispatch. Feed it bytes as they arrive; it hands back a
 * complete response line when a request has been dealt with. Nothing here
 * touches hardware either. */
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stddef.h>
#include "game.h"

#define PROTO_MAX_FRAME    64u    /* bytes including the terminator */
#define PROTO_MAX_RESPONSE 32u
#define PROTO_IDLE_MS      2000u  /* a partial frame older than this is dropped */

typedef struct {
    char     buf[PROTO_MAX_FRAME];
    size_t   len;
    int      discarding;      /* over-length: skip to the next newline */
    unsigned last_rx_ms;
    game_t   game;
} server_t;

void server_init(server_t *s);

/* Returns 1 when *out holds a response line (without the terminator). */
int server_rx_byte(server_t *s, char c, unsigned now_ms, char *out, size_t out_sz);

/* Call from the main loop so a client that stops mid-frame does not wedge it. */
void server_tick(server_t *s, unsigned now_ms);

/* For the LED: 'X', 'O', or 0 when no game is running. */
char server_turn(const server_t *s);

#endif /* PROTOCOL_H */
