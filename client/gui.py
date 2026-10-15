import argparse
import pathlib
import queue
import sys
import threading
import tkinter as tk
from tkinter import font as tkfont

HERE = pathlib.Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
sys.path.insert(0, str(HERE.parent))

from session import interpret  # noqa: E402
from link import SerialTransport, SimTransport  # noqa: E402

BG = "#f4f4f2"
FG = "#1b1b1b"
ACCENT = "#1d4ed8"
MUTED = "#4a4a4a"
CELL = "#ffffff"
CELL_LOCKED = "#e4e4e0"   # a finished board reads as inactive at a glance


class Client(tk.Tk):
    def __init__(self, transport):
        super().__init__()
        self.transport = transport
        self.replies = queue.Queue()
        self.pending = []
        self.playable = False

        self.title("Serial Game Server")
        self.configure(bg=BG, padx=20, pady=20)
        self.resizable(False, False)

        self.cell_font = tkfont.Font(family="Helvetica", size=30, weight="bold")
        self.text_font = tkfont.Font(family="Helvetica", size=14)

        self.cells = []
        grid = tk.Frame(self, bg=BG)
        grid.pack()
        for i in range(9):
            b = tk.Button(
                grid,
                text=" ",
                width=3,
                height=1,
                font=self.cell_font,
                fg=FG,
                bg=CELL,
                activebackground="#e8eeff",
                highlightthickness=1,
                command=lambda i=i: self.on_cell(i),
            )
            b.grid(row=i // 3, column=i % 3, padx=4, pady=4, ipadx=8, ipady=8)
            self.cells.append(b)

        self.status = tk.Label(
            self,
            text="Connecting...",
            font=self.text_font,
            fg=MUTED,
            bg=BG,
            wraplength=320,
            justify="left",
        )
        self.status.pack(pady=(16, 8), anchor="w")

        row = tk.Frame(self, bg=BG)
        row.pack(fill="x")
        self.new_button = tk.Button(
            row, text="New game", font=self.text_font, fg="#ffffff", bg=ACCENT,
            activebackground="#1b3fae", padx=16, pady=8, command=self.on_new,
        )
        self.new_button.pack(side="left")
        tk.Button(
            row, text="Resign", font=self.text_font, fg=FG, bg="#e6e6e3",
            padx=16, pady=8, command=lambda: self.send("RESIGN"),
        ).pack(side="left", padx=8)
        tk.Button(
            row, text="Disconnect", font=self.text_font, fg=FG, bg="#e6e6e3",
            padx=16, pady=8, command=self.on_quit,
        ).pack(side="right")

        self.protocol("WM_DELETE_WINDOW", self.on_quit)

        threading.Thread(target=self.reader, daemon=True).start()
        self.after(50, self.drain)
        self.send("CONNECT")

    # --- transport -------------------------------------------------------
    def reader(self):
        while True:
            try:
                self.replies.put(self.transport.recv(timeout=60))
            except Exception:
                return

    def send(self, line):
        self.pending.append(line)
        self.transport.send(line)

    def drain(self):
        while not self.replies.empty():
            self.apply(self.replies.get())
        self.after(50, self.drain)

    # --- responses -------------------------------------------------------
    def apply(self, reply):
        request = self.pending.pop(0) if self.pending else ""
        view = interpret(request, reply)

        # Playability first: draw() reflects it, and a STATE redraw after a
        # finished game must not make the cells look live again.
        if view.playable is not None:
            self.playable = view.playable
        if view.board is not None:
            self.draw(view.board)
        if view.finished:
            self.lock_board()
            self.send("STATE")
        if view.message:
            self.say(view.message)

    def draw(self, board):
        live = self.playable
        for i, cell in enumerate(board[:9]):
            self.cells[i].config(text=" " if cell == "." else cell,
                                 state="normal" if live else "disabled",
                                 bg=CELL if live else CELL_LOCKED,
                                 disabledforeground=FG)

    def lock_board(self):
        for b in self.cells:
            b.config(state="disabled", bg=CELL_LOCKED, disabledforeground=FG)

    def say(self, message):
        self.status.config(text=message)

    # --- input -----------------------------------------------------------
    def on_cell(self, index):
        if not self.playable:
            self.say("No game running. Start a new game.")
            return
        self.send("MOVE:%d" % index)

    def on_new(self):
        self.send("NEW")

    def on_quit(self):
        try:
            self.transport.send("DISCONNECT")
            self.transport.close()
        except Exception:
            pass
        self.destroy()


def main():
    ap = argparse.ArgumentParser(
        description="Tic-tac-toe client for the serial game server.")
    ap.add_argument("--port", help="serial port of the board, e.g. /dev/ttyACM0")
    ap.add_argument("--sim", action="store_true", help="use the host simulator")
    args = ap.parse_args()

    root = HERE.parent
    if args.port:
        transport = SerialTransport(args.port)
    elif args.sim:
        transport = SimTransport(root / "sim" / "server_sim")
    else:
        ap.error("pass --port <device> or --sim")

    Client(transport).mainloop()


if __name__ == "__main__":
    main()
