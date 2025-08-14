#include "bitboard.h"

// Pre-computed bitboards
Bitboard rank_bb[8];
Bitboard file_bb[8];
Bitboard square_bb[64];
Bitboard knight_attacks[64];
Bitboard king_attacks[64];
Bitboard pawn_attacks[2][64];

// Bitboard operations
Bitboard set_bit(Bitboard bb, Square sq) {
    return bb | (1ULL << sq);
}

Bitboard clear_bit(Bitboard bb, Square sq) {
    return bb & ~(1ULL << sq);
}

int test_bit(Bitboard bb, Square sq) {
    return (bb & (1ULL << sq)) != 0;
}

int pop_count(Bitboard bb) {
    int count = 0;
    while (bb) {
        count++;
        bb &= bb - 1; // Clear least significant bit
    }
    return count;
}

Square lsb(Bitboard bb) {
    if (bb == 0) return A1; // Return A1 as default
    return (Square)__builtin_ctzll(bb);
}

Square msb(Bitboard bb) {
    if (bb == 0) return A1; // Return A1 as default
    return (Square)(63 - __builtin_clzll(bb));
}

Square pop_lsb(Bitboard* bb) {
    Square sq = lsb(*bb);
    *bb &= *bb - 1; // Clear least significant bit
    return sq;
}

// Bitboard shifts
Bitboard shift_north(Bitboard bb) {
    return bb << 8;
}

Bitboard shift_south(Bitboard bb) {
    return bb >> 8;
}

Bitboard shift_east(Bitboard bb) {
    return (bb << 1) & ~FILE_A_BB;
}

Bitboard shift_west(Bitboard bb) {
    return (bb >> 1) & ~FILE_H_BB;
}

Bitboard shift_northeast(Bitboard bb) {
    return (bb << 9) & ~FILE_A_BB;
}

Bitboard shift_northwest(Bitboard bb) {
    return (bb << 7) & ~FILE_H_BB;
}

Bitboard shift_southeast(Bitboard bb) {
    return (bb >> 7) & ~FILE_A_BB;
}

Bitboard shift_southwest(Bitboard bb) {
    return (bb >> 9) & ~FILE_H_BB;
}

// Initialize pre-computed bitboards
void init_bitboards(void) {
    // Initialize rank and file bitboards
    rank_bb[0] = RANK_1_BB;
    rank_bb[1] = RANK_2_BB;
    rank_bb[2] = RANK_3_BB;
    rank_bb[3] = RANK_4_BB;
    rank_bb[4] = RANK_5_BB;
    rank_bb[5] = RANK_6_BB;
    rank_bb[6] = RANK_7_BB;
    rank_bb[7] = RANK_8_BB;

    file_bb[0] = FILE_A_BB;
    file_bb[1] = FILE_B_BB;
    file_bb[2] = FILE_C_BB;
    file_bb[3] = FILE_D_BB;
    file_bb[4] = FILE_E_BB;
    file_bb[5] = FILE_F_BB;
    file_bb[6] = FILE_G_BB;
    file_bb[7] = FILE_H_BB;

    // Initialize square bitboards
    for (int i = 0; i < 64; i++) {
        square_bb[i] = 1ULL << i;
    }

    // Initialize knight attacks
    for (Square sq = A1; sq < SQUARE_COUNT; sq++) {
        Bitboard bb = square_bb[sq];
        knight_attacks[sq] = 
            shift_north(shift_northeast(bb)) | shift_north(shift_northwest(bb)) |  // 2 up, 1 left/right
            shift_south(shift_southeast(bb)) | shift_south(shift_southwest(bb)) |  // 2 down, 1 left/right
            shift_east(shift_northeast(bb)) | shift_east(shift_southeast(bb)) |    // 2 right, 1 up/down
            shift_west(shift_northwest(bb)) | shift_west(shift_southwest(bb));     // 2 left, 1 up/down
    }

    // Initialize king attacks
    for (Square sq = A1; sq < SQUARE_COUNT; sq++) {
        Bitboard bb = square_bb[sq];
        king_attacks[sq] = 
            shift_north(bb) | shift_south(bb) | shift_east(bb) | shift_west(bb) |
            shift_northeast(bb) | shift_northwest(bb) | shift_southeast(bb) | shift_southwest(bb);
    }

    // Initialize pawn attacks
    for (Square sq = A1; sq < SQUARE_COUNT; sq++) {
        Bitboard bb = square_bb[sq];
        // White pawn attacks (moving up)
        pawn_attacks[WHITE][sq] = shift_northeast(bb) | shift_northwest(bb);
        // Black pawn attacks (moving down)
        pawn_attacks[BLACK][sq] = shift_southeast(bb) | shift_southwest(bb);
    }
} 