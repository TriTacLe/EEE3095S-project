REJECTIONS = {
    "NACK:OCCUPIED": "That cell is taken.",
    "NACK:RANGE": "That cell is not on the board.",
    "NACK:NOTURN": "Not that player's turn.",
    "NACK:NOGAME": "The server will not accept that right now.",
    "NACK:BADCMD": "The server did not understand that.",
    "NACK:BADCRC": "Checksum rejected, the message was dropped.",
    "NACK:TOOLONG": "That message was too long for the server.",
}


class View:
    """What to render. board is None when the reply carried no board."""

    def __init__(self, board=None, message="", playable=None, finished=False):
        self.board = board
        self.message = message
        self.playable = playable
        self.finished = finished

    def __repr__(self):
        return "View(board=%r, message=%r, playable=%r, finished=%r)" % (
            self.board, self.message, self.playable, self.finished)


def interpret(request, reply):
    if reply.startswith("OK:BOARD:") or reply.startswith("BOARD:"):
        board = reply.split("BOARD:", 1)[1]
        if request == "NEW":
            return View(board, "New game. X starts.", playable=True)
        if request == "STATE":
            # A redraw after a finished game must not make the board live again,
            # so STATE never speaks about playability.
            return View(board)
        return View(board, "Move accepted.", playable=True)

    if reply == "OK":
        if request == "CONNECT":
            return View(message="Connected. Start a new game.")
        if request == "DISCONNECT":
            return View(message="Disconnected.", playable=False)
        return View(message="Done.")

    if reply.startswith("WIN:"):
        return View(message="%s wins." % reply.split(":", 1)[1],
                    playable=False, finished=True)

    if reply == "DRAW":
        return View(message="A draw.", playable=False, finished=True)

    if reply == "OK:GAMEOVER":
        return View(message="Game resigned.", playable=False, finished=True)

    if reply.startswith("NACK:"):
        return View(message=REJECTIONS.get(reply, reply))

    return View(message="Unexpected reply: %s" % reply)
