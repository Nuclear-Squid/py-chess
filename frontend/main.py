#!/bin/env python3
import ctypes
import tkinter as tk

from enum import Enum

LIBCHESS = ctypes.CDLL("zig-out/lib/libchess.so")

class PieceColor(Enum):
    WHITE, BLACK = range(2)

    @classmethod
    def from_str(cls, input: str):
        match input:
            case "white":
                return cls.WHITE
            case "black":
                return cls.BLACK
            case other:
                raise ValueError(f"Unknown PieceColor value `{other}`")


class PieceType(Enum):
    EMPTY, PAWN, ROOK, KNIGHT, BISHOP, QWEEN, KING = range(0, 7)

    @classmethod
    def from_str(cls, input: str):
        match input:
            case "empty" | "":
                return cls.EMPTY
            case "pawn":
                return cls.PAWN
            case "rook":
                return cls.ROOK
            case "knight":
                return cls.KNIGHT
            case "bishop":
                return cls.BISHOP
            case "qween":
                return cls.QWEEN
            case "king":
                return cls.KING
            case other:
                raise ValueError(f"Unknown PieceType value `{other}`")


class Position(ctypes.Structure):
    _fields_ = [("col", ctypes.c_uint8), ("row", ctypes.c_uint8)]

    @classmethod
    def new(cls, col, row):
        return cls(ctypes.c_uint8(col), ctypes.c_uint8(row))

class Piece(ctypes.Structure):
    _fields_ = [("color", ctypes.c_uint8, 1), ("type", ctypes.c_uint8, 3)]

    @classmethod
    def new(cls, color: str, piece_type: str):
        return cls(PieceColor.from_str(color).value, PieceType.from_str(piece_type).value)

    @classmethod
    def empty(cls):
        return cls(PieceColor.WHITE.value, PieceType.EMPTY.value)


class ChessBoard:
    def __init__(self):
        P = Piece.new # shorter alias
        E = Piece.empty
        board_line = Piece * 8
        self.pieces = (board_line * 8)(
            board_line(P("black", "rook"), P("black", "knight"), P("black", "bishop"), P("black", "qween"), P("black", "king"), P("black", "bishop"), P("black", "knight"), P("black", "rook")),
            board_line(P("black", "pawn"), P("black", "pawn"), P("black", "pawn"), P("black", "pawn"), P("black", "pawn"), P("black", "pawn"), P("black", "pawn"), P("black", "pawn")),
            board_line(E(), E(), E(), E(), E(), E(), E(), E()),
            board_line(E(), E(), E(), E(), E(), E(), E(), E()),
            board_line(E(), E(), E(), E(), E(), E(), E(), E()),
            board_line(E(), E(), E(), E(), E(), E(), E(), E()),
            board_line(P("white", "pawn"), P("white", "pawn"), P("white", "pawn"), P("white", "pawn"), P("white", "pawn"), P("white", "pawn"), P("white", "pawn"), P("white", "pawn")),
            board_line(P("white", "rook"), P("white", "knight"), P("white", "bishop"), P("white", "qween"), P("white", "king"), P("white", "bishop"), P("white", "knight"), P("white", "rook")),
        )


class ChessBoardWidget(tk.Canvas):
    LIGHT_COLOR = "#cccccc"
    DARK_COLOR = "#444444"

    def __init__(self, parent, size):
        super().__init__(parent, width=size, height=size)

        self.cell_size = size // 8
        self.board = ChessBoard()

        self.bind("<Button-1>", lambda event:
            LIBCHESS.show_possible_moves(
                self.board.pieces,
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
