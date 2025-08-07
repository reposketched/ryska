#ifndef BITBOARD_H
#define BITBOARD_H

#include "constants.h"
#include <stdint.h>

// Bitboard type
typedef uint64_t Bitboard;

// Bitboard operations
Bitboard set_bit(Bitboard bb, Square sq);
Bitboard clear_bit(Bitboard bb, Square sq);
int test_bit(Bitboard bb, Square sq);
int pop_count(Bitboard bb);
Square lsb(Bitboard bb);
Square msb(Bitboard bb);
Square pop_lsb(Bitboard* bb);

// Bitboard shifts
Bitboard shift_north(Bitboard bb);
Bitboard shift_south(Bitboard bb);
Bitboard shift_east(Bitboard bb);
Bitboard shift_west(Bitboard bb);
Bitboard shift_northeast(Bitboard bb);
Bitboard shift_northwest(Bitboard bb);
Bitboard shift_southeast(Bitboard bb);
Bitboard shift_southwest(Bitboard bb);

// Pre-computed bitboards
extern Bitboard rank_bb[8];
extern Bitboard file_bb[8];
extern Bitboard square_bb[64];
extern Bitboard knight_attacks[64];
extern Bitboard king_attacks[64];
extern Bitboard pawn_attacks[2][64];

// Initialization
void init_bitboards(void);

#endif // BITBOARD_H 