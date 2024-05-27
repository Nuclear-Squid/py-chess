// vim:ft=c
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef   int8_t        i8;
typedef   int16_t       i16;
typedef   int32_t       i32;
typedef   int64_t       i64;

typedef   uint8_t       u8;
typedef   uint16_t      u16;
typedef   uint32_t      u32;
typedef   uint64_t      u64;

typedef   float         f32;
typedef   double        f64;

typedef enum: u8 { WHITE, BLACK } PieceColor;
typedef enum: u8 { PAWN, ROOK, KNIGHT, BISHOP, QWEEN, KING } PiecesType;

typedef struct {
    PieceColor color: 1;
    PiecesType type : 3;
    bool is_empty: 1;  // default value is 0, so false
} Cell;

#define EMPTY_CELL ((Cell) { .is_empty = true })

typedef Cell ChessBoard[8][8];

typedef struct {
    i8 col;
    i8 row;
} Position;
typedef Position Direction;

const Direction DIR_N = { .col =  0, .row = -1 };
const Direction DIR_S = { .col =  0, .row =  1 };
const Direction DIR_E = { .col =  1, .row =  0 };
const Direction DIR_W = { .col = -1, .row =  0 };

const Direction DIR_NE = { .col =  1, .row = -1 };
const Direction DIR_NW = { .col = -1, .row = -1 };
const Direction DIR_SE = { .col =  1, .row =  1 };
const Direction DIR_SW = { .col = -1, .row =  1 };



typedef enum: u8 { OUT_OF_BOUNDS, FREE, SAME_COLOR, OTHER_COLOR } CellState;

typedef enum { NO_CHECKS, CHECK, CHECK_MATE } KingStatus;

typedef struct {
    bool your_king_in_check;
    KingStatus ennemy_king_status;
    bool draw_match;
} PlayedMoveStatus;
