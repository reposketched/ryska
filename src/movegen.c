#include "movegen.h"
#include "bitboard.h"

// Generate all legal moves for the current position
int generate_moves(const Board* board, Move* moves) {
    int count = 0;
    
    // Generate moves for each piece type
    generate_pawn_moves(board, moves, &count);
    generate_knight_moves(board, moves, &count);
    generate_bishop_moves(board, moves, &count);
    generate_rook_moves(board, moves, &count);
    generate_queen_moves(board, moves, &count);
    generate_king_moves(board, moves, &count);
    
    return count;
}

// Generate pawn moves
int generate_pawn_moves(const Board* board, Move* moves, int* count) {
    Color color = board->side_to_move;
    Bitboard pawns = board->pieces[color][PAWN];
    Bitboard empty = board->empty;
    Bitboard enemies = board_get_all_pieces(board, color_opposite(color));
    
    while (pawns) {
        Square from = pop_lsb(&pawns);
        Bitboard attacks = pawn_attacks[color][from] & enemies;
        Bitboard pushes = 0;
        
        // Single pawn push
        if (color == WHITE) {
            if (test_bit(empty, from + 8)) {
                pushes = set_bit(pushes, from + 8);
                // Double pawn push
                if (rank_of(from) == RANK_2 && test_bit(empty, from + 16)) {
                    moves[*count] = make_move(from, (Square)(from + 16), DOUBLE_PAWN_PUSH);
                    (*count)++;
                }
            }
        } else {
            if (test_bit(empty, from - 8)) {
                pushes = set_bit(pushes, from - 8);
                // Double pawn push
                if (rank_of(from) == RANK_7 && test_bit(empty, from - 16)) {
                    moves[*count] = make_move(from, (Square)(from - 16), DOUBLE_PAWN_PUSH);
                    (*count)++;
                }
            }
        }
        
        // Add quiet moves
        while (pushes) {
            Square to = pop_lsb(&pushes);
            if ((color == WHITE && rank_of(to) == RANK_8) || 
                (color == BLACK && rank_of(to) == RANK_1)) {
                // Promotions
                moves[*count] = make_move(from, to, PROMOTION);
                (*count)++;
            } else {
                moves[*count] = make_move(from, to, QUIET);
                (*count)++;
            }
        }
        
        // Add captures
        while (attacks) {
            Square to = pop_lsb(&attacks);
            if ((color == WHITE && rank_of(to) == RANK_8) || 
                (color == BLACK && rank_of(to) == RANK_1)) {
                // Promotion captures
                moves[*count] = make_move(from, to, PROMOTION_CAPTURE);
                (*count)++;
            } else {
                moves[*count] = make_move(from, to, CAPTURE);
                (*count)++;
            }
        }
        
        // En passant
        if (board->en_passant != A1) {
            Bitboard ep_attacks = pawn_attacks[color][from] & square_bb[board->en_passant];
            if (ep_attacks) {
                moves[*count] = make_move(from, board->en_passant, EN_PASSANT);
                (*count)++;
            }
        }
    }
    
    return *count;
}

// Generate knight moves
int generate_knight_moves(const Board* board, Move* moves, int* count) {
    Color color = board->side_to_move;
    Bitboard knights = board->pieces[color][KNIGHT];
    Bitboard empty = board->empty;
    Bitboard enemies = board_get_all_pieces(board, color_opposite(color));
    
    while (knights) {
        Square from = pop_lsb(&knights);
        Bitboard attacks = knight_attacks[from] & (empty | enemies);
        
        while (attacks) {
            Square to = pop_lsb(&attacks);
            uint8_t flags = test_bit(enemies, to) ? CAPTURE : QUIET;
            moves[*count] = make_move(from, to, flags);
            (*count)++;
        }
    }
    
    return *count;
}

// Generate bishop moves (simplified - would need proper ray generation)
int generate_bishop_moves(const Board* board, Move* moves, int* count) {
    Color color = board->side_to_move;
    Bitboard bishops = board->pieces[color][BISHOP];
    Bitboard empty = board->empty;
    Bitboard enemies = board_get_all_pieces(board, color_opposite(color));
    
    while (bishops) {
        Square from = pop_lsb(&bishops);
        // Simplified bishop moves - just diagonal attacks
        Bitboard attacks = (shift_northeast(square_bb[from]) | shift_northwest(square_bb[from]) |
                          shift_southeast(square_bb[from]) | shift_southwest(square_bb[from])) & 
                          (empty | enemies);
        
        while (attacks) {
            Square to = pop_lsb(&attacks);
            uint8_t flags = test_bit(enemies, to) ? CAPTURE : QUIET;
            moves[*count] = make_move(from, to, flags);
            (*count)++;
        }
    }
    
    return *count;
}

// Generate rook moves (simplified - would need proper ray generation)
int generate_rook_moves(const Board* board, Move* moves, int* count) {
    Color color = board->side_to_move;
    Bitboard rooks = board->pieces[color][ROOK];
    Bitboard empty = board->empty;
    Bitboard enemies = board_get_all_pieces(board, color_opposite(color));
    
    while (rooks) {
        Square from = pop_lsb(&rooks);
        // Simplified rook moves - just rank and file attacks
        Bitboard attacks = (shift_north(square_bb[from]) | shift_south(square_bb[from]) |
                          shift_east(square_bb[from]) | shift_west(square_bb[from])) & 
                          (empty | enemies);
        
        while (attacks) {
            Square to = pop_lsb(&attacks);
            uint8_t flags = test_bit(enemies, to) ? CAPTURE : QUIET;
            moves[*count] = make_move(from, to, flags);
            (*count)++;
        }
    }
    
    return *count;
}

// Generate queen moves (simplified - would need proper ray generation)
int generate_queen_moves(const Board* board, Move* moves, int* count) {
    Color color = board->side_to_move;
    Bitboard queens = board->pieces[color][QUEEN];
    Bitboard empty = board->empty;
    Bitboard enemies = board_get_all_pieces(board, color_opposite(color));
    
    while (queens) {
        Square from = pop_lsb(&queens);
        // Simplified queen moves - combination of bishop and rook
        Bitboard attacks = (shift_northeast(square_bb[from]) | shift_northwest(square_bb[from]) |
                          shift_southeast(square_bb[from]) | shift_southwest(square_bb[from]) |
                          shift_north(square_bb[from]) | shift_south(square_bb[from]) |
                          shift_east(square_bb[from]) | shift_west(square_bb[from])) & 
                          (empty | enemies);
        
        while (attacks) {
            Square to = pop_lsb(&attacks);
            uint8_t flags = test_bit(enemies, to) ? CAPTURE : QUIET;
            moves[*count] = make_move(from, to, flags);
            (*count)++;
        }
    }
    
    return *count;
}

// Generate king moves
int generate_king_moves(const Board* board, Move* moves, int* count) {
    Color color = board->side_to_move;
    Bitboard kings = board->pieces[color][KING];
    Bitboard empty = board->empty;
    Bitboard enemies = board_get_all_pieces(board, color_opposite(color));
    
    while (kings) {
        Square from = pop_lsb(&kings);
        Bitboard attacks = king_attacks[from] & (empty | enemies);
        
        while (attacks) {
            Square to = pop_lsb(&attacks);
            uint8_t flags = test_bit(enemies, to) ? CAPTURE : QUIET;
            moves[*count] = make_move(from, to, flags);
            (*count)++;
        }
        
        // Castling (simplified)
        if (color == WHITE) {
            if ((board->castling_rights & WHITE_KINGSIDE) && 
                test_bit(empty, F1) && test_bit(empty, G1) &&
                test_bit(board->pieces[WHITE][ROOK], H1)) {
                moves[*count] = make_move(E1, G1, KING_CASTLE);
                (*count)++;
            }
            if ((board->castling_rights & WHITE_QUEENSIDE) && 
                test_bit(empty, D1) && test_bit(empty, C1) &&
                test_bit(board->pieces[WHITE][ROOK], A1)) {
                moves[*count] = make_move(E1, C1, QUEEN_CASTLE);
                (*count)++;
            }
        } else {
            if ((board->castling_rights & BLACK_KINGSIDE) && 
                test_bit(empty, F8) && test_bit(empty, G8) &&
                test_bit(board->pieces[BLACK][ROOK], H8)) {
                moves[*count] = make_move(E8, G8, KING_CASTLE);
                (*count)++;
            }
            if ((board->castling_rights & BLACK_QUEENSIDE) && 
                test_bit(empty, D8) && test_bit(empty, C8) &&
                test_bit(board->pieces[BLACK][ROOK], A8)) {
                moves[*count] = make_move(E8, C8, QUEEN_CASTLE);
                (*count)++;
            }
        }
    }
    
    return *count;
}

// Check if a move is legal
int is_legal_move(const Board* board, Move move) {
    // Make the move on a temporary board
    Board temp_board = *board;
    board_make_move(&temp_board, move);
    
    // Check if the move leaves the king in check
    return !board_is_check(&temp_board);
} 