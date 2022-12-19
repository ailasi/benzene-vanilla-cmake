"""A game of Hex.

Contains a list of moves and a result."""


class Game:

    def __init__(self):
        self._result = '?'
        self._elapsedBlack = 0.0
        self._elapsedWhite = 0.0
        self._moves = []

    def add_move(self, move):
        self._moves.append(move)

    def move_list(self):
        return self._moves

    def played_swap(self):
        if len(self._moves) >= 1 and self._moves[1] == 'swap-pieces':
            return True
        return False

    def get_length(self):
        return len(self._moves)

    def set_result(self, result):
        self._result = result

    def get_result(self):
        return self._result

    def set_elapsed(self, color, elapsed):
        if color == "black":
            self._elapsedBlack = elapsed
        elif color == "white":
            self._elapsedWhite = elapsed

    def get_elapsed(self, color):
        if color == "black":
            return self._elapsedBlack
        elif color == "white":
            return self._elapsedWhite
