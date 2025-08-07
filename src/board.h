#ifndef BOARD_H
#define BOARD_H

#include "constants.h"
#include "bitboard.h"
#include "move.h"

// Board structure
typedef struct {
    Bitboard pieces[2][6];  // [color][piece_type]
    Bitboard occupied;       // All occupied squares
    Bitboard empty;          // All empty squares
    Color side_to_move;      // Current side to move
    Square en_passant;       // En passant square (if any)
    int castling_rights;     // Castling rights (bit flags)
    int halfmove_clock;      // Halfmove clock for 50-move rule
    int fullmove_number;     // Fullmove number
} Board;

// Castling rights
#define WHITE_KINGSIDE  1
#define WHITE_QUEENSIDE 2
#define BLACK_KINGSIDE  4
#define BLACK_QUEENSIDE 8

// Board functions
void board_init(Board* board);
void board_set_fen(Board* board, const char* fen);
void board_get_fen(const Board* board, char* fen);
void board_make_move(Board* board, Move move);
void board_undo_move(Board* board, Move move);
int board_is_check(const Board* board);
int board_is_checkmate(const Board* board);
int board_is_stalemate(const Board* board);
int board_is_legal_move(const Board* board, Move move);

// Board state queries
Bitboard board_get_pieces(const Board* board, Color color, PieceType piece);
Bitboard board_get_all_pieces(const Board* board, Color color);
PieceType board_get_piece_at(const Board* board, Square sq);
Color board_get_color_at(const Board* board, Square sq);
int board_is_square_occupied(const Board* board, Square sq);
int board_is_square_attacked(const Board* board, Square sq, Color by_color);

// Utility functions
void board_print(const Board* board);
int board_validate(const Board* board);

#endif // BOARD_H 