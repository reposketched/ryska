#include "movegen.h"
#include "bitboard.h"

// Generate all pseudo-legal moves for the current position
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
                // Generate 4 promotion piece choices
                for (int pc = 0; pc < 4; pc++) {
                    moves[*count] = make_move(from, to, (uint8_t)(PROMOTION | pc));
                    (*count)++;
                }
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
                for (int pc = 0; pc < 4; pc++) {
                    moves[*count] = make_move(from, to, (uint8_t)(PROMOTION_CAPTURE | pc));
                    (*count)++;
                }
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

// Generate bishop moves (sliding along diagonals)
int generate_bishop_moves(const Board* board, Move* moves, int* count) {
    Color color = board->side_to_move;
    Bitboard bishops = board->pieces[color][BISHOP];
    Bitboard enemies = board_get_all_pieces(board, color_opposite(color));
    Bitboard own = board_get_all_pieces(board, color);
    
    while (bishops) {
        Square from = pop_lsb(&bishops);
        int deltas[4] = { 9, 7, -7, -9 };
        for (int d = 0; d < 4; d++) {
            int to = (int)from + deltas[d];
            while (to >= 0 && to < 64) {
                int prev = to - deltas[d];
                int file_diff = (int)file_of((Square)to) - (int)file_of((Square)prev);
                if (file_diff != 1 && file_diff != -1) break;
                if (test_bit(own, (Square)to)) break;
                uint8_t flags = test_bit(enemies, (Square)to) ? CAPTURE : QUIET;
                moves[*count] = make_move(from, (Square)to, flags);
                (*count)++;
                if (flags == CAPTURE) break;
                to += deltas[d];
            }
        }
    }
    
    return *count;
}

// Generate rook moves (sliding along ranks/files)
int generate_rook_moves(const Board* board, Move* moves, int* count) {
    Color color = board->side_to_move;
    Bitboard rooks = board->pieces[color][ROOK];
    Bitboard enemies = board_get_all_pieces(board, color_opposite(color));
    Bitboard own = board_get_all_pieces(board, color);
    
    while (rooks) {
        Square from = pop_lsb(&rooks);
        int deltas[4] = { 8, -8, 1, -1 };
        for (int d = 0; d < 4; d++) {
            int to = (int)from + deltas[d];
            while (to >= 0 && to < 64) {
                if (d >= 2) {
                    int prev = to - deltas[d];
                    int file_diff = (int)file_of((Square)to) - (int)file_of((Square)prev);
                    if (file_diff != 1 && file_diff != -1) break;
                }
                if (test_bit(own, (Square)to)) break;
                uint8_t flags = test_bit(enemies, (Square)to) ? CAPTURE : QUIET;
                moves[*count] = make_move(from, (Square)to, flags);
                (*count)++;
                if (flags == CAPTURE) break;
                to += deltas[d];
            }
        }
    }
    
    return *count;
}

// Generate queen moves (sliding: rook + bishop)
int generate_queen_moves(const Board* board, Move* moves, int* count) {
    Color color = board->side_to_move;
    Bitboard queens = board->pieces[color][QUEEN];
    Bitboard enemies = board_get_all_pieces(board, color_opposite(color));
    Bitboard own = board_get_all_pieces(board, color);
    
    while (queens) {
        Square from = pop_lsb(&queens);
        int deltas[8] = { 9, 7, -7, -9, 8, -8, 1, -1 };
        for (int d = 0; d < 8; d++) {
            int to = (int)from + deltas[d];
            while (to >= 0 && to < 64) {
                int prev = to - deltas[d];
                // Diagonals and horizontals must step one file at a time
                if (d < 4 || d >= 6) {
                    int file_diff = (int)file_of((Square)to) - (int)file_of((Square)prev);
                    if (file_diff != 1 && file_diff != -1) break;
                }
                if (test_bit(own, (Square)to)) break;
                uint8_t flags = test_bit(enemies, (Square)to) ? CAPTURE : QUIET;
                moves[*count] = make_move(from, (Square)to, flags);
                (*count)++;
                if (flags == CAPTURE) break;
                to += deltas[d];
            }
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
        
        // Castling (with basic legality)
        if (color == WHITE) {
            if ((board->castling_rights & WHITE_KINGSIDE) && 
                test_bit(empty, F1) && test_bit(empty, G1) &&
                test_bit(board->pieces[WHITE][ROOK], H1) &&
                !board_is_square_attacked(board, E1, BLACK) &&
                !board_is_square_attacked(board, F1, BLACK) &&
                !board_is_square_attacked(board, G1, BLACK)) {
                moves[*count] = make_move(E1, G1, KING_CASTLE);
                (*count)++;
            }
            if ((board->castling_rights & WHITE_QUEENSIDE) && 
                test_bit(empty, D1) && test_bit(empty, C1) &&
                test_bit(board->pieces[WHITE][ROOK], A1) &&
                !board_is_square_attacked(board, E1, BLACK) &&
                !board_is_square_attacked(board, D1, BLACK) &&
                !board_is_square_attacked(board, C1, BLACK)) {
                moves[*count] = make_move(E1, C1, QUEEN_CASTLE);
                (*count)++;
            }
        } else {
            if ((board->castling_rights & BLACK_KINGSIDE) && 
                test_bit(empty, F8) && test_bit(empty, G8) &&
                test_bit(board->pieces[BLACK][ROOK], H8) &&
                !board_is_square_attacked(board, E8, WHITE) &&
                !board_is_square_attacked(board, F8, WHITE) &&
                !board_is_square_attacked(board, G8, WHITE)) {
                moves[*count] = make_move(E8, G8, KING_CASTLE);
                (*count)++;
            }
            if ((board->castling_rights & BLACK_QUEENSIDE) && 
                test_bit(empty, D8) && test_bit(empty, C8) &&
                test_bit(board->pieces[BLACK][ROOK], A8) &&
                !board_is_square_attacked(board, E8, WHITE) &&
                !board_is_square_attacked(board, D8, WHITE) &&
                !board_is_square_attacked(board, C8, WHITE)) {
                moves[*count] = make_move(E8, C8, QUEEN_CASTLE);
                (*count)++;
            }
        }
    }
    
    return *count;
}

// Check if a move is legal (does not leave own king in check)
int is_legal_move(const Board* board, Move move) {
    Board temp = *board;
    Color moving = board->side_to_move;
    board_make_move(&temp, move);
    Square king_sq = lsb(temp.pieces[moving][KING]);
    return !board_is_square_attacked(&temp, king_sq, color_opposite(moving));
}