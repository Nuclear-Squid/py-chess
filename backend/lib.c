#include <stdio.h>

#include "common_types.h"

static ChessBoard main_chess_board = {
    { {BLACK, ROOK}, {BLACK, KNIGHT}, {BLACK, BISHOP}, {BLACK, QWEEN}, {BLACK, KING}, {BLACK, BISHOP}, {BLACK, KNIGHT}, {BLACK, ROOK} },
    { {BLACK, PAWN}, {BLACK, PAWN}, {BLACK, PAWN}, {BLACK, PAWN}, {BLACK, PAWN}, {BLACK, PAWN}, {BLACK, PAWN}, {BLACK, PAWN} },
    { {BLACK, EMPTY}, {BLACK, EMPTY}, {BLACK, EMPTY}, {BLACK, EMPTY}, {BLACK, EMPTY}, {BLACK, EMPTY}, {BLACK, EMPTY}, {BLACK, EMPTY} },
    { {BLACK, EMPTY}, {BLACK, EMPTY}, {BLACK, EMPTY}, {BLACK, EMPTY}, {BLACK, EMPTY}, {BLACK, EMPTY}, {BLACK, EMPTY}, {BLACK, EMPTY} },
    { {BLACK, EMPTY}, {BLACK, EMPTY}, {BLACK, EMPTY}, {BLACK, EMPTY}, {BLACK, EMPTY}, {BLACK, EMPTY}, {BLACK, EMPTY}, {BLACK, EMPTY} },
    { {BLACK, EMPTY}, {BLACK, EMPTY}, {BLACK, EMPTY}, {BLACK, EMPTY}, {BLACK, EMPTY}, {BLACK, EMPTY}, {BLACK, EMPTY}, {BLACK, EMPTY} },
    { {WHITE, PAWN}, {WHITE, PAWN}, {WHITE, PAWN}, {WHITE, PAWN}, {WHITE, PAWN}, {WHITE, PAWN}, {WHITE, PAWN}, {WHITE, PAWN} },
    { {WHITE, ROOK}, {WHITE, KNIGHT}, {WHITE, BISHOP}, {WHITE, QWEEN}, {WHITE, KING}, {WHITE, BISHOP}, {WHITE, KNIGHT}, {WHITE, ROOK} },
};

static PieceColor color_to_play = WHITE;

ChessBoard* get_main_chess_board() { return &main_chess_board; }
PieceColor get_color_to_play() { return color_to_play; }

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

Piece get_piece_at(ChessBoard board, Position pos) {
    return board[pos.row][pos.col];
}

static inline CellState get_cell_state(ChessBoard board, Position cell_pos, PieceColor piece_color) {
    if (cell_pos.col >= 8 || cell_pos.row >= 8) return OUT_OF_BOUNDS;
    const Piece other_piece = get_piece_at(board, cell_pos);
    if (other_piece.type == EMPTY) return FREE;
    return other_piece.color == piece_color ? SAME_COLOR : OTHER_COLOR;
}

static size_t get_moves_line(ChessBoard board, Position pos, Direction dir, PieceColor color, Position* output) {
    // FIXME: This is fucking disgusting.
    Position aimed_cell;
    CellState aimed_cell_state;
    i32 i = 0;

    while ((aimed_cell_state = get_cell_state(board, aimed_cell = add_positions(pos, mul_position(dir, ++i)), color)) == FREE)
        output[i - 1] = aimed_cell;

    if (aimed_cell_state == OTHER_COLOR) {
        output[i - 1] = aimed_cell;
        return i;
    }

    return i - 1;
}

static size_t get_move_if_valid(ChessBoard board, Position pos, Direction dir, PieceColor color, Position* output) {
    Position aimed_cell = add_positions(pos, dir);
    switch (get_cell_state(board, aimed_cell, color)) {
        case FREE:
        case OTHER_COLOR:
            *output = aimed_cell;
            return 1;

        case SAME_COLOR:
        case OUT_OF_BOUNDS:
            return 0;
    }
}

size_t get_possible_moves(ChessBoard board, Position pos, Position* output) {
    const Piece piece = get_piece_at(board, pos);
    Direction pawn_direction = piece.color == WHITE
        ? (Direction) { .row = -1, .col =  0 }
        : (Direction) { .row =  1, .col =  0 };

    size_t nb_moves = 0;

    switch (piece.type) {
        case EMPTY: break;
        case PAWN:
            output[nb_moves++] = add_positions(pos, pawn_direction);
            if (pos.row == 1 || pos.row == 6)
                output[nb_moves++] = add_positions(pos, mul_position(pawn_direction, 2));

            pawn_direction.col = 1;
            Position aimed_cell = add_positions(pos, pawn_direction);
            if (get_cell_state(board, aimed_cell, piece.color) == OTHER_COLOR)
                output[nb_moves++] = aimed_cell;

            pawn_direction.col = -1;
            aimed_cell = add_positions(pos, pawn_direction);
            if (get_cell_state(board, aimed_cell, piece.color) == OTHER_COLOR)
                output[nb_moves++] = aimed_cell;
            break;

        case ROOK:
            nb_moves += get_moves_line(board, pos, (Direction) { .row = -1, .col =  0 }, piece.color, output + nb_moves);
            nb_moves += get_moves_line(board, pos, (Direction) { .row =  1, .col =  0 }, piece.color, output + nb_moves);
            nb_moves += get_moves_line(board, pos, (Direction) { .row =  0, .col = -1 }, piece.color, output + nb_moves);
            nb_moves += get_moves_line(board, pos, (Direction) { .row =  0, .col =  1 }, piece.color, output + nb_moves);
            break;

        case KNIGHT:
            nb_moves += get_move_if_valid(board, pos, (Direction) { .row =  1, .col =  2 }, piece.color, output + nb_moves);
            nb_moves += get_move_if_valid(board, pos, (Direction) { .row = -1, .col =  2 }, piece.color, output + nb_moves);
            nb_moves += get_move_if_valid(board, pos, (Direction) { .row =  1, .col = -2 }, piece.color, output + nb_moves);
            nb_moves += get_move_if_valid(board, pos, (Direction) { .row = -1, .col = -2 }, piece.color, output + nb_moves);
            nb_moves += get_move_if_valid(board, pos, (Direction) { .row =  2, .col =  1 }, piece.color, output + nb_moves);
            nb_moves += get_move_if_valid(board, pos, (Direction) { .row =  2, .col = -1 }, piece.color, output + nb_moves);
            nb_moves += get_move_if_valid(board, pos, (Direction) { .row = -2, .col =  1 }, piece.color, output + nb_moves);
            nb_moves += get_move_if_valid(board, pos, (Direction) { .row = -2, .col = -1 }, piece.color, output + nb_moves);
            break;

        case BISHOP:
            nb_moves += get_moves_line(board, pos, (Direction) { .row =  1, .col =  1 }, piece.color, output + nb_moves);
            nb_moves += get_moves_line(board, pos, (Direction) { .row =  1, .col = -1 }, piece.color, output + nb_moves);
            nb_moves += get_moves_line(board, pos, (Direction) { .row = -1, .col =  1 }, piece.color, output + nb_moves);
            nb_moves += get_moves_line(board, pos, (Direction) { .row = -1, .col = -1 }, piece.color, output + nb_moves);
            break;

        case QWEEN:
            nb_moves += get_moves_line(board, pos, (Direction) { .row = -1, .col =  0 }, piece.color, output + nb_moves);
            nb_moves += get_moves_line(board, pos, (Direction) { .row =  1, .col =  0 }, piece.color, output + nb_moves);
            nb_moves += get_moves_line(board, pos, (Direction) { .row =  0, .col = -1 }, piece.color, output + nb_moves);
            nb_moves += get_moves_line(board, pos, (Direction) { .row =  0, .col =  1 }, piece.color, output + nb_moves);
            nb_moves += get_moves_line(board, pos, (Direction) { .row =  1, .col =  1 }, piece.color, output + nb_moves);
            nb_moves += get_moves_line(board, pos, (Direction) { .row =  1, .col = -1 }, piece.color, output + nb_moves);
            nb_moves += get_moves_line(board, pos, (Direction) { .row = -1, .col =  1 }, piece.color, output + nb_moves);
            nb_moves += get_moves_line(board, pos, (Direction) { .row = -1, .col = -1 }, piece.color, output + nb_moves);
            break;

        case KING:
            nb_moves += get_move_if_valid(board, pos, (Direction) { .row = -1, .col =  0 }, piece.color, output + nb_moves);
            nb_moves += get_move_if_valid(board, pos, (Direction) { .row =  1, .col =  0 }, piece.color, output + nb_moves);
            nb_moves += get_move_if_valid(board, pos, (Direction) { .row =  0, .col = -1 }, piece.color, output + nb_moves);
            nb_moves += get_move_if_valid(board, pos, (Direction) { .row =  0, .col =  1 }, piece.color, output + nb_moves);
            nb_moves += get_move_if_valid(board, pos, (Direction) { .row =  1, .col =  1 }, piece.color, output + nb_moves);
            nb_moves += get_move_if_valid(board, pos, (Direction) { .row =  1, .col = -1 }, piece.color, output + nb_moves);
            nb_moves += get_move_if_valid(board, pos, (Direction) { .row = -1, .col =  1 }, piece.color, output + nb_moves);
            nb_moves += get_move_if_valid(board, pos, (Direction) { .row = -1, .col = -1 }, piece.color, output + nb_moves);
            break;
    }

    return nb_moves;
}

void play_move(ChessBoard board, Position start, Position end) {
    board[end.row][end.col] = board[start.row][start.col];
    board[start.row][start.col].type = EMPTY;
    color_to_play = color_to_play == WHITE ? BLACK : WHITE;
}

void debug_log_chess_board(ChessBoard board) {
    printf("+----\n");

    for (size_t row = 0; row < 8; row++) {
        printf("%c", row < 2 ? '|' : ' ');

        for (size_t col = 0; col < 8; col++) {
            Piece current_piece = get_piece_at(board, (Position) { col, row });
            if (current_piece.type == EMPTY) {
                printf(" --");
                continue;
            }

            printf(" %c", current_piece.color == WHITE ? 'w' : 'b');
            switch (current_piece.type) {
                case EMPTY: break;  // already handled in the code above
                case PAWN:   printf("p"); break;
                case ROOK:   printf("r"); break;
                case KNIGHT: printf("h"); break;  // 'h' for "horse"
                case BISHOP: printf("b"); break;
                case QWEEN:  printf("q"); break;
                case KING:   printf("k"); break;
            }
        }

        printf("%c\n", row >= 6 ? '|' : ' ');
    }

    printf("                     ----+\n");
}
