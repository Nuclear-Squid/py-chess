#!/bin/env python3
import ctypes
import tkinter as tk

from enum import Enum

class PieceColor(Enum):
    WHITE, BLACK = range(2)

    @classmethod
    def from_param(cls, obj):
        return obj

    def __str__(self):
        match self:
            case PieceColor.WHITE: return "white"
            case PieceColor.BLACK: return "black"
            case other: raise ValueError(f"Unknown color {other}")

    def __repr__(self):
        return self.__str__()

class PieceType(Enum):
    EMPTY, PAWN, ROOK, KNIGHT, BISHOP, QWEEN, KING = range(7)

    def __str__(self):
        match self:
            case PieceType.EMPTY:  return "empty"
            case PieceType.PAWN:   return "pawn"
            case PieceType.ROOK:   return "rook"
            case PieceType.KNIGHT: return "knight"
            case PieceType.BISHOP: return "bishop"
            case PieceType.QWEEN:  return "qween"
            case PieceType.KING:   return "king"
            case other: raise ValueError(f"Unknown piece type {other}")

    def __repr__(self):
        return self.__str__()

class Piece(ctypes.Structure):
    _fields_ = [("color", ctypes.c_uint8, 1), ("type", ctypes.c_uint8, 3)]

    def __str__(self):
        return f"{PieceColor(self.color)}-{PieceType(self.type)}"

    def __repr__(self):
        return self.__str__()

    def __eq__(self, other):
        return self.color == other.color and self.type == other.type

class Position(ctypes.Structure):
    _fields_ = [("col", ctypes.c_uint8), ("row", ctypes.c_uint8)]

    def __str__(self):
        return f"(col: {self.col}, row: {self.row})"

    def __repr__(self):
        return self.__str__()

    def __eq__(self, other):
        return self.col == other.col and self.row == other.row

LIBCHESS = ctypes.CDLL("zig-out/lib/libchess.so")
LIBCHESS.get_main_chess_board.restype = ctypes.POINTER((Piece * 8) * 8)
LIBCHESS.get_possible_moves.restype = ctypes.c_size_t
LIBCHESS.get_color_to_play.restype = PieceColor
LIBCHESS.get_piece_at.restype = Piece

class ChessBoardWidget(tk.Canvas):
    LIGHT_COLOR = "#cccccc"
    DARK_COLOR = "#444444"

    def __init__(self, parent, size):
        super().__init__(parent, width=size, height=size)

        self.cell_size = size // 8
        self.moves_buffer = []
        self.selected_cell = None

        # TODO: Render the pieces on the board

        def clicked_cell(event):
            board = LIBCHESS.get_main_chess_board().contents
            cell_clicked = Position(event.x // self.cell_size, event.y // self.cell_size)
            color_to_play = LIBCHESS.get_color_to_play()

            if cell_clicked in self.moves_buffer:
                LIBCHESS.play_move(board, self.selected_cell, cell_clicked)
                return

            if LIBCHESS.get_piece_at(board, cell_clicked).color == color_to_play.value:
                self.selected_cell = cell_clicked
                self.moves_buffer = (Position * 24)()
                nb_moves = LIBCHESS.get_possible_moves(board, cell_clicked, self.moves_buffer)
                self.moves_buffer = self.moves_buffer[:nb_moves]
                return

        self.bind("<Button-1>", clicked_cell)

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
