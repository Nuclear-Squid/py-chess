#!/bin/env python3
import ctypes
import tkinter as tk


from enum import Enum
from typing import Optional

class PieceColor(Enum):
    WHITE, BLACK = range(2)

    def __str__(self):
        match self:
            case PieceColor.WHITE: return "white"
            case PieceColor.BLACK: return "black"
            case _: raise ValueError(f"Unknown color")

    def __repr__(self):
        return self.__str__()
        

class PieceType(Enum):
    PAWN, ROOK, KNIGHT, BISHOP, QWEEN, KING = range(6)

    def __str__(self):
        match self:
            case PieceType.PAWN:   return "pawn"
            case PieceType.ROOK:   return "rook"
            case PieceType.KNIGHT: return "knight"
            case PieceType.BISHOP: return "bishop"
            case PieceType.QWEEN:  return "qween"
            case PieceType.KING:   return "king"
            case _: raise ValueError(f"Unknown piece type")

    def __repr__(self):
        return self.__str__()

class Cell(ctypes.Structure):
    _fields_ = [("color", ctypes.c_uint8, 1), ("type", ctypes.c_uint8, 3), ("is_empty", ctypes.c_uint8, 1)]

    def __str__(self):
        if self.is_empty:
            return "empty-cell"
        else:
            return f"{PieceColor(self.color)}-{PieceType(self.type)}"

    def __repr__(self):
        return self.__str__()

    def __eq__(self, other):
        return self.is_empty == other.is_empty or (self.color == other.color and self.type == other.type)
        
    def __hash__(self):
        return hash((self.color, self.type, self.is_empty))

class Position(ctypes.Structure):
    _fields_ = [("col", ctypes.c_uint8), ("row", ctypes.c_uint8)]

    def __str__(self):
        return f"(col: {self.col}, row: {self.row})"

    def __repr__(self):
        return self.__str__()

    def __eq__(self, other):
        return self.col == other.col and self.row == other.row


class ChessBoard((Cell * 8) * 8):
    def get_piece_at(self, pos: Position):
        return LIBCHESS.get_piece_at(self, pos)

    def log(self):
        LIBCHESS.debug_log_chess_board(self)

    def get_possible_moves(self, position):
        moves_buffer = (Position * 24)()
        nb_moves = LIBCHESS.get_possible_moves(self, position, moves_buffer)
        return moves_buffer[:nb_moves]


class KingStatus(Enum):
    NO_CHECKS, WHITE_IN_CHECK, BLACK_IN_CHECK, WHITE_CHECK_MATE, BLACK_CHECK_MATE = range(5)

    def __str__(self):
        match self:
            case KingStatus.NO_CHECKS:        return "No checks"
            case KingStatus.WHITE_IN_CHECK:   return "White in check"
            case KingStatus.BLACK_IN_CHECK:   return "Black in check"
            case KingStatus.WHITE_CHECK_MATE: return "Black won"
            case KingStatus.BLACK_CHECK_MATE: return "White won"
            case _: raise ValueError("Unknown KingStatus")

    def __repr__(self):
        return self.__str__()

    def __eq__(self, other):
        return self.value == other.value


class PlayedMoveStatus(ctypes.Structure):
    _fields_ = [
        ("was_valid", ctypes.c_bool),
        ("king_status", ctypes.c_uint8),
    ]

    def __str__(self):
        valid = "Valid" if self.was_valid else "Not valid"
        return f"({valid} move, {self.king_status})"

    def __eq__(self, other):
        return self.was_valid == other.was_valid and \
                self.king_status == other.king_status


LIBCHESS = ctypes.CDLL(".build/libchess.so")
LIBCHESS.get_main_chess_board.restype = ctypes.POINTER(ChessBoard)
LIBCHESS.get_possible_moves.restype = ctypes.c_size_t
LIBCHESS.get_color_to_play.restype = PieceColor
LIBCHESS.get_piece_at.restype = Cell
LIBCHESS.try_play_move.restype = PlayedMoveStatus

class ChessBoardWidget(tk.Canvas):
    LIGHT_COLOR = "#5B3C11"
    DARK_COLOR = "#C8AD7F"
    PIECE_COLOR = "#5B3C11"

    def __init__(self, parent, size, Dpieces):
        super().__init__(parent, width=size, height=size)

        self.cell_size: int = size // 8
        self.possible_moves: list[Position] = []
        self.selected_cell: Optional[Position] = None
        self.Dpieces = Dpieces

        # TODO: Render the pieces on the board
        self.render()

        def on_click(event):
            board = LIBCHESS.get_main_chess_board().contents
            clicked_cell = Position(event.x // self.cell_size, event.y // self.cell_size)
            piece_on_cell = board.get_piece_at(clicked_cell)
            color_to_play = LIBCHESS.get_color_to_play()

            if self.selected_cell is None or clicked_cell not in self.possible_moves:
                if piece_on_cell.color != color_to_play.value:
                    return
                self.possible_moves = board.get_possible_moves(clicked_cell)
                self.selected_cell = clicked_cell
                self.render()
                return
            

            # TODO: Check the status and render it appropriately
            LIBCHESS.try_play_move(board, self.selected_cell, clicked_cell)
            self.possible_moves = []
            board.log()
            self.render()

        self.bind("<Button-1>", on_click)


    def render(self):
        board = LIBCHESS.get_main_chess_board().contents
        board.log()
        for col in range(8):
            for row in range(8):
                start_corner = (col * self.cell_size, row * self.cell_size)
                end_corner = (col + 1) * self.cell_size, (row + 1) * self.cell_size
                color = self.LIGHT_COLOR if (col + row) % 2 == 0 else self.DARK_COLOR
                self.create_rectangle(start_corner, end_corner, fill=color, outline=color)
        for col in range(8):
            for row in range(8):
                current_cell = Position(row,col)
                piece_on_cell = board.get_piece_at(current_cell)
                print(current_cell, piece_on_cell.is_empty, piece_on_cell)
                if (not piece_on_cell.is_empty) :
                    pos_y = row *self.cell_size + self.cell_size / 2
                    pos_x= col *self.cell_size + self.cell_size /2
                    self.create_image(pos_y,pos_x, image=self.Dpieces[piece_on_cell])
        for case in self.possible_moves:
            taille=40
            start_corner=(case.col * self.cell_size+taille,case.row * self.cell_size+taille)
            end_corner=((case.col+1) * self.cell_size-taille,(case.row+1) * self.cell_size-taille)
            self.create_oval(start_corner,end_corner,fill='grey',outline='grey')                   
      


def main():
    root = tk.Tk()
    root.title("py-chess")
    
    Dpieces= {
        Cell(PieceColor.BLACK.value, PieceType.PAWN.value, False): tk.PhotoImage(file="frontend/Image/pion.png").subsample(10, 10),
        Cell(PieceColor.WHITE.value, PieceType.PAWN.value, False): tk.PhotoImage(file="frontend/Image/pion_blanc.png").subsample(10, 10),
        Cell(PieceColor.WHITE.value, PieceType.ROOK.value, False): tk.PhotoImage(file="frontend/Image/tourb.png").subsample(8, 6),
        Cell(PieceColor.BLACK.value, PieceType.ROOK.value, False): tk.PhotoImage(file="frontend/Image/tourn.png").subsample(8, 6),
        Cell(PieceColor.BLACK.value, PieceType.BISHOP.value, False): tk.PhotoImage(file="frontend/Image/foun.png").subsample(5, 6),
        Cell(PieceColor.WHITE.value, PieceType.BISHOP.value, False): tk.PhotoImage(file="frontend/Image/foub.png").subsample(5, 6),
        Cell(PieceColor.BLACK.value, PieceType.KNIGHT.value, False): tk.PhotoImage(file="frontend/Image/horsen.png").subsample(6, 6),
        Cell(PieceColor.WHITE.value, PieceType.KNIGHT.value, False): tk.PhotoImage(file="frontend/Image/horseb.png").subsample(6, 6),
        Cell(PieceColor.BLACK.value, PieceType.KING.value, False): tk.PhotoImage(file="frontend/Image/roin.png").subsample(6, 6),
        Cell(PieceColor.WHITE.value, PieceType.KING.value, False): tk.PhotoImage(file="frontend/Image/roib.png").subsample(6, 6),
        Cell(PieceColor.BLACK.value, PieceType.QWEEN.value, False): tk.PhotoImage(file="frontend/Image/reinen.png").subsample(6, 6),
        Cell(PieceColor.WHITE.value, PieceType.QWEEN.value, False): tk.PhotoImage(file="frontend/Image/reineb.png").subsample(6, 6)
    }

    ChessBoardWidget(root, 800, Dpieces).pack(anchor=tk.CENTER, expand=True)

    root.mainloop()

if __name__ == "__main__":
    main()
