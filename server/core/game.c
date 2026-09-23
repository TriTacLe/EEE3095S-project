#include "game.h"

static const int LINES[8][3] = {
    {0,1,2}, {3,4,5}, {6,7,8},   /* rows */
    {0,3,6}, {1,4,7}, {2,5,8},   /* columns */
    {0,4,8}, {2,4,6}             /* diagonals */
};

static void clear_board(game_t *g)
{
    int i;
    for (i = 0; i < GAME_CELLS; i++)
    {
        g->cells[i] = '.';
    }
    g->cells[GAME_CELLS] = '\0';
    g->winner = 0;
}

void game_init(game_t *g)
{
    clear_board(g);
    g->turn  = 'X';
    g->state = GAME_IDLE;
}

void game_new(game_t *g)
{
    clear_board(g);
    g->turn  = 'X';
    g->state = GAME_IN_PROGRESS;
}

void game_resign(game_t *g)
{
    g->state  = GAME_OVER;
    g->winner = 0;
}

static int line_won(const game_t *g, char p)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        if (g->cells[LINES[i][0]] == p &&
            g->cells[LINES[i][1]] == p &&
            g->cells[LINES[i][2]] == p)
        {
            return 1;
        }
    }
    return 0;
}

static int board_full(const game_t *g)
{
    int i;
    for (i = 0; i < GAME_CELLS; i++)
    {
        if (g->cells[i] == '.')
        {
            return 0;
        }
    }
    return 1;
}

/* Checks run range, then turn, then occupancy, so a client that names the wrong
 * player is told that rather than being told about a cell it may not touch. */
move_result_t game_move(game_t *g, int index, char player)
{
    if (index < 0 || index >= GAME_CELLS)
    {
        return MOVE_ERR_RANGE;
    }
    if (player != 0 && player != g->turn)
    {
        return MOVE_ERR_NOTURN;
    }
    if (g->cells[index] != '.')
    {
        return MOVE_ERR_OCCUPIED;
    }

    g->cells[index] = g->turn;

    if (line_won(g, g->turn))
    {
        g->winner = g->turn;
        g->state  = GAME_OVER;
        return MOVE_WIN;
    }
    if (board_full(g))
    {
        g->state = GAME_OVER;
        return MOVE_DRAW;
    }

    g->turn = (g->turn == 'X') ? 'O' : 'X';
    return MOVE_OK;
}

const char *game_board(const game_t *g)
{
    return g->cells;
}
