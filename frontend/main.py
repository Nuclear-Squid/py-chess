#!/bin/env python3
import ctypes
import tkinter as tk

from enum import Enum

class PieceColor(Enum):
    WHITE, BLACK = range(2)

class PieceType(Enum):
    EMPTY, PAWN, ROOK, KNIGHT, BISHOP, QWEEN, KING = range(7)

class Piece(ctypes.Structure):
    _fields_ = [("color", ctypes.c_uint8, 1), ("type", ctypes.c_uint8, 3)]

class Position(ctypes.Structure):
    _fields_ = [("col", ctypes.c_uint8), ("row", ctypes.c_uint8)]

    @classmethod
    def new(cls, col, row):
        return cls(ctypes.c_uint8(col), ctypes.c_uint8(row))


LIBCHESS = ctypes.CDLL("zig-out/lib/libchess.so")
LIBCHESS.get_main_chess_board.restype = ctypes.POINTER((Piece * 8) * 8)

class ChessBoardWidget(tk.Canvas):
    LIGHT_COLOR = "#cccccc"
    DARK_COLOR = "#444444"

    def __init__(self, parent, size):
        super().__init__(parent, width=size, height=size)

        self.cell_size = size // 8

        self.bind("<Button-1>", lambda event:
            LIBCHESS.show_possible_moves(
                LIBCHESS.get_main_chess_board().contents,
                Position.new(event.x // self.cell_size, event.y // self.cell_size)
            )
        )


        for col in range(8):
            for row in range(8):
                start_corner = (col * self.cell_size, row * self.cell_size)
                end_corner = (col + 1) * self.cell_size, (row + 1) * self.cell_size
                color = self.LIGHT_COLOR if (col + row) % 2 == 0 else self.DARK_COLOR
                self.create_rectangle(start_corner, end_corner, fill=color, outline=color)


def main():
    root = tk.Tk()
    root.title("py-chess")

    ChessBoardWidget(root, 400).pack(anchor=tk.CENTER, expand=True)

    root.mainloop()

if __name__ == "__main__":
    main()
