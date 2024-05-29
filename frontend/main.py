#!/bin/env python3
import ctypes
import tkinter as tk
import threading

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

    def get_opposite(self):
        match self:
            case PieceColor.WHITE: return PieceColor.BLACK
            case PieceColor.BLACK: return PieceColor.WHITE

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

    def find_cell(self, cell):
        return LIBCHESS.find_cell(self, cell)


class KingStatus(Enum):
    NO_CHECKS, CHECK, CHECK_MATE = range(3)

    def __str__(self):
        match self:
            case KingStatus.NO_CHECKS:  return "No checks"
            case KingStatus.CHECK:      return "Check"
            case KingStatus.CHECK_MATE: return "Check mate"
            case _: raise ValueError("Unknown KingStatus")

    def __repr__(self):
        return self.__str__()

    def __eq__(self, other):
        return self.value == other.value


class PlayedMoveStatus(ctypes.Structure):
    _fields_ = [
        ("your_king_in_check", ctypes.c_bool),
        ("ennemy_king_status", ctypes.c_uint8),
        ("draw_match", ctypes.c_bool),
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
LIBCHESS.find_cell.restype = Position


timer: Optional[threading.Timer] = None
texte_timer1: Optional[tk.Label] = None
texte_timer2: Optional[tk.Label] = None

def kill_timer():
    global timer, texte_timer1, texte_timer2
    if timer is None or texte_timer1 is None or texte_timer2 is None:
        raise ValueError("timer or texte timer is None")

    timer.cancel()
    texte_timer1['text']= "termine"
    texte_timer2['text']= "termine"


def chrono(duree_noir, duree_blanc, board):
    global timer, texte_timer1, texte_timer2
    if texte_timer1 is None or texte_timer2 is None:
        raise ValueError("texte timer is None")

    texte_timer1['text']= str(round(duree_noir, 2)) + "s"
    texte_timer2['text']= str(round(duree_blanc, 2)) + "s"

    if duree_noir <= 0.0 or duree_blanc <= 0.0:
        out_of_time_color = PieceColor(LIBCHESS.get_color_to_play())
        opponent_color = out_of_time_color.get_opposite()
        board.show_message(f"{out_of_time_color} is out of time\n{opponent_color} wins")
        kill_timer()
        return

    if LIBCHESS.get_color_to_play() == PieceColor.WHITE:
        timer = threading.Timer(0.1, chrono, [duree_noir, duree_blanc-0.1, board])
    else:
        timer = threading.Timer(0.1, chrono, [duree_noir-0.1, duree_blanc, board])
    timer.start()

def quit(tk_root):
    def inner():
        global timer
        if not timer is None:
            timer.cancel()
        tk_root.destroy()

    return inner

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
            move_status = LIBCHESS.try_play_move(board, self.selected_cell, clicked_cell)

            self.possible_moves = []
            self.selected_cell = None
            self.render()

            if move_status.your_king_in_check:
                taille=60
                case = board.find_cell(Cell(color_to_play.value, PieceType.KING.value))
                start_corner=(case.col * self.cell_size+taille,case.row * self.cell_size+taille)
                end_corner=((case.col+1) * self.cell_size-taille,(case.row+1) * self.cell_size-taille)
                self.create_oval(start_corner,end_corner,fill='red',outline='red')
                threading.Timer(0.5, self.render).start()
                return

            if move_status.draw_match:
                self.show_message("stalemate")
                return

            match KingStatus(move_status.ennemy_king_status):
                case KingStatus.NO_CHECKS:
                    pass

                case KingStatus.CHECK:
                    taille=60
                    case = board.find_cell(Cell(color_to_play.get_opposite().value, PieceType.KING.value))
                    start_corner=(case.col * self.cell_size+taille,case.row * self.cell_size+taille)
                    end_corner=((case.col+1) * self.cell_size-taille,(case.row+1) * self.cell_size-taille)
                    self.create_oval(start_corner,end_corner,fill='red',outline='red')
                    return

                case KingStatus.CHECK_MATE:
                    self.show_message(f"{color_to_play} wins\nby checkmate")
                    kill_timer()
                    return

        self.bind("<Button-1>", on_click)


    def render(self):
        self.delete("all")
        board = LIBCHESS.get_main_chess_board().contents
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
                if (not piece_on_cell.is_empty) :
                    pos_y = row *self.cell_size + self.cell_size / 2
                    pos_x= col *self.cell_size + self.cell_size /2
                    self.create_image(pos_y,pos_x, image=self.Dpieces[piece_on_cell])

        for case in self.possible_moves:
            taille=40
            start_corner=(case.col * self.cell_size+taille,case.row * self.cell_size+taille)
            end_corner=((case.col+1) * self.cell_size-taille,(case.row+1) * self.cell_size-taille)
            self.create_oval(start_corner,end_corner,fill='grey',outline='grey')


    def show_message(self, message):
        border_length = self.cell_size * 8
        center = (border_length / 2, border_length / 2)
        self.create_text(center, text=message, anchor="center", fill="red", font="Arial 30 bold", justify="center")

    def resign(self):
        resigned_color = PieceColor(LIBCHESS.get_color_to_play())
        opponent_color = resigned_color.get_opposite()
        self.show_message(f"{resigned_color} resigns\n{opponent_color} wins")
        kill_timer()

def main():
    global texte_timer1, texte_timer2

    root = tk.Tk()
    root.title("py-chess")

    Dpieces= {
        Cell(PieceColor.BLACK.value, PieceType.PAWN.value, False): tk.PhotoImage(file="frontend/Image/pion.png").subsample(4, 4),
        Cell(PieceColor.WHITE.value, PieceType.PAWN.value, False): tk.PhotoImage(file="frontend/Image/pionb.png").subsample(4, 4),
        Cell(PieceColor.WHITE.value, PieceType.ROOK.value, False): tk.PhotoImage(file="frontend/Image/tourb.png").subsample(4, 4),
        Cell(PieceColor.BLACK.value, PieceType.ROOK.value, False): tk.PhotoImage(file="frontend/Image/tourn.png").subsample(4, 4),
        Cell(PieceColor.BLACK.value, PieceType.BISHOP.value, False): tk.PhotoImage(file="frontend/Image/foun.png").subsample(4, 4),
        Cell(PieceColor.WHITE.value, PieceType.BISHOP.value, False): tk.PhotoImage(file="frontend/Image/foub.png").subsample(4, 4),
        Cell(PieceColor.BLACK.value, PieceType.KNIGHT.value, False): tk.PhotoImage(file="frontend/Image/horsen.png").subsample(4, 4),
        Cell(PieceColor.WHITE.value, PieceType.KNIGHT.value, False): tk.PhotoImage(file="frontend/Image/horseb.png").subsample(4, 4),
        Cell(PieceColor.BLACK.value, PieceType.KING.value, False): tk.PhotoImage(file="frontend/Image/roin.png").subsample(4, 4),
        Cell(PieceColor.WHITE.value, PieceType.KING.value, False): tk.PhotoImage(file="frontend/Image/roib.png").subsample(4, 4),
        Cell(PieceColor.BLACK.value, PieceType.QWEEN.value, False): tk.PhotoImage(file="frontend/Image/reinen.png").subsample(4, 4),
        Cell(PieceColor.WHITE.value, PieceType.QWEEN.value, False): tk.PhotoImage(file="frontend/Image/reineb.png").subsample(4, 4)
    }

    texte_timer1 = tk.Label(root,text="")
    texte_timer1.pack()

    board = ChessBoardWidget(root, 600, Dpieces)
    board.pack(anchor=tk.CENTER, expand=True)

    texte_timer2 = tk.Label(root,text="")
    texte_timer2.pack()

    tk.Button(root, text="quit", command=quit(root)).pack()
    tk.Button(root, text="resign", command=board.resign).pack()

    duree_noir  = 100.0
    duree_blanc = 100.0
    chrono(duree_noir, duree_blanc, board)

    root.mainloop()

if __name__ == "__main__":
    main()
