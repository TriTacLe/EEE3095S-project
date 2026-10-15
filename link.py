"""Two ways to talk to the server: the host simulator, or a real serial port.

The tests never know which one they got, so the same suite covers the laptop
build and the board.
"""

import subprocess
import time


class SimTransport:
    """Runs sim/server_sim and speaks to it over its stdin and stdout."""

    def __init__(self, binary):
        self.proc = subprocess.Popen(
            [str(binary)],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            bufsize=0,
        )

    def send(self, line):
        self.proc.stdin.write((line + "\n").encode())
        self.proc.stdin.flush()

    def send_raw(self, data):
        self.proc.stdin.write(data)
        self.proc.stdin.flush()

    def recv(self, timeout=2.0):
        deadline = time.time() + timeout
        chunks = b""
        while time.time() < deadline:
            byte = self.proc.stdout.read(1)
            if not byte:
                break
            if byte == b"\n":
                return chunks.decode().strip()
            chunks += byte
        raise TimeoutError("no response line from the simulator")

    def ask(self, line, timeout=2.0):
        self.send(line)
        return self.recv(timeout)

    def reset(self):
        """Nothing to do: every test gets its own process, already in IDLE."""

    def close(self):
        self.proc.stdin.close()
        self.proc.terminate()
        self.proc.wait(timeout=2)


class SerialTransport:
    """The same interface over a UART. Needs pyserial."""

    def __init__(self, port, baud=115200):
        import serial

        self.ser = serial.Serial(port, baud, timeout=2)
        time.sleep(0.2)
        self.ser.reset_input_buffer()

    def send(self, line):
        self.ser.write((line + "\n").encode())

    def send_raw(self, data):
        self.ser.write(data)

    def recv(self, timeout=2.0):
        self.ser.timeout = timeout
        line = self.ser.readline()
        if not line:
            raise TimeoutError("no response line from the board")
        return line.decode(errors="replace").strip()

    def ask(self, line, timeout=2.0):
        self.send(line)
        return self.recv(timeout)

    def reset(self):
        """Put the board back in IDLE with an empty game.

        Unlike the simulator the board survives between tests, so a test that
        leaves a game running would break the next one: NEW is refused while a
        game is in progress. RESIGN then DISCONNECT reaches IDLE from any of
        the three states, and either may be refused along the way.
        """
        self.ser.write(b"\n")          # end any partial frame left behind
        time.sleep(0.05)
        self.ser.reset_input_buffer()
        for command in ("RESIGN", "DISCONNECT"):
            self.send(command)
            try:
                self.recv(timeout=1.0)
            except TimeoutError:
                pass

    def close(self):
        self.ser.close()
