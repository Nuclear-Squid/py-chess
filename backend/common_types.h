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
    u8 col;
    u8 row;
} Position;
typedef Position Direction;

typedef enum: u8 { OUT_OF_BOUNDS, FREE, SAME_COLOR, OTHER_COLOR } CellState;

typedef enum { NO_CHECKS, CHECK, CHECK_MATE } KingStatus;
