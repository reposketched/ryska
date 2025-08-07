#include "evaluation.h"
#include "bitboard.h"
#include "movegen.h"

// Piece-square tables
int pawn_table[64];
int knight_table[64];
int bishop_table[64];
int rook_table[64];
int queen_table[64];
int king_table[64];

// Mobility tables
int knight_mobility_bonus[9];
int bishop_mobility_bonus[14];
int rook_mobility_bonus[15];
int queen_mobility_bonus[28];

// Initialize evaluation tables
void init_evaluation_tables(void) {
    // Pawn table - encourage center pawns and pawn advancement
    for (int i = 0; i < 64; i++) {
        Rank r = rank_of(i);
        File f = file_of(i);
        int bonus = 0;
        
        // Center bonus
        if (f >= FILE_C && f <= FILE_F) bonus += 10;
        
        // Advancement bonus
        if (r >= RANK_4 && r <= RANK_6) bonus += 5;
        if (r >= RANK_5) bonus += 10;
        
        pawn_table[i] = bonus;
    }
    
    // Knight table - encourage knights in the center
    for (int i = 0; i < 64; i++) {
        Rank r = rank_of(i);
        File f = file_of(i);
        int bonus = 0;
        
        // Center bonus
        if (r >= RANK_3 && r <= RANK_6 && f >= FILE_C && f <= FILE_F) bonus += 10;
        
        // Edge penalty
        if (r == RANK_1 || r == RANK_8 || f == FILE_A || f == FILE_H) bonus -= 5;
        
        knight_table[i] = bonus;
    }
    
    // Bishop table - encourage bishops on long diagonals
    for (int i = 0; i < 64; i++) {
        Rank r = rank_of(i);
        File f = file_of(i);
        int bonus = 0;
        
        // Center bonus
        if (r >= RANK_3 && r <= RANK_6 && f >= FILE_C && f <= FILE_F) bonus += 5;
        
        bishop_table[i] = bonus;
    }
    
    // Rook table - encourage rooks on open files and 7th rank
    for (int i = 0; i < 64; i++) {
        Rank r = rank_of(i);
        File f = file_of(i);
        int bonus = 0;
        
        // 7th rank bonus
        if (r == RANK_7) bonus += 20;
        
        // Center files bonus
        if (f >= FILE_C && f <= FILE_F) bonus += 5;
        
        rook_table[i] = bonus;
    }
    
    // Queen table - encourage queen in the center
    for (int i = 0; i < 64; i++) {
        Rank r = rank_of(i);
        File f = file_of(i);
        int bonus = 0;
        
        // Center bonus
        if (r >= RANK_3 && r <= RANK_6 && f >= FILE_C && f <= FILE_F) bonus += 5;
        
        queen_table[i] = bonus;
    }
    
    // King table - encourage king safety
    for (int i = 0; i < 64; i++) {
        Rank r = rank_of(i);
        File f = file_of(i);
        int bonus = 0;
        
        // Center penalty (king should stay safe)
        if (r >= RANK_3 && r <= RANK_6 && f >= FILE_C && f <= FILE_F) bonus -= 10;
        
        // Corner bonus (castled position)
        if ((r == RANK_1 && f == FILE_G) || (r == RANK_1 && f == FILE_C) ||
            (r == RANK_8 && f == FILE_G) || (r == RANK_8 && f == FILE_C)) bonus += 20;
        
        king_table[i] = bonus;
    }
    
    // Initialize mobility tables
    for (int i = 0; i < 9; i++) {
        knight_mobility_bonus[i] = i * 3;
    }
    
    for (int i = 0; i < 14; i++) {
        bishop_mobility_bonus[i] = i * 2;
    }
    
    for (int i = 0; i < 15; i++) {
        rook_mobility_bonus[i] = i * 2;
    }
    
    for (int i = 0; i < 28; i++) {
        queen_mobility_bonus[i] = i;
    }
}

// Evaluate material balance
int evaluate_material(const Board* board) {
    int score = 0;
    
    for (Color c = WHITE; c < COLOR_COUNT; c++) {
        int color_multiplier = (c == WHITE) ? 1 : -1;
        
        // Count pieces
        int pawns = pop_count(board->pieces[c][PAWN]);
        int knights = pop_count(board->pieces[c][KNIGHT]);
        int bishops = pop_count(board->pieces[c][BISHOP]);
        int rooks = pop_count(board->pieces[c][ROOK]);
        int queens = pop_count(board->pieces[c][QUEEN]);
        int kings = pop_count(board->pieces[c][KING]);
        
        score += color_multiplier * (
            pawns * PAWN_VALUE +
            knights * KNIGHT_VALUE +
            bishops * BISHOP_VALUE +
            rooks * ROOK_VALUE +
            queens * QUEEN_VALUE +
            kings * KING_VALUE
        );
    }
    
    return score;
}

// Evaluate positional factors
int evaluate_positional(const Board* board) {
    int score = 0;
    
    for (Color c = WHITE; c < COLOR_COUNT; c++) {
        int color_multiplier = (c == WHITE) ? 1 : -1;
        
        // Evaluate pawn positions
        Bitboard pawns = board->pieces[c][PAWN];
        while (pawns) {
            Square sq = pop_lsb(&pawns);
            score += color_multiplier * pawn_table[sq];
        }
        
        // Evaluate knight positions
        Bitboard knights = board->pieces[c][KNIGHT];
        while (knights) {
            Square sq = pop_lsb(&knights);
            score += color_multiplier * knight_table[sq];
        }
        
        // Evaluate bishop positions
        Bitboard bishops = board->pieces[c][BISHOP];
        while (bishops) {
            Square sq = pop_lsb(&bishops);
            score += color_multiplier * bishop_table[sq];
        }
        
        // Evaluate rook positions
        Bitboard rooks = board->pieces[c][ROOK];
        while (rooks) {
            Square sq = pop_lsb(&rooks);
            score += color_multiplier * rook_table[sq];
        }
        
        // Evaluate queen positions
        Bitboard queens = board->pieces[c][QUEEN];
        while (queens) {
            Square sq = pop_lsb(&queens);
            score += color_multiplier * queen_table[sq];
        }
        
        // Evaluate king positions
        Bitboard kings = board->pieces[c][KING];
        while (kings) {
            Square sq = pop_lsb(&kings);
            score += color_multiplier * king_table[sq];
        }
    }
    
    return score;
}

// Evaluate piece mobility
int evaluate_mobility(const Board* board) {
    int score = 0;
    
    for (Color c = WHITE; c < COLOR_COUNT; c++) {
        int color_multiplier = (c == WHITE) ? 1 : -1;
        
        // Count knight moves
        Bitboard knights = board->pieces[c][KNIGHT];
        while (knights) {
            Square sq = pop_lsb(&knights);
            Bitboard attacks = knight_attacks[sq] & ~board->pieces[c][0] & ~board->pieces[c][1] & ~board->pieces[c][2] & ~board->pieces[c][3] & ~board->pieces[c][4] & ~board->pieces[c][5];
            int mobility = pop_count(attacks);
            if (mobility < 9) {
                score += color_multiplier * knight_mobility_bonus[mobility];
            }
        }
        
        // Count bishop moves (simplified)
        Bitboard bishops = board->pieces[c][BISHOP];
        while (bishops) {
            Square sq = pop_lsb(&bishops);
            // Simplified bishop mobility calculation
            int mobility = 8; // Approximate
            if (mobility < 14) {
                score += color_multiplier * bishop_mobility_bonus[mobility];
            }
        }
    }
    
    return score * MOBILITY_WEIGHT;
}

// Evaluate pawn structure
int evaluate_pawn_structure(const Board* board) {
    int score = 0;
    
    for (Color c = WHITE; c < COLOR_COUNT; c++) {
        int color_multiplier = (c == WHITE) ? 1 : -1;
        Bitboard pawns = board->pieces[c][PAWN];
        
        // Doubled pawns penalty
        for (File f = FILE_A; f < FILE_H + 1; f++) {
            Bitboard file_pawns = pawns & file_bb[f];
            int pawn_count = pop_count(file_pawns);
            if (pawn_count > 1) {
                score -= color_multiplier * (pawn_count - 1) * 10;
            }
        }
        
        // Isolated pawns penalty
        for (File f = FILE_A; f < FILE_H + 1; f++) {
            Bitboard file_pawns = pawns & file_bb[f];
            if (file_pawns) {
                Bitboard adjacent_files = 0;
                if (f > FILE_A) adjacent_files |= file_bb[f - 1];
                if (f < FILE_H) adjacent_files |= file_bb[f + 1];
                
                if (!(pawns & adjacent_files)) {
                    score -= color_multiplier * 15;
                }
            }
        }
    }
    
    return score * PAWN_STRUCTURE_WEIGHT;
}

// Evaluate king safety
int evaluate_king_safety(const Board* board) {
    int score = 0;
    
    for (Color c = WHITE; c < COLOR_COUNT; c++) {
        int color_multiplier = (c == WHITE) ? 1 : -1;
        Bitboard king = board->pieces[c][KING];
        
        if (king) {
            Square king_sq = lsb(king);
            Rank r = rank_of(king_sq);
            File f = file_of(king_sq);
            
            // Penalty for king in center during middlegame
            if (r >= RANK_3 && r <= RANK_6 && f >= FILE_C && f <= FILE_F) {
                score -= color_multiplier * 20;
            }
            
            // Bonus for castled king
            if ((r == RANK_1 && (f == FILE_G || f == FILE_C)) ||
                (r == RANK_8 && (f == FILE_G || f == FILE_C))) {
                score += color_multiplier * 30;
            }
        }
    }
    
    return score * KING_SAFETY_WEIGHT;
}

// Evaluate bishop pair
int evaluate_bishop_pair(const Board* board) {
    int score = 0;
    
    for (Color c = WHITE; c < COLOR_COUNT; c++) {
        int color_multiplier = (c == WHITE) ? 1 : -1;
        int bishop_count = pop_count(board->pieces[c][BISHOP]);
        
        if (bishop_count >= 2) {
            score += color_multiplier * BISHOP_PAIR_WEIGHT;
        }
    }
    
    return score;
}

// Evaluate rook position
int evaluate_rook_position(const Board* board) {
    int score = 0;
    
    for (Color c = WHITE; c < COLOR_COUNT; c++) {
        int color_multiplier = (c == WHITE) ? 1 : -1;
        Bitboard rooks = board->pieces[c][ROOK];
        
        while (rooks) {
            Square sq = pop_lsb(&rooks);
            Rank r = rank_of(sq);
            File f = file_of(sq);
            
            // Bonus for rook on 7th rank
            if ((c == WHITE && r == RANK_7) || (c == BLACK && r == RANK_2)) {
                score += color_multiplier * ROOK_7TH_RANK_WEIGHT;
            }
            
            // Bonus for rook on open/semi-open files
            Bitboard file_pawns = board->pieces[WHITE][PAWN] | board->pieces[BLACK][PAWN];
            Bitboard this_file = file_bb[f];
            if (!(file_pawns & this_file)) {
                score += color_multiplier * ROOK_OPEN_FILE_WEIGHT;
            }
        }
    }
    
    return score;
}

// Main evaluation function
int evaluate_position(const Board* board) {
    int material_score = evaluate_material(board);
    int positional_score = evaluate_positional(board);
    int mobility_score = evaluate_mobility(board);
    int pawn_structure_score = evaluate_pawn_structure(board);
    int king_safety_score = evaluate_king_safety(board);
    int bishop_pair_score = evaluate_bishop_pair(board);
    int rook_position_score = evaluate_rook_position(board);
    
    return material_score + positional_score + mobility_score + 
           pawn_structure_score + king_safety_score + bishop_pair_score + 
           rook_position_score;
} 