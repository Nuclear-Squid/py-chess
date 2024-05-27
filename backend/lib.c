#include <stdio.h>
#include <stdlib.h>

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

static bool white_short_can_castle = true;
static bool black_short_can_castle = true;
static bool white_long_can_castle = true;
static bool black_long_can_castle = true;

static struct {
    Cell moved_piece;
    Position start_position;
    Position end_position;
} last_move;

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

static inline Position add_positions3(Position pos_a, Position pos_b, Position pos_c) {
    return (Position) {
        .col = pos_a.col + pos_b.col + pos_c.col,
        .row = pos_a.row + pos_b.row + pos_c.row,
    };
}

static inline Position mul_position(Position pos, i8 multiplier) {
    return (Position) {
        .col = pos.col * multiplier,
        .row = pos.row * multiplier,
    };
}

static inline bool eq_cells(Cell cell_a, Cell cell_b) {
    if (cell_a.is_empty) return cell_b.is_empty;
    return cell_a.is_empty == cell_b.is_empty && cell_a.type == cell_b.type && cell_a.color == cell_b.color;
}

static inline bool eq_positions(Position pos_a, Position pos_b) {
    return pos_a.col == pos_b.col && pos_a.row == pos_b.row;
}

static inline PieceColor get_opposite_color(PieceColor color) { return color ^ 1; }

Cell get_piece_at(ChessBoard board, Position pos) {
    return board[pos.row][pos.col];
}

void set_piece_at(ChessBoard board, Position pos, Cell piece) {
    board[pos.row][pos.col] = piece;
}

static inline CellState get_cell_state(ChessBoard board, Position cell_pos, PieceColor piece_color) {
    if (cell_pos.col >= 8 || cell_pos.row >= 8) return OUT_OF_BOUNDS;
    if (cell_pos.col < 0  || cell_pos.row < 0)  return OUT_OF_BOUNDS;
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

static size_t get_pawn_moves_line(ChessBoard board, Position pos, Position* output, PieceColor piece_color) {
    Direction pawn_direction = piece_color == WHITE ? DIR_N : DIR_S;
    size_t nb_moves = 0;

    Position aimed_cell = add_positions(pos, pawn_direction);
    if (get_cell_state(board, aimed_cell, piece_color) != FREE) return 0;
    output[nb_moves++] = aimed_cell;

    if (pos.row == 1 || pos.row == 6) {
        aimed_cell = add_positions(aimed_cell, pawn_direction);
        if (get_cell_state(board, aimed_cell, piece_color) == FREE) {
            output[nb_moves++] = aimed_cell;
        }
    }

    return nb_moves;
}

static size_t get_pawn_diagonal_if_valid(ChessBoard board, Position pos, Position* output, PieceColor piece_color, Direction horizontal) {
    Direction pawn_direction = piece_color == WHITE ? DIR_N : DIR_S;
    Position aimed_cell = add_positions3(pos, pawn_direction, horizontal);

    if (get_cell_state(board, aimed_cell, piece_color) == OTHER_COLOR) {
        *output = aimed_cell;
        return 1;
    }

    // En passant
    if (last_move.moved_piece.type != PAWN) return 0;
    if (abs(last_move.end_position.row - last_move.start_position.row) != 2) return 0;

    if (eq_positions(last_move.end_position, add_positions(pos, horizontal))) {
        *output = aimed_cell;
        return 1;
    }
    return 0;
}

static size_t get_possible_moves_pawn(ChessBoard board, Position pos, Position* output, PieceColor piece_color) {
    size_t nb_moves = get_pawn_moves_line(board, pos, output, piece_color);
    nb_moves += get_pawn_diagonal_if_valid(board, pos, output + nb_moves, piece_color, DIR_W);
    nb_moves += get_pawn_diagonal_if_valid(board, pos, output + nb_moves, piece_color, DIR_E);
    return nb_moves;
}

static size_t get_possible_moves_rook(ChessBoard board, Position pos, Position* output, PieceColor piece_color) {
    size_t nb_moves = 0;
    nb_moves += get_moves_line(board, pos, DIR_W, piece_color, output + nb_moves);
    nb_moves += get_moves_line(board, pos, DIR_E, piece_color, output + nb_moves);
    nb_moves += get_moves_line(board, pos, DIR_N, piece_color, output + nb_moves);
    nb_moves += get_moves_line(board, pos, DIR_S, piece_color, output + nb_moves);
    return nb_moves;
}

static size_t get_possible_moves_knight(ChessBoard board, Position pos, Position* output, PieceColor piece_color) {
    size_t nb_moves = 0;
    nb_moves += get_move_if_valid(board, pos, (Direction) { .row =  1, .col =  2 }, piece_color, output + nb_moves);
    nb_moves += get_move_if_valid(board, pos, (Direction) { .row = -1, .col =  2 }, piece_color, output + nb_moves);
    nb_moves += get_move_if_valid(board, pos, (Direction) { .row =  1, .col = -2 }, piece_color, output + nb_moves);
    nb_moves += get_move_if_valid(board, pos, (Direction) { .row = -1, .col = -2 }, piece_color, output + nb_moves);
    nb_moves += get_move_if_valid(board, pos, (Direction) { .row =  2, .col =  1 }, piece_color, output + nb_moves);
    nb_moves += get_move_if_valid(board, pos, (Direction) { .row =  2, .col = -1 }, piece_color, output + nb_moves);
    nb_moves += get_move_if_valid(board, pos, (Direction) { .row = -2, .col =  1 }, piece_color, output + nb_moves);
    nb_moves += get_move_if_valid(board, pos, (Direction) { .row = -2, .col = -1 }, piece_color, output + nb_moves);

    return nb_moves;
}

static size_t get_possible_moves_bishop(ChessBoard board, Position pos, Position* output, PieceColor piece_color) {
    size_t nb_moves = 0;
    nb_moves += get_moves_line(board, pos, DIR_SE, piece_color, output + nb_moves);
    nb_moves += get_moves_line(board, pos, DIR_SW, piece_color, output + nb_moves);
    nb_moves += get_moves_line(board, pos, DIR_NE, piece_color, output + nb_moves);
    nb_moves += get_moves_line(board, pos, DIR_NW, piece_color, output + nb_moves);
    return nb_moves;
}

static size_t get_possible_moves_qween(ChessBoard board, Position pos, Position* output, PieceColor piece_color) {
    size_t nb_moves = 0;
    nb_moves += get_moves_line(board, pos, DIR_W, piece_color, output + nb_moves);
    nb_moves += get_moves_line(board, pos, DIR_E, piece_color, output + nb_moves);
    nb_moves += get_moves_line(board, pos, DIR_N, piece_color, output + nb_moves);
    nb_moves += get_moves_line(board, pos, DIR_S, piece_color, output + nb_moves);
    nb_moves += get_moves_line(board, pos, DIR_SE, piece_color, output + nb_moves);
    nb_moves += get_moves_line(board, pos, DIR_SW, piece_color, output + nb_moves);
    nb_moves += get_moves_line(board, pos, DIR_NE, piece_color, output + nb_moves);
    nb_moves += get_moves_line(board, pos, DIR_NW, piece_color, output + nb_moves);
    return nb_moves;
}

static size_t get_possible_moves_king(ChessBoard board, Position pos, Position* output, PieceColor piece_color) {
    size_t nb_moves = 0;
    nb_moves += get_move_if_valid(board, pos, DIR_W, piece_color, output + nb_moves);
    nb_moves += get_move_if_valid(board, pos, DIR_E, piece_color, output + nb_moves);
    nb_moves += get_move_if_valid(board, pos, DIR_N, piece_color, output + nb_moves);
    nb_moves += get_move_if_valid(board, pos, DIR_S, piece_color, output + nb_moves);
    nb_moves += get_move_if_valid(board, pos, DIR_SE, piece_color, output + nb_moves);
    nb_moves += get_move_if_valid(board, pos, DIR_SW, piece_color, output + nb_moves);
    nb_moves += get_move_if_valid(board, pos, DIR_NE, piece_color, output + nb_moves);
    nb_moves += get_move_if_valid(board, pos, DIR_NW, piece_color, output + nb_moves);

    bool long_castle_available = true;
    bool can_long_castle = piece_color == WHITE
        ? white_long_can_castle
        : black_long_can_castle;

    for (size_t col = 1; col < pos.col; col++) {
        const Position aimed_cell = { .col = col, .row = pos.row };
        if (get_cell_state(board, aimed_cell, piece_color) != FREE) {
            long_castle_available = false;
            break;
        }
    }

    if (long_castle_available && can_long_castle)
        output[nb_moves++] = (Position) { .col = 1, .row = pos.row };

    bool short_castle_available = true;
    bool can_short_castle = piece_color == WHITE
        ? white_short_can_castle
        : black_short_can_castle;

    for (size_t col = pos.col + 1; col < 7; col++) {
        const Position aimed_cell = { .col = col, .row = pos.row };
        if (get_cell_state(board, aimed_cell, piece_color) != FREE) {
            short_castle_available = false;
            break;
        }
    }

    if (short_castle_available && can_short_castle)
        output[nb_moves++] = (Position) { .col = 6, .row = pos.row };

    return nb_moves;
}

typedef size_t (*MovesGetter)(ChessBoard, Position, Position*, PieceColor);

static MovesGetter get_moves_getter(PiecesType piece_type) {
    static const MovesGetter move_getters[] = {
        [PAWN]   = get_possible_moves_pawn,
        [ROOK]   = get_possible_moves_rook,
        [BISHOP] = get_possible_moves_bishop,
        [KNIGHT] = get_possible_moves_knight,
        [QWEEN]  = get_possible_moves_qween,
        [KING]   = get_possible_moves_king,
    };

    return move_getters[piece_type];
}

size_t get_possible_moves(ChessBoard board, Position pos, Position* output) {
    const Cell piece = get_piece_at(board, pos);
    if (piece.is_empty) return 0;
    return get_moves_getter(piece.type)(board, pos, output, piece.color);
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

Position find_cell(ChessBoard board, Cell cell) {
    for (size_t row = 0; row < 8; row++) {
        for (size_t col = 0; col < 8; col++) {
            const Position piece_position = { col, row };
            const Cell current_cell = get_piece_at(board, piece_position);
            if (current_cell.is_empty) continue;
            if (eq_cells(current_cell, cell)) return piece_position;
        }
    }

    exit(1);
}

static bool is_in_check_by(ChessBoard board, Position king_position, PieceColor king_color, PiecesType piece_type) {
    const Cell template = { get_opposite_color(king_color), piece_type };
    Position moves_buffer[24];
    MovesGetter moves_getter = get_moves_getter(piece_type);
    size_t nb_moves = moves_getter(board, king_position, moves_buffer, king_color);

    for (size_t i = 0; i < nb_moves; i++) {
        const Cell current_cell = get_piece_at(board, moves_buffer[i]);
        if (eq_cells(template, current_cell)) {
            return true;
        }
    }

    return false;
}

bool is_in_check(ChessBoard board, PieceColor king_color) {
    const Position king_position = find_cell(board, (Cell) { king_color, KING });
    if (is_in_check_by(board, king_position, king_color, PAWN)) return true;
    if (is_in_check_by(board, king_position, king_color, ROOK)) return true;
    if (is_in_check_by(board, king_position, king_color, KNIGHT)) return true;
    if (is_in_check_by(board, king_position, king_color, BISHOP)) return true;
    if (is_in_check_by(board, king_position, king_color, QWEEN)) return true;
    if (is_in_check_by(board, king_position, king_color, KING)) return true;
    return false;
}


// NOTE: currently assumes the move is legal, but checks if it leads to a self-check
PlayedMoveStatus try_play_move(ChessBoard board, Position start, Position end) {
    const Cell original_piece_at_end = get_piece_at(board, end);
    const Cell moved_piece = get_piece_at(board, start);
    set_piece_at(board, end, moved_piece);
    set_piece_at(board, start, EMPTY_CELL);

    // Prevent putting yourself in check, and undo move if you did
    bool your_king_in_check = is_in_check(board, color_to_play);
    if (your_king_in_check) {
        set_piece_at(board, start, moved_piece);
        set_piece_at(board, end, original_piece_at_end);
        return (PlayedMoveStatus) { true, NO_CHECKS };
    }

    // Casteling
    bool* long_castle = color_to_play == WHITE
        ? &white_long_can_castle
        : &black_long_can_castle;

    bool* short_castle = color_to_play == WHITE
        ? &white_short_can_castle
        : &black_short_can_castle;

    if (moved_piece.type == KING && end.col == 1 && *long_castle) {
        const Position corner = { .row = end.row, .col = 0 };
        const Position new_pos_rook = { .row = end.row, .col = 2 };
        const Cell rook = { .color = color_to_play, .type = ROOK };

        set_piece_at(board, corner, EMPTY_CELL);
        set_piece_at(board, new_pos_rook, rook);
        *long_castle = false;
        *short_castle = false;
    }

    if (moved_piece.type == KING && end.col == 6 && *short_castle) {
        const Position corner = { .row = end.row, .col = 7 };
        const Position new_pos_rook = { .row = end.row, .col = 5 };
        const Cell rook = { .color = color_to_play, .type = ROOK };

        set_piece_at(board, corner, EMPTY_CELL);
        set_piece_at(board, new_pos_rook, rook);
        *long_castle = false;
        *short_castle = false;
    }

    if (moved_piece.type == ROOK && start.col == 0) *long_castle = false;
    if (moved_piece.type == ROOK && start.col == 7) *short_castle = false;

    // Promotion
    if (moved_piece.type == PAWN && (end.row == 0 || end.row == 7)) {
        const Cell qween = { moved_piece.color, QWEEN };
        set_piece_at(board, end, qween);
    }

    // En passant
    if (last_move.moved_piece.type == PAWN &&
        abs(last_move.start_position.row - last_move.end_position.row) == 2 &&
        last_move.start_position.col == end.col &&
        (last_move.start_position.row + last_move.end_position.row) / 2 == end.row
    ) {
        set_piece_at(board, last_move.end_position, EMPTY_CELL);
    }

    PieceColor enemy_color = get_opposite_color(color_to_play);
    bool enemy_king_in_check = is_in_check(board, enemy_color);

    last_move.moved_piece = moved_piece;
    last_move.start_position = start;
    last_move.end_position = end;

    color_to_play = get_opposite_color(color_to_play);

    if (enemy_king_in_check)
        return has_moves_available(board, enemy_color)
            ? (PlayedMoveStatus) { false, CHECK }
            : (PlayedMoveStatus) { false, CHECK_MATE };

    return (PlayedMoveStatus) { false, NO_CHECKS };
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
