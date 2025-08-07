#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdint.h>

// Piece types
typedef enum {
    PAWN = 0,
    KNIGHT = 1,
    BISHOP = 2,
    ROOK = 3,
    QUEEN = 4,
    KING = 5,
    PIECE_COUNT = 6
} PieceType;

// Colors
typedef enum {
    WHITE = 0,
    BLACK = 1,
    COLOR_COUNT = 2
} Color;

// Squares
typedef enum {
    A1 = 0, B1 = 1, C1 = 2, D1 = 3, E1 = 4, F1 = 5, G1 = 6, H1 = 7,
    A2 = 8, B2 = 9, C2 = 10, D2 = 11, E2 = 12, F2 = 13, G2 = 14, H2 = 15,
    A3 = 16, B3 = 17, C3 = 18, D3 = 19, E3 = 20, F3 = 21, G3 = 22, H3 = 23,
    A4 = 24, B4 = 25, C4 = 26, D4 = 27, E4 = 28, F4 = 29, G4 = 30, H4 = 31,
    A5 = 32, B5 = 33, C5 = 34, D5 = 35, E5 = 36, F5 = 37, G5 = 38, H5 = 39,
    A6 = 40, B6 = 41, C6 = 42, D6 = 43, E6 = 44, F6 = 45, G6 = 46, H6 = 47,
    A7 = 48, B7 = 49, C7 = 50, D7 = 51, E7 = 52, F7 = 53, G7 = 54, H7 = 55,
    A8 = 56, B8 = 57, C8 = 58, D8 = 59, E8 = 60, F8 = 61, G8 = 62, H8 = 63,
    SQUARE_COUNT = 64
} Square;

// Files and ranks
typedef enum {
    FILE_A = 0, FILE_B = 1, FILE_C = 2, FILE_D = 3,
    FILE_E = 4, FILE_F = 5, FILE_G = 6, FILE_H = 7
} File;

typedef enum {
    RANK_1 = 0, RANK_2 = 1, RANK_3 = 2, RANK_4 = 3,
    RANK_5 = 4, RANK_6 = 5, RANK_7 = 6, RANK_8 = 7
} Rank;

// Move flags
typedef enum {
    QUIET = 0,
    DOUBLE_PAWN_PUSH = 1,
    KING_CASTLE = 2,
    QUEEN_CASTLE = 3,
    CAPTURE = 4,
    EN_PASSANT = 5,
    PROMOTION = 8,
    PROMOTION_CAPTURE = 12
} MoveFlag;

// Bitboard constants
#define RANK_1_BB 0x00000000000000FFULL
#define RANK_2_BB 0x000000000000FF00ULL
#define RANK_3_BB 0x0000000000FF0000ULL
#define RANK_4_BB 0x00000000FF000000ULL
#define RANK_5_BB 0x000000FF00000000ULL
#define RANK_6_BB 0x0000FF0000000000ULL
#define RANK_7_BB 0x00FF000000000000ULL
#define RANK_8_BB 0xFF00000000000000ULL

#define FILE_A_BB 0x0101010101010101ULL
#define FILE_B_BB 0x0202020202020202ULL
#define FILE_C_BB 0x0404040404040404ULL
#define FILE_D_BB 0x0808080808080808ULL
#define FILE_E_BB 0x1010101010101010ULL
#define FILE_F_BB 0x2020202020202020ULL
#define FILE_G_BB 0x4040404040404040ULL
#define FILE_H_BB 0x8080808080808080ULL

// Utility functions
static inline Square make_square(File f, Rank r) {
    return (Square)(r * 8 + f);
}

static inline File file_of(Square s) {
    return (File)(s & 7);
}

static inline Rank rank_of(Square s) {
    return (Rank)(s >> 3);
}

static inline int is_valid_square(Square s) {
    return s >= 0 && s < SQUARE_COUNT;
}

static inline Color color_opposite(Color c) {
    return (Color)(c ^ 1);
}

#endif // CONSTANTS_H 