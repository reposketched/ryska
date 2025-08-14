#include "movegen.h"
#include "bitboard.h"
#include <stdlib.h>

// Helper function to check if a square is on the board and reachable
static inline int is_valid_destination(Square from, Square to, int delta) {
    if (to < 0 || to >= 64) return 0;
    
    // For diagonal and horizontal moves, check file distance
    if (delta == 9 || delta == 7 || delta == -7 || delta == -9) {
        // Diagonal moves: file distance must be exactly 1
        int file_diff = abs((int)file_of(to) - (int)file_of(from));
        return file_diff == 1;
    } else if (delta == 1 || delta == -1) {
        // Horizontal moves: must stay on same rank
        return rank_of(to) == rank_of(from);
    }
    return 1; // Vertical moves (delta ±8) are always valid if within bounds
}

// Generate all legal moves for the current position (not pseudo-legal)
int generate_moves(const Board* board, Move* moves) {
    int count = 0;
    Move pseudo_moves[256];
    int pseudo_count = 0;
    
    // Generate all pseudo-legal moves
    generate_pawn_moves(board, pseudo_moves, &pseudo_count);
    generate_knight_moves(board, pseudo_moves, &pseudo_count);
    generate_bishop_moves(board, pseudo_moves, &pseudo_count);
    generate_rook_moves(board, pseudo_moves, &pseudo_count);
    generate_queen_moves(board, pseudo_moves, &pseudo_count);
    generate_king_moves(board, pseudo_moves, &pseudo_count);
    
    // Filter out illegal moves (moves that leave king in check)
    for (int i = 0; i < pseudo_count; i++) {
        if (is_legal_move(board, pseudo_moves[i])) {
            moves[count++] = pseudo_moves[i];
        }
    }
    
    return count;
}

// Generate all legal moves - alias for generate_moves for consistency
int generate_legal_moves(const Board* board, Move* moves) {
    return generate_moves(board, moves);
}

// Generate pawn moves
int generate_pawn_moves(const Board* board, Move* moves, int* count) {
    Color color = board->side_to_move;
    Bitboard pawns = board->pieces[color][PAWN];
    Bitboard empty = board->empty;
    Bitboard enemies = board_get_all_pieces(board, color_opposite(color));
    
    while (pawns) {
        Square from = pop_lsb(&pawns);
        
        // Single pawn push
        Square push_sq = (color == WHITE) ? (Square)(from + 8) : (Square)(from - 8);
        if (push_sq >= 0 && push_sq < 64 && test_bit(empty, push_sq)) {
            // Check for promotion
            if ((color == WHITE && rank_of(push_sq) == RANK_8) || 
                (color == BLACK && rank_of(push_sq) == RANK_1)) {
                // Generate all 4 promotion pieces
                moves[*count] = make_move(from, push_sq, (uint8_t)(PROMOTION | 0)); // Knight
                (*count)++;
                moves[*count] = make_move(from, push_sq, (uint8_t)(PROMOTION | 1)); // Bishop
                (*count)++;
                moves[*count] = make_move(from, push_sq, (uint8_t)(PROMOTION | 2)); // Rook
                (*count)++;
                moves[*count] = make_move(from, push_sq, (uint8_t)(PROMOTION | 3)); // Queen
                (*count)++;
            } else {
                moves[*count] = make_move(from, push_sq, QUIET);
                (*count)++;
                
                // Double pawn push from starting rank
                if ((color == WHITE && rank_of(from) == RANK_2) || 
                    (color == BLACK && rank_of(from) == RANK_7)) {
                    Square double_push = (color == WHITE) ? (Square)(from + 16) : (Square)(from - 16);
                    if (double_push >= 0 && double_push < 64 && test_bit(empty, double_push)) {
                        moves[*count] = make_move(from, double_push, DOUBLE_PAWN_PUSH);
                        (*count)++;
                    }
                }
            }
        }
        
        // Pawn captures
        Bitboard attacks = pawn_attacks[color][from] & enemies;
        while (attacks) {
            Square to = pop_lsb(&attacks);
            if ((color == WHITE && rank_of(to) == RANK_8) || 
                (color == BLACK && rank_of(to) == RANK_1)) {
                // Capture promotions
                moves[*count] = make_move(from, to, (uint8_t)(PROMOTION_CAPTURE | 0)); // Knight
                (*count)++;
                moves[*count] = make_move(from, to, (uint8_t)(PROMOTION_CAPTURE | 1)); // Bishop
                (*count)++;
                moves[*count] = make_move(from, to, (uint8_t)(PROMOTION_CAPTURE | 2)); // Rook
                (*count)++;
                moves[*count] = make_move(from, to, (uint8_t)(PROMOTION_CAPTURE | 3)); // Queen
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
    Bitboard own_pieces = board_get_all_pieces(board, color);
    
    while (knights) {
        Square from = pop_lsb(&knights);
        Bitboard attacks = knight_attacks[from] & ~own_pieces;
        
        while (attacks) {
            Square to = pop_lsb(&attacks);
            Bitboard enemies = board_get_all_pieces(board, color_opposite(color));
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
    Bitboard own_pieces = board_get_all_pieces(board, color);
    Bitboard enemy_pieces = board_get_all_pieces(board, color_opposite(color));
    
    while (bishops) {
        Square from = pop_lsb(&bishops);
        int deltas[4] = { 9, 7, -7, -9 }; // NE, NW, SW, SE
        
        for (int d = 0; d < 4; d++) {
            int to = (int)from + deltas[d];
            
            while (to >= 0 && to < 64 && is_valid_destination(from, (Square)to, deltas[d])) {
                if (test_bit(own_pieces, (Square)to)) break; // Blocked by own piece
                
                uint8_t flags = test_bit(enemy_pieces, (Square)to) ? CAPTURE : QUIET;
                moves[*count] = make_move(from, (Square)to, flags);
                (*count)++;
                
                if (flags == CAPTURE) break; // Can't continue past capture
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
    Bitboard own_pieces = board_get_all_pieces(board, color);
    Bitboard enemy_pieces = board_get_all_pieces(board, color_opposite(color));
    
    while (rooks) {
        Square from = pop_lsb(&rooks);
        int deltas[4] = { 8, -8, 1, -1 }; // N, S, E, W
        
        for (int d = 0; d < 4; d++) {
            int to = (int)from + deltas[d];
            
            while (to >= 0 && to < 64 && is_valid_destination(from, (Square)to, deltas[d])) {
                if (test_bit(own_pieces, (Square)to)) break; // Blocked by own piece
                
                uint8_t flags = test_bit(enemy_pieces, (Square)to) ? CAPTURE : QUIET;
                moves[*count] = make_move(from, (Square)to, flags);
                (*count)++;
                
                if (flags == CAPTURE) break; // Can't continue past capture
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
    Bitboard own_pieces = board_get_all_pieces(board, color);
    Bitboard enemy_pieces = board_get_all_pieces(board, color_opposite(color));
    
    while (queens) {
        Square from = pop_lsb(&queens);
        int deltas[8] = { 9, 7, -7, -9, 8, -8, 1, -1 }; // All 8 directions
        
        for (int d = 0; d < 8; d++) {
            int to = (int)from + deltas[d];
            
            while (to >= 0 && to < 64 && is_valid_destination(from, (Square)to, deltas[d])) {
                if (test_bit(own_pieces, (Square)to)) break; // Blocked by own piece
                
                uint8_t flags = test_bit(enemy_pieces, (Square)to) ? CAPTURE : QUIET;
                moves[*count] = make_move(from, (Square)to, flags);
                (*count)++;
                
                if (flags == CAPTURE) break; // Can't continue past capture
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
    Bitboard own_pieces = board_get_all_pieces(board, color);
    
    while (kings) {
        Square from = pop_lsb(&kings);
        Bitboard attacks = king_attacks[from] & ~own_pieces;
        
        while (attacks) {
            Square to = pop_lsb(&attacks);
            Bitboard enemy_pieces = board_get_all_pieces(board, color_opposite(color));
            uint8_t flags = test_bit(enemy_pieces, to) ? CAPTURE : QUIET;
            moves[*count] = make_move(from, to, flags);
            (*count)++;
        }
        
        // Castling moves - only generate if all conditions are met
        if (!board_is_check(board)) { // Can't castle out of check
            if (color == WHITE) {
                // White kingside castling
                if ((board->castling_rights & WHITE_KINGSIDE) && 
                    test_bit(board->empty, F1) && test_bit(board->empty, G1) &&
                    test_bit(board->pieces[WHITE][ROOK], H1) &&
                    !board_is_square_attacked(board, E1, BLACK) &&
                    !board_is_square_attacked(board, F1, BLACK) &&
                    !board_is_square_attacked(board, G1, BLACK)) {
                    moves[*count] = make_move(E1, G1, KING_CASTLE);
                    (*count)++;
                }
                
                // White queenside castling
                if ((board->castling_rights & WHITE_QUEENSIDE) && 
                    test_bit(board->empty, B1) && test_bit(board->empty, C1) && test_bit(board->empty, D1) &&
                    test_bit(board->pieces[WHITE][ROOK], A1) &&
                    !board_is_square_attacked(board, E1, BLACK) &&
                    !board_is_square_attacked(board, D1, BLACK) &&
                    !board_is_square_attacked(board, C1, BLACK)) {
                    moves[*count] = make_move(E1, C1, QUEEN_CASTLE);
                    (*count)++;
                }
            } else {
                // Black kingside castling
                if ((board->castling_rights & BLACK_KINGSIDE) && 
                    test_bit(board->empty, F8) && test_bit(board->empty, G8) &&
                    test_bit(board->pieces[BLACK][ROOK], H8) &&
                    !board_is_square_attacked(board, E8, WHITE) &&
                    !board_is_square_attacked(board, F8, WHITE) &&
                    !board_is_square_attacked(board, G8, WHITE)) {
                    moves[*count] = make_move(E8, G8, KING_CASTLE);
                    (*count)++;
                }
                
                // Black queenside castling
                if ((board->castling_rights & BLACK_QUEENSIDE) && 
                    test_bit(board->empty, B8) && test_bit(board->empty, C8) && test_bit(board->empty, D8) &&
                    test_bit(board->pieces[BLACK][ROOK], A8) &&
                    !board_is_square_attacked(board, E8, WHITE) &&
                    !board_is_square_attacked(board, D8, WHITE) &&
                    !board_is_square_attacked(board, C8, WHITE)) {
                    moves[*count] = make_move(E8, C8, QUEEN_CASTLE);
                    (*count)++;
                }
            }
        }
    }
    
    return *count;
}

// Check if a move is legal (does not leave own king in check)
int is_legal_move(const Board* board, Move move) {
    Board temp = *board;
    Color moving_color = board->side_to_move;
    
    board_make_move(&temp, move);
    
    // Find the king square after the move
    Square king_sq = lsb(temp.pieces[moving_color][KING]);
    
    // Check if the king is in check after the move
    return !board_is_square_attacked(&temp, king_sq, color_opposite(moving_color));
}