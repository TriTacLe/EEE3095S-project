import pytest

EMPTY = "OK:BOARD:........."


def test_connect_returns_ok(server):
    assert server.ask("CONNECT") == "OK"


def test_new_returns_an_empty_board(server):
    server.ask("CONNECT")
    assert server.ask("NEW") == EMPTY


def test_a_legal_move_changes_the_board_exactly(game):
    assert game.ask("MOVE:4") == "OK:BOARD:....X...."
    assert game.ask("MOVE:0") == "OK:BOARD:O...X...."


def test_occupied_cell_is_rejected_and_nothing_changes(game):
    game.ask("MOVE:4")
    assert game.ask("MOVE:4") == "NACK:OCCUPIED"
    assert game.ask("STATE") == "BOARD:....X...."


def test_out_of_range_is_rejected(game):
    assert game.ask("MOVE:9") == "NACK:RANGE"
    assert game.ask("STATE") == "BOARD:........."


def test_out_of_turn_is_rejected(game):
    assert game.ask("MOVE:0:O") == "NACK:NOTURN"
    assert game.ask("STATE") == "BOARD:........."


def test_a_win_is_detected(game):
    for cell in ["0", "3", "1", "4"]:
        game.ask("MOVE:" + cell)
    assert game.ask("MOVE:2") == "WIN:X"


def test_a_draw_is_detected(game):
    # 0 1 2 / 3 4 5 / 6 7 8, X starts: X0 O1 X2 O3 X4 O6 X5 O8 X7 fills with no line.
    for cell in ["0", "1", "2", "3", "4", "6", "5", "8"]:
        assert game.ask("MOVE:" + cell).startswith("OK:BOARD:")
    assert game.ask("MOVE:7") == "DRAW"


def test_moves_after_game_over_are_refused(game):
    for cell in ["0", "3", "1", "4"]:
        game.ask("MOVE:" + cell)
    assert game.ask("MOVE:2") == "WIN:X"
    assert game.ask("MOVE:5") == "NACK:NOGAME"


def test_state_is_readable_after_the_game_ends(game):
    for cell in ["0", "3", "1", "4"]:
        game.ask("MOVE:" + cell)
    game.ask("MOVE:2")
    assert game.ask("STATE") == "BOARD:XXXOO...."


def test_resign_ends_the_game(game):
    assert game.ask("RESIGN") == "OK:GAMEOVER"
    assert game.ask("MOVE:0") == "NACK:NOGAME"


def test_new_game_after_game_over(game):
    game.ask("RESIGN")
    assert game.ask("NEW") == EMPTY


@pytest.mark.parametrize(
    "request_line",
    ["HELLO", "MOVE", "MOVE:", "MOVE:X", "MOVE:44", "move:4", "MOVE:4:Z"],
)
def test_malformed_requests_are_rejected(game, request_line):
    assert game.ask(request_line).startswith("NACK:")
    assert game.ask("STATE") == "BOARD:........."


def test_an_oversized_frame_is_rejected_and_the_parser_recovers(game):
    assert game.ask("MOVE:" + "9" * 200) == "NACK:TOOLONG"
    assert game.ask("MOVE:4") == "OK:BOARD:....X...."


def test_garbage_bytes_do_not_corrupt_the_next_request(game):
    game.send_raw(b"\x00\x01\xff\x7f")
    assert game.ask("MOVE:4") == "OK:BOARD:....X...."


def test_a_split_request_is_reassembled(game):
    game.send_raw(b"MOV")
    game.send_raw(b"E:4")
    game.send_raw(b"\n")
    assert game.recv() == "OK:BOARD:....X...."


def test_carriage_returns_are_tolerated(game):
    game.send_raw(b"MOVE:4\r\n")
    assert game.recv() == "OK:BOARD:....X...."


def test_a_correct_checksum_is_accepted(game):
    assert game.ask("MOVE:4*1F") == "OK:BOARD:....X...."


def test_a_wrong_checksum_is_rejected(game):
    assert game.ask("MOVE:4*00") == "NACK:BADCRC"
    assert game.ask("STATE") == "BOARD:........."
