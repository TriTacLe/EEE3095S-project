# Design

## Where the rules live

The board is the referee. The client sends requests and draws whatever comes
back; it holds no copy of the rules. Everything that decides anything is in
`server/core/`, which builds for the STM32 and for the host without a change,
so the same code that runs in the lab is what the tests run on a laptop.

```mermaid
flowchart LR
    subgraph PC
        GUI[Tkinter client]
        TESTS[pytest harness]
    end
    subgraph Board[STM32F051]
        PORT[port_hal.c<br/>UART, button, LED]
        APP[app.c<br/>button, LED, session]
        CORE[core<br/>rules and parser]
    end
    GUI -- request line --> PORT
    TESTS -. same protocol .-> PORT
    PORT --> APP --> CORE
    CORE -- response line --> PORT
    PORT -- reply --> GUI
```

## Server state machine

```mermaid
stateDiagram-v2
    [*] --> IDLE
    IDLE --> IDLE: CONNECT / OK
    IDLE --> IN_PROGRESS: NEW / OK:BOARD
    IDLE --> IDLE: DISCONNECT / OK
    IN_PROGRESS --> IN_PROGRESS: MOVE legal / OK:BOARD
    IN_PROGRESS --> IN_PROGRESS: MOVE illegal / NACK, state unchanged
    IN_PROGRESS --> IN_PROGRESS: STATE / BOARD
    IN_PROGRESS --> GAME_OVER: MOVE wins or fills / WIN or DRAW
    IN_PROGRESS --> GAME_OVER: RESIGN / OK:GAMEOVER
    GAME_OVER --> IN_PROGRESS: NEW / OK:BOARD
    GAME_OVER --> GAME_OVER: STATE / BOARD
    GAME_OVER --> IDLE: DISCONNECT / OK
```

Anything a state does not accept is answered `NACK:NOGAME` and changes nothing.

## Move validation

```mermaid
flowchart TD
    A[MOVE:i received] --> B{i a single digit?}
    B -- no --> BAD[NACK:BADCMD]
    B -- yes --> C{0 to 8?}
    C -- no --> RANGE[NACK:RANGE]
    C -- yes --> D{player named and not the turn?}
    D -- yes --> TURN[NACK:NOTURN]
    D -- no --> E{cell empty?}
    E -- no --> OCC[NACK:OCCUPIED]
    E -- yes --> F[place the mark]
    F --> G{three in a line?}
    G -- yes --> WIN[WIN and GAME_OVER]
    G -- no --> H{board full?}
    H -- yes --> DRAW[DRAW and GAME_OVER]
    H -- no --> I[swap the turn, return OK:BOARD]
```

Each rejection returns before the board is touched, which is what makes the
client-untrusted claim testable: the test asserts the board after the rejection
is the board before it.

## Frame handling

```mermaid
flowchart TD
    A[byte in] --> B{printable?}
    B -- no --> DROP[drop it]
    B -- yes --> C{newline?}
    C -- no --> D{buffer full?}
    D -- yes --> E[mark discarding, empty the buffer]
    D -- no --> F[append]
    C -- yes --> G{discarding?}
    G -- yes --> H[NACK:TOOLONG, reset]
    G -- no --> I[dispatch the line]
    J[main loop tick] --> K{partial frame older than 2 s?}
    K -- yes --> L[drop it, ready for the next line]
```

## GUI flow

```mermaid
flowchart TD
    START([start]) --> CONN[open the link, send CONNECT]
    CONN --> WAIT[wait for a reply]
    WAIT --> NEW[New game pressed, send NEW]
    NEW --> BOARD[draw the board from the reply]
    BOARD --> CLICK{cell clicked}
    CLICK --> SEND[send MOVE:i]
    SEND --> REPLY{reply}
    REPLY -- OK:BOARD --> BOARD
    REPLY -- NACK --> MSG[show the server's reason, redraw nothing]
    MSG --> CLICK
    REPLY -- WIN or DRAW --> END[lock the board, announce, send STATE]
    END --> NEW
    BOARD --> QUIT[Disconnect pressed, send DISCONNECT and close]
```

The client never decides that a click is illegal. It sends the move and shows
what the server says about it.

## Link

```mermaid
flowchart LR
    PC[PC] -- one USB cable --> STL[ST-LINK on the board]
    STL -- SWD --> STM[STM32F051 course board]
    STL -- virtual COM port --> VCP["/dev/ttyACM0 on the PC"]
    VCP -- USART1, PA9 TX and PA10 RX --> STM
    STM --- LED[LED0 on PB0, turn indicator]
    STM --- BTN[Button0 on PA0, new game]
```

One USB cable does both jobs. The on-board ST-LINK flashes the part over SWD
and presents a USB virtual COM port that is wired to USART1, so the client
opens that port and no external adapter is involved. 115200 8N1, no flow
control.
