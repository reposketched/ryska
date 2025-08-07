#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "constants.h"
#include "board.h"
#include "move.h"

// Move generation functions
int generate_moves(const Board* board, Move* moves);
int generate_pawn_moves(const Board* board, Move* moves, int* count);
int generate_knight_moves(const Board* board, Move* moves, int* count);
int generate_bishop_moves(const Board* board, Move* moves, int* count);
int generate_rook_moves(const Board* board, Move* moves, int* count);
int generate_queen_moves(const Board* board, Move* moves, int* count);
int generate_king_moves(const Board* board, Move* moves, int* count);

// Move validation
int is_legal_move(const Board* board, Move move);

#endif // MOVEGEN_H 