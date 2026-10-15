#include <string.h>
#include "protocol.h"

static void reply(char *out, size_t out_sz, const char *s)
{
    size_t n = strlen(s);
    if (n >= out_sz)
    {
        n = out_sz - 1u;
    }
    memcpy(out, s, n);
    out[n] = '\0';
}

static void reply_board(char *out, size_t out_sz, const char *prefix, const game_t *g)
{
    reply(out, out_sz, prefix);
    strncat(out, game_board(g), out_sz - strlen(out) - 1u);
}

static int hex_digit(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

/* Strips an optional *HH suffix. Returns 0 when it is present and wrong. */
static int checksum_ok(char *line)
{
    char *star = strrchr(line, '*');
    int   hi, lo, want, have = 0;
    char *p;

    if (star == NULL)
    {
        return 1;
    }
    hi = hex_digit(star[1]);
    lo = hex_digit(star[2]);
    if (hi < 0 || lo < 0 || star[3] != '\0')
    {
        return 0;
    }
    want = (hi << 4) | lo;

    for (p = line; p < star; p++)
    {
        have ^= (unsigned char)*p;
    }
    *star = '\0';
    return have == want;
}

static int parse_index(const char *s, int *out)
{
    if (s[0] < '0' || s[0] > '9' || s[1] != '\0')
    {
        return 0;
    }
    *out = s[0] - '0';
    return 1;
}

static void handle_move(server_t *s, char *arg, char *out, size_t out_sz)
{
    char *player_field = strchr(arg, ':');
    char  player = 0;
    int   index;

    if (player_field != NULL)
    {
        *player_field++ = '\0';
        if ((player_field[0] != 'X' && player_field[0] != 'O') || player_field[1] != '\0')
        {
            reply(out, out_sz, "NACK:BADCMD");
            return;
        }
        player = player_field[0];
    }

    if (!parse_index(arg, &index))
    {
        reply(out, out_sz, "NACK:BADCMD");
        return;
    }

    switch (game_move(&s->game, index, player))
    {
    case MOVE_OK:            reply_board(out, out_sz, "OK:BOARD:", &s->game); break;
    case MOVE_WIN:           reply(out, out_sz, s->game.winner == 'X' ? "WIN:X" : "WIN:O"); break;
    case MOVE_DRAW:          reply(out, out_sz, "DRAW"); break;
    case MOVE_ERR_RANGE:     reply(out, out_sz, "NACK:RANGE"); break;
    case MOVE_ERR_OCCUPIED:  reply(out, out_sz, "NACK:OCCUPIED"); break;
    case MOVE_ERR_NOTURN:    reply(out, out_sz, "NACK:NOTURN"); break;
    default:                 reply(out, out_sz, "NACK:BADCMD"); break;
    }
}

/* One complete line in, one response line out. Every path that rejects leaves
 * the game untouched, which is the whole point of the exercise. */
static void dispatch(server_t *s, char *line, char *out, size_t out_sz)
{
    char *arg;

    if (!checksum_ok(line))
    {
        reply(out, out_sz, "NACK:BADCRC");
        return;
    }
    arg = strchr(line, ':');
    if (arg != NULL)
    {
        *arg++ = '\0';
    }

    if (strcmp(line, "CONNECT") == 0 && arg == NULL)
    {
        reply(out, out_sz, "OK");
        return;
    }
    if (strcmp(line, "NEW") == 0 && arg == NULL)
    {
        if (s->game.state == GAME_IN_PROGRESS)
        {
            reply(out, out_sz, "NACK:NOGAME");
            return;
        }
        game_new(&s->game);
        reply_board(out, out_sz, "OK:BOARD:", &s->game);
        return;
    }
    if (strcmp(line, "MOVE") == 0 && arg != NULL)
    {
        if (s->game.state != GAME_IN_PROGRESS)
        {
            reply(out, out_sz, "NACK:NOGAME");
            return;
        }
        handle_move(s, arg, out, out_sz);
        return;
    }
    if (strcmp(line, "STATE") == 0 && arg == NULL)
    {
        if (s->game.state == GAME_IDLE)
        {
            reply(out, out_sz, "NACK:NOGAME");
            return;
        }
        reply_board(out, out_sz, "BOARD:", &s->game);
        return;
    }
    if (strcmp(line, "RESIGN") == 0 && arg == NULL)
    {
        if (s->game.state != GAME_IN_PROGRESS)
        {
            reply(out, out_sz, "NACK:NOGAME");
            return;
        }
        game_resign(&s->game);
        reply(out, out_sz, "OK:GAMEOVER");
        return;
    }
    if (strcmp(line, "DISCONNECT") == 0 && arg == NULL)
    {
        if (s->game.state == GAME_IN_PROGRESS)
        {
            reply(out, out_sz, "NACK:NOGAME");
            return;
        }
        game_init(&s->game);
        reply(out, out_sz, "OK");
        return;
    }

    reply(out, out_sz, "NACK:BADCMD");
}

void server_init(server_t *s)
{
    memset(s, 0, sizeof(*s));
    game_init(&s->game);
}

int server_rx_byte(server_t *s, char c, unsigned now_ms, char *out, size_t out_sz)
{
    s->last_rx_ms = now_ms;

    if (c == '\r')
    {
        return 0;
    }

    if (c != '\n')
    {
        if (c < 0x20 || c > 0x7E)
        {
            return 0;                      /* control and high bytes never frame */
        }
        if (s->len + 1u >= PROTO_MAX_FRAME)
        {
            s->discarding = 1;
            s->len = 0;
            return 0;
        }
        s->buf[s->len++] = c;
        return 0;
    }

    if (s->discarding)
    {
        s->discarding = 0;
        s->len = 0;
        reply(out, out_sz, "NACK:TOOLONG");
        return 1;
    }
    if (s->len == 0u)
    {
        return 0;                          /* bare newline, nothing to answer */
    }

    s->buf[s->len] = '\0';
    s->len = 0;
    dispatch(s, s->buf, out, out_sz);
    return 1;
}

void server_tick(server_t *s, unsigned now_ms)
{
    if ((s->len > 0u || s->discarding) &&
        (unsigned)(now_ms - s->last_rx_ms) >= PROTO_IDLE_MS)
    {
        s->len = 0;
        s->discarding = 0;
    }
}

char server_turn(const server_t *s)
{
    return (s->game.state == GAME_IN_PROGRESS) ? s->game.turn : 0;
}
