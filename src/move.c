#include "move.h"

// Null move definition
const Move NULL_MOVE = {0};

// Move creation and access functions
Move make_move(Square from, Square to, uint8_t flags) {
    Move move;
    move.data = (from & 0x3F) | ((to & 0x3F) << 6) | ((flags & 0x0F) << 12);
    return move;
}

Square move_from(Move move) {
    return (Square)(move.data & 0x3F);
}

Square move_to(Move move) {
    return (Square)((move.data >> 6) & 0x3F);
}

uint8_t move_flags(Move move) {
    return (move.data >> 12) & 0x0F;
}

// Move type checking
int is_capture(Move move) {
    return (move_flags(move) & 0x04) != 0;
}

int is_promotion(Move move) {
    return (move_flags(move) & 0x08) != 0;
}

int is_castle(Move move) {
    uint8_t flags = move_flags(move);
    return flags == KING_CASTLE || flags == QUEEN_CASTLE;
}

int is_en_passant(Move move) {
    return move_flags(move) == EN_PASSANT;
}

int is_double_pawn_push(Move move) {
    return move_flags(move) == DOUBLE_PAWN_PUSH;
}

PieceType promotion_piece(Move move) {
    if (!is_promotion(move)) return PAWN;
    // Lower two bits encode: 0=n,1=b,2=r,3=q
    switch (move_flags(move) & 0x03) {
        case 0: return KNIGHT;
        case 1: return BISHOP;
        case 2: return ROOK;
        default: return QUEEN;
    }
}

// Move comparison
int move_equal(Move a, Move b) {
    return a.data == b.data;
}

int move_not_equal(Move a, Move b) {
    return a.data != b.data;
}

// Raw data access
uint16_t move_raw(Move move) {
    return move.data;
} 