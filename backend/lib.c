#include <stdio.h>

#include "common_types.h"

static ChessBoard main_chess_board = {
    { {BLACK, ROOK}, {BLACK, KNIGHT}, {BLACK, BISHOP}, {BLACK, QWEEN}, {BLACK, KING}, {BLACK, BISHOP}, {BLACK, KNIGHT}, {BLACK, ROOK} },
    { {BLACK, PAWN}, {BLACK, PAWN}, {BLACK, PAWN}, {BLACK, PAWN}, {BLACK, PAWN}, {BLACK, PAWN}, {BLACK, PAWN}, {BLACK, PAWN} },
    { {.is_empty = true}, {.is_empty = true}, {.is_empty = true}, {.is_empty = true}, {.is_empty = true}, {.is_empty = true}, {.is_empty = true}, {.is_empty = true} },
    { {.is_empty = true}, {.is_empty = true}, {.is_empty = true}, {.is_empty = true}, {.is_empty = true}, {.is_empty = true}, {.is_empty = true}, {.is_empty = true} },
    { {.is_empty = true}, {.is_empty = true}, {.is_empty = true}, {.is_empty = true}, {.is_empty = true}, {.is_empty = true}, {.is_empty = true}, {.is_empty = true} },
    { {.is_empty = true}, {.is_empty = true}, {.is_empty = true}, {.is_empty = true}, {.is_empty = true}, {.is_empty = true}, {.is_empty = true}, {.is_empty = true} },
    { {WHITE, PAWN}, {WHITE, PAWN}, {WHITE, PAWN}, {WHITE, PAWN}, {WHITE, PAWN}, {WHITE, PAWN}, {WHITE, PAWN}, {WHITE, PAWN} },
    { {WHITE, ROOK}, {WHITE, KNIGHT}, {WHITE, BISHOP}, {WHITE, QWEEN}, {WHITE, KING}, {WHITE, BISHOP}, {WHITE, KNIGHT}, {WHITE, ROOK} },
};

static PieceColor color_to_play = WHITE;

static KingStatus king_status = NO_CHECKS;

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

static inline bool eq_positions(Position pos_a, Position pos_b) {
    return pos_a.col == pos_b.col && pos_a.row == pos_b.row;
}

Cell get_piece_at(ChessBoard board, Position pos) {
    return board[pos.row][pos.col];
}

void set_piece_at(ChessBoard board, Position pos, Cell piece) {
    board[pos.row][pos.col] = piece;
}

static inline CellState get_cell_state(ChessBoard board, Position cell_pos, PieceColor piece_color) {
    if (cell_pos.col >= 8 || cell_pos.row >= 8) return OUT_OF_BOUNDS;
    const Cell other_piece = get_piece_at(board, cell_pos);
    if (other_piece.is_empty) return FREE;
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
    const Cell piece = get_piece_at(board, pos);
    if (piece.is_empty) return 0;

    Direction pawn_direction = piece.color == WHITE
        ? (Direction) { .row = -1, .col =  0 }
        : (Direction) { .row =  1, .col =  0 };

    size_t nb_moves = 0;

    switch (piece.type) {
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

static bool has_moves_available(ChessBoard board, PieceColor color) {
    for (size_t row = 0; row < 8; row++) {
        for (size_t col = 0; col < 8; col++) {
            const Position piece_position = { col, row };
            const Cell current_piece = get_piece_at(board, piece_position);

            if (current_piece.is_empty || current_piece.color != color) continue;

            Position moves_buffer[24];
            if (get_possible_moves(board, piece_position, moves_buffer) > 0)
                return true;
        }
    }

    return false;
}

KingStatus check_check(ChessBoard board, Position start, Position end) {
    // Apply move, but save the replaced piece
    const Cell moved_piece = get_piece_at(board, start);
    const Cell replaced_piece = get_piece_at(board, end);
    set_piece_at(board, end, moved_piece);
    set_piece_at(board, start, (Cell) { .is_empty = true });

    // Store return value and use goto due to needed cleanup.
    KingStatus rv = NO_CHECKS;

    Position king_position;
    const PieceColor enemy_king_color = color_to_play == WHITE ? BLACK : WHITE;

    for (size_t row = 0; row < 8; row++) {
        for (size_t col = 0; col < 8; col++) {
            const Position piece_position = { col, row };
            const Cell current_piece = get_piece_at(board, piece_position);

            if (current_piece.type == KING && current_piece.color == enemy_king_color) {
                king_position = piece_position;
                goto found_king;
            }
        }
    }

    found_king:

    for (size_t row = 0; row < 8; row++) {
        for (size_t col = 0; col < 8; col++) {
            const Position piece_position = { col, row };
            const Cell current_piece = get_piece_at(board, piece_position);

            if (current_piece.is_empty || current_piece.color == color_to_play)
                continue;

            Position move_buffer[24];
            const size_t nb_moves = get_possible_moves(board, piece_position, move_buffer);

            for (size_t i = 0; i < nb_moves; i++) {
                if (eq_positions(move_buffer[i], king_position)) {
                    if (has_moves_available(board, enemy_king_color)) {
                        rv = color_to_play == WHITE
                            ? BLACK_IN_CHECK
                            : WHITE_IN_CHECK;
                    }
                    else {
                        rv = color_to_play == WHITE
                            ? BLACK_CHECK_MATE
                            : WHITE_CHECK_MATE;
                    }
                    goto cleanup;
                }
            }
        }
    }

    cleanup:
    set_piece_at(board, end, replaced_piece);
    set_piece_at(board, start, moved_piece);
    return rv;
}

// Returns true if the status refers to the same player as the color.
static bool color_eq_king_status(PieceColor color, KingStatus status) {
    if (status == NO_CHECKS) return false;
    switch (color) {
        case WHITE: return status == WHITE_IN_CHECK || status == WHITE_CHECK_MATE;
        case BLACK: return status == BLACK_IN_CHECK || status == BLACK_CHECK_MATE;
    }
}

typedef struct {
    bool was_valid;
    KingStatus king_status;
} PlayedMoveStatus;

// NOTE: currently assumes the move is legal, but checks if it leads to a self-check
PlayedMoveStatus try_play_move(ChessBoard board, Position start, Position end) {
    KingStatus checks = check_check(board, start, end);
    bool self_check = color_eq_king_status(color_to_play, checks);

    if (!self_check) {
        set_piece_at(board, end, get_piece_at(board, start));
        set_piece_at(board, start, (Cell) { .is_empty = true });
        color_to_play = color_to_play == WHITE ? BLACK : WHITE;
    }

    return (PlayedMoveStatus) {
        .king_status = checks,
        .was_valid = !self_check,
    };
}

void debug_log_chess_board(ChessBoard board) {
    printf("+----\n");

    for (size_t row = 0; row < 8; row++) {
        printf("%c", row < 2 ? '|' : ' ');

        for (size_t col = 0; col < 8; col++) {
            Cell current_piece = get_piece_at(board, (Position) { col, row });
            if (current_piece.is_empty) {
                printf(" --");
                continue;
            }

            printf(" %c", current_piece.color == WHITE ? 'w' : 'b');
            switch (current_piece.type) {
                case PAWN:   printf("p"); break;
                case ROOK:   printf("r"); break;
                case KNIGHT: printf("h"); break;  // 'h' for "horse"
                case BISHOP: printf("b"); break;
                case QWEEN:  printf("q"); break;
                case KING:   printf("k"); break;
            }
        }

        printf(" %c\n", row >= 6 ? '|' : ' ');
    }

    printf("                      ----+\n");
}
