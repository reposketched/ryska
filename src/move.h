#ifndef MOVE_H
#define MOVE_H

#include "constants.h"
#include <stdint.h>

// Move representation using 16 bits
// Bits 0-5: from square
// Bits 6-11: to square  
// Bits 12-15: move flags
typedef struct {
    uint16_t data;
} Move;

// Null move
extern const Move NULL_MOVE;

// Move creation and access functions
Move make_move(Square from, Square to, uint8_t flags);
Square move_from(Move move);
Square move_to(Move move);
uint8_t move_flags(Move move);

// Move type checking
int is_capture(Move move);
int is_promotion(Move move);
int is_castle(Move move);
int is_en_passant(Move move);
int is_double_pawn_push(Move move);
PieceType promotion_piece(Move move);

// Move comparison
int move_equal(Move a, Move b);
int move_not_equal(Move a, Move b);

// Raw data access
uint16_t move_raw(Move move);

#endif // MOVE_H 