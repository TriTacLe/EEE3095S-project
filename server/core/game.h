#ifndef GAME_H
#define GAME_H

#define GAME_CELLS 9

typedef enum {
    GAME_IDLE = 0,
    GAME_IN_PROGRESS,
    GAME_OVER
} game_state_t;

typedef enum {
    MOVE_OK = 0,
    MOVE_WIN,
    MOVE_DRAW,
    MOVE_ERR_RANGE,
    MOVE_ERR_OCCUPIED,
    MOVE_ERR_NOTURN
} move_result_t;

typedef struct {
    char         cells[GAME_CELLS + 1];  /* '.', 'X' or 'O', NUL terminated */
    char         turn;                   /* 'X' or 'O' */
    char         winner;                 /* 'X', 'O' or 0 for none */
    game_state_t state;
} game_t;

void game_init(game_t *g);    /* empty board, IDLE */
void game_new(game_t *g);     /* empty board, X to move, IN_PROGRESS */
void game_resign(game_t *g);  /* straight to GAME_OVER, no winner credited */

/* player is 'X', 'O', or 0 when the client did not say. A rejected move
 * changes nothing in *g. */
move_result_t game_move(game_t *g, int index, char player);

const char *game_board(const game_t *g);

#endif /* GAME_H */
