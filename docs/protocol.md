# Message protocol

ASCII, one message per line. The client sends a request, the server answers with
exactly one line. The server never speaks first.

## Framing

- Terminator is `\n`. A `\r` immediately before it is accepted and ignored, so a
  terminal that sends CRLF works.
- Maximum frame is 64 bytes including the terminator. A longer line is discarded
  up to and including the next `\n` and answered `NACK:TOOLONG`.
- A frame that has been sitting incomplete for 2000 ms is cleared. Whatever
  arrives next starts a new frame.
- Bytes outside printable ASCII are dropped as they arrive.
- Commands are uppercase. Anything unrecognised is `NACK:BADCMD`.

Nothing in this list can change the game. A rejected, truncated or malformed
request leaves the board, the turn and the game status exactly as they were.

## Board encoding

Nine characters, index 0 to 8, row major:

```
 0 | 1 | 2
 3 | 4 | 5
 6 | 7 | 8
```

`.` is an empty cell, `X` and `O` are the players. X always moves first, so the
client can derive whose turn it is by counting: equal counts means X to move.

## Commands

| Request | Response | Meaning |
|---|---|---|
| `CONNECT` | `OK` | Open a session. Server is IDLE. |
| `NEW` | `OK:BOARD:.........` | Start a game, X to move. |
| `MOVE:<i>` | `OK:BOARD:<9>` | Legal move, board after it. |
| `MOVE:<i>` | `WIN:X` / `WIN:O` / `DRAW` | The move ended the game. |
| `MOVE:<i>` | `NACK:RANGE` / `NACK:OCCUPIED` / `NACK:NOTURN` | Rejected, nothing changed. |
| `MOVE:<i>:<p>` | as above | Optional form, `<p>` is `X` or `O`. |
| `STATE` | `BOARD:<9>` | Current board. |
| `RESIGN` | `OK:GAMEOVER` | Forfeit. The other player is not credited a win. |
| `DISCONNECT` | `OK` | End the session, back to IDLE. |

Move checks run in the order range, turn, occupancy, so a client that names the
wrong player hears about that rather than about a cell it has no business
touching.

`<i>` is a single decimal digit. A `MOVE` with a missing or non-numeric argument
is `NACK:BADCMD`; a numeric one outside 0 to 8 is `NACK:RANGE`.

## States

| State | Accepts | Anything else |
|---|---|---|
| IDLE | `CONNECT`, `NEW`, `DISCONNECT` | `NACK:NOGAME` |
| IN_PROGRESS | `MOVE`, `STATE`, `RESIGN` | `NACK:NOGAME` |
| GAME_OVER | `NEW`, `STATE`, `DISCONNECT` | `NACK:NOGAME` |

A terminal move enters GAME_OVER, so the `WIN` or `DRAW` line is the last thing
that move produces. The client issues `STATE` afterwards to redraw the final
board, because the terminal responses carry no board by design.

## Departures from the brief

Four, all additive, none of them change a response the brief specifies:

- `NACK:TOOLONG` for an over-length frame. The brief asks for a maximum frame
  length but names no error for hitting it.
- The optional `MOVE:<i>:<p>` form. Without a player field a single client
  cannot be out of turn, so `NACK:NOTURN` would be unreachable and the
  out-of-turn requirement untestable.
- `DISCONNECT` is also accepted in IDLE. The brief lists it only under
  GAME_OVER, which would refuse a client that connects and then leaves without
  playing. It is still refused mid-game.
- An optional `*HH` checksum suffix on any request, `HH` being the XOR of every
  byte before the `*` as two uppercase hex digits. Absent, nothing is checked;
  present and wrong, the answer is `NACK:BADCRC` and the request is dropped.

## Examples

```
CONNECT            -> OK
NEW                -> OK:BOARD:.........
MOVE:4             -> OK:BOARD:....X....
MOVE:4             -> NACK:OCCUPIED
MOVE:9             -> NACK:RANGE
MOVE:0:O           -> OK:BOARD:O...X....
MOVE:1:O           -> NACK:NOTURN
STATE              -> BOARD:O...X....
MOVE:8*13          -> OK:BOARD:O...X...X
```
