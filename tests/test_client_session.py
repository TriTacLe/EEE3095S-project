"""The client's reading of server replies, with no window in the way."""

import pathlib
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "client"))

from session import interpret  # noqa: E402


def test_new_game_draws_the_empty_board():
    view = interpret("NEW", "OK:BOARD:.........")
    assert view.board == "........."
    assert view.playable is True


def test_a_confirmed_move_redraws():
    view = interpret("MOVE:4", "OK:BOARD:....X....")
    assert view.board == "....X...."
    assert view.playable is True


def test_a_rejection_shows_the_reason_and_does_not_redraw():
    view = interpret("MOVE:4", "NACK:OCCUPIED")
    assert view.board is None
    assert view.playable is None
    assert "taken" in view.message


def test_an_unknown_rejection_is_shown_verbatim():
    assert "NACK:WHAT" in interpret("MOVE:4", "NACK:WHAT").message


def test_a_win_finishes_the_game():
    view = interpret("MOVE:2", "WIN:X")
    assert view.finished is True
    assert view.playable is False
    assert "X wins" in view.message


def test_a_draw_finishes_the_game():
    assert interpret("MOVE:7", "DRAW").finished is True


def test_state_after_a_win_redraws_without_reopening_the_board():
    view = interpret("STATE", "BOARD:XXXOO....")
    assert view.board == "XXXOO...."
    assert view.playable is None


def test_connect_reports_the_session():
    assert "Connected" in interpret("CONNECT", "OK").message
