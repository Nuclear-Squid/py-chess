#include <stdio.h>
#include <stdbool.h>

#include "stdint_aliases.h"

typedef enum: u8 { WHITE, BLACK } PieceColor;
typedef enum: u8 { EMPTY, PAWN, ROOK, KNIGHT, BISHOP, QWEEN, KING } PiecesType;
typedef struct {
    PieceColor color: 1;
    PiecesType type : 3;
} Piece;

typedef Piece ChessBoard[8][8];

typedef struct {
    u8 col;
    u8 row;
} Position;
typedef Position Direction;

typedef enum: u8 { OUT_OF_BOUNDS, FREE, SAME_COLOR, OTHER_COLOR } CellState;

static void log_position(Position pos) {
    printf("(col: %hhu, row: %hhu)\n", pos.col, pos.row);
}

static inline Position add_positions(Position pos_a, Position pos_b) {
    return (Position) {
        .col = pos_a.col + pos_b.col,
        .row = pos_a.row + pos_b.row,
    };
}

static inline Position mul_position(Position pos, i8 multiplier) {
    return (Position) {
        .col = pos.col * multiplier,
        .row = pos.row * multiplier,
    };
}

static inline Piece get_piece_at(ChessBoard board, Position pos) {
    return board[pos.row][pos.col];
}

static inline CellState get_cell_state(ChessBoard board, Position cell_pos, PieceColor piece_color) {
    if (cell_pos.col >= 8 || cell_pos.row >= 8) return OUT_OF_BOUNDS;
    const Piece other_piece = get_piece_at(board, cell_pos);
    if (other_piece.type == EMPTY) return FREE;
    printf("%hhu, %hhu\n", other_piece.color, piece_color);
    return other_piece.color == piece_color ? SAME_COLOR : OTHER_COLOR;
}

static void show_moves_line(ChessBoard board, Position pos, Direction dir, PieceColor color) {
    // FIXME: This is fucking disgusting.
    Position aimed_cell;
    CellState aimed_cell_state;
    i32 i = 0;

    while ((aimed_cell_state = get_cell_state(board, aimed_cell = add_positions(pos, mul_position(dir, ++i)), color)) == FREE)
        log_position(aimed_cell);
    if (aimed_cell_state == OTHER_COLOR) log_position(aimed_cell);
}

static void show_move_if_valid(ChessBoard board, Position pos, Direction dir, PieceColor color) {
    Position aimed_cell = add_positions(pos, dir);
    switch (get_cell_state(board, aimed_cell, color)) {
        case FREE:
        case OTHER_COLOR:
            return log_position(aimed_cell);

        case SAME_COLOR:
        case OUT_OF_BOUNDS:
            break;
    }
}

void show_possible_moves(ChessBoard board, Position pos) {
    const Piece piece = get_piece_at(board, pos);
    Direction pawn_direction = piece.color == WHITE
        ? (Direction) { .row = -1, .col =  0 }
        : (Direction) { .row =  1, .col =  0 };

    switch (piece.type) {
        case EMPTY: break;
        case PAWN:
            log_position(add_positions(pos, pawn_direction));
            if (pos.row == 1 || pos.row == 6)
                log_position(add_positions(pos, mul_position(pawn_direction, 2)));

            pawn_direction.col = 1;
            Position aimed_cell = add_positions(pos, pawn_direction);
            if (get_cell_state(board, aimed_cell, piece.color) == OTHER_COLOR)
                log_position(aimed_cell);

            pawn_direction.col = -1;
            aimed_cell = add_positions(pos, pawn_direction);
            if (get_cell_state(board, aimed_cell, piece.color) == OTHER_COLOR)
                log_position(aimed_cell);

            break;

        case ROOK:
            show_moves_line(board, pos, (Direction) { .row = -1, .col =  0 }, piece.color);
            show_moves_line(board, pos, (Direction) { .row =  1, .col =  0 }, piece.color);
            show_moves_line(board, pos, (Direction) { .row =  0, .col = -1 }, piece.color);
            show_moves_line(board, pos, (Direction) { .row =  0, .col =  1 }, piece.color);
            break;

        case KNIGHT:
            show_move_if_valid(board, pos, (Direction) { .row =  1, .col =  2 }, piece.color);
            show_move_if_valid(board, pos, (Direction) { .row = -1, .col =  2 }, piece.color);
            show_move_if_valid(board, pos, (Direction) { .row =  1, .col = -2 }, piece.color);
            show_move_if_valid(board, pos, (Direction) { .row = -1, .col = -2 }, piece.color);
            show_move_if_valid(board, pos, (Direction) { .row =  2, .col =  1 }, piece.color);
            show_move_if_valid(board, pos, (Direction) { .row =  2, .col = -1 }, piece.color);
            show_move_if_valid(board, pos, (Direction) { .row = -2, .col =  1 }, piece.color);
            show_move_if_valid(board, pos, (Direction) { .row = -2, .col = -1 }, piece.color);
            break;

        case BISHOP:
            show_moves_line(board, pos, (Direction) { .row =  1, .col =  1 }, piece.color);
            show_moves_line(board, pos, (Direction) { .row =  1, .col = -1 }, piece.color);
            show_moves_line(board, pos, (Direction) { .row = -1, .col =  1 }, piece.color);
            show_moves_line(board, pos, (Direction) { .row = -1, .col = -1 }, piece.color);
            break;

        case QWEEN:
            show_moves_line(board, pos, (Direction) { .row = -1, .col =  0 }, piece.color);
            show_moves_line(board, pos, (Direction) { .row =  1, .col =  0 }, piece.color);
            show_moves_line(board, pos, (Direction) { .row =  0, .col = -1 }, piece.color);
            show_moves_line(board, pos, (Direction) { .row =  0, .col =  1 }, piece.color);
            show_moves_line(board, pos, (Direction) { .row =  1, .col =  1 }, piece.color);
            show_moves_line(board, pos, (Direction) { .row =  1, .col = -1 }, piece.color);
            show_moves_line(board, pos, (Direction) { .row = -1, .col =  1 }, piece.color);
            show_moves_line(board, pos, (Direction) { .row = -1, .col = -1 }, piece.color);
            break;

        case KING:
            show_move_if_valid(board, pos, (Direction) { .row = -1, .col =  0 }, piece.color);
            show_move_if_valid(board, pos, (Direction) { .row =  1, .col =  0 }, piece.color);
            show_move_if_valid(board, pos, (Direction) { .row =  0, .col = -1 }, piece.color);
            show_move_if_valid(board, pos, (Direction) { .row =  0, .col =  1 }, piece.color);
            show_move_if_valid(board, pos, (Direction) { .row =  1, .col =  1 }, piece.color);
            show_move_if_valid(board, pos, (Direction) { .row =  1, .col = -1 }, piece.color);
            show_move_if_valid(board, pos, (Direction) { .row = -1, .col =  1 }, piece.color);
            show_move_if_valid(board, pos, (Direction) { .row = -1, .col = -1 }, piece.color);
            break;
    }
}
