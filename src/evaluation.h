#ifndef EVALUATION_H
#define EVALUATION_H

#include "constants.h"
#include "board.h"

// Piece values
#define PAWN_VALUE   100
#define KNIGHT_VALUE 320
#define BISHOP_VALUE 330
#define ROOK_VALUE   500
#define QUEEN_VALUE  900
#define KING_VALUE   20000

// Evaluation weights
#define MOBILITY_WEIGHT 10
#define PAWN_STRUCTURE_WEIGHT 15
#define KING_SAFETY_WEIGHT 20
#define BISHOP_PAIR_WEIGHT 30
#define ROOK_OPEN_FILE_WEIGHT 25
#define ROOK_7TH_RANK_WEIGHT 40

// Main evaluation function
int evaluate_position(const Board* board);

// Evaluation components
int evaluate_material(const Board* board);
int evaluate_positional(const Board* board);
int evaluate_mobility(const Board* board);
int evaluate_pawn_structure(const Board* board);
int evaluate_king_safety(const Board* board);
int evaluate_bishop_pair(const Board* board);
int evaluate_rook_position(const Board* board);

// Initialize evaluation tables
void init_evaluation_tables(void);

// Piece-square tables
extern int pawn_table[64];
extern int knight_table[64];
extern int bishop_table[64];
extern int rook_table[64];
extern int queen_table[64];
extern int king_table[64];

// Mobility tables
extern int knight_mobility_bonus[9];
extern int bishop_mobility_bonus[14];
extern int rook_mobility_bonus[15];
extern int queen_mobility_bonus[28];

#endif // EVALUATION_H 