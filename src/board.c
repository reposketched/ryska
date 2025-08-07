#include "board.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Initialize board to starting position
void board_init(Board* board) {
    memset(board, 0, sizeof(Board));
    // Set up initial piece positions
    board->pieces[WHITE][PAWN]   = RANK_2_BB;
    board->pieces[WHITE][KNIGHT] = (1ULL << B1) | (1ULL << G1);
    board->pieces[WHITE][BISHOP] = (1ULL << C1) | (1ULL << F1);
    board->pieces[WHITE][ROOK]   = (1ULL << A1) | (1ULL << H1);
    board->pieces[WHITE][QUEEN]  = (1ULL << D1);
    board->pieces[WHITE][KING]   = (1ULL << E1);
    board->pieces[BLACK][PAWN]   = RANK_7_BB;
    board->pieces[BLACK][KNIGHT] = (1ULL << B8) | (1ULL << G8);
    board->pieces[BLACK][BISHOP] = (1ULL << C8) | (1ULL << F8);
    board->pieces[BLACK][ROOK]   = (1ULL << A8) | (1ULL << H8);
    board->pieces[BLACK][QUEEN]  = (1ULL << D8);
    board->pieces[BLACK][KING]   = (1ULL << E8);
    board->occupied = 0;
    for (Color c = WHITE; c < COLOR_COUNT; c++)
        for (PieceType pt = PAWN; pt < PIECE_COUNT; pt++)
            board->occupied |= board->pieces[c][pt];
    board->empty = ~board->occupied;
    board->side_to_move = WHITE;
    board->en_passant = A1;
    board->castling_rights = WHITE_KINGSIDE | WHITE_QUEENSIDE | BLACK_KINGSIDE | BLACK_QUEENSIDE;
    board->halfmove_clock = 0;
    board->fullmove_number = 1;
}

// Parse FEN string and set board state
void board_set_fen(Board* board, const char* fen) {
    board_init(board);
    const char* p = fen;
    Square sq = A8;
    // Piece placement
    while (*p && *p != ' ') {
        if (*p == '/') {
            sq = (Square)(sq - 16);
        } else if (isdigit(*p)) {
            sq = (Square)(sq + (*p - '0'));
        } else {
            Color color = isupper(*p) ? WHITE : BLACK;
            PieceType piece;
            char c = toupper(*p);
            switch (c) {
                case 'P': piece = PAWN; break;
                case 'N': piece = KNIGHT; break;
                case 'B': piece = BISHOP; break;
                case 'R': piece = ROOK; break;
                case 'Q': piece = QUEEN; break;
                case 'K': piece = KING; break;
                default: piece = PAWN; break;
            }
            board->pieces[color][piece] |= (1ULL << sq);
            sq = (Square)(sq + 1);
        }
        p++;
    }
    board->occupied = 0;
    for (Color c = WHITE; c < COLOR_COUNT; c++)
        for (PieceType pt = PAWN; pt < PIECE_COUNT; pt++)
            board->occupied |= board->pieces[c][pt];
    board->empty = ~board->occupied;
    if (*p == ' ') p++;
    if (*p == 'w') board->side_to_move = WHITE;
    else if (*p == 'b') board->side_to_move = BLACK;
    p++;
    if (*p == ' ') p++;
    board->castling_rights = 0;
    while (*p && *p != ' ') {
        switch (*p) {
            case 'K': board->castling_rights |= WHITE_KINGSIDE; break;
            case 'Q': board->castling_rights |= WHITE_QUEENSIDE; break;
            case 'k': board->castling_rights |= BLACK_KINGSIDE; break;
            case 'q': board->castling_rights |= BLACK_QUEENSIDE; break;
        }
        p++;
    }
    if (*p == ' ') p++;
    if (*p == '-') {
        board->en_passant = A1;
        p++;
    } else if (*p >= 'a' && *p <= 'h' && *(p+1) >= '1' && *(p+1) <= '8') {
        File f = (File)(*p - 'a');
        Rank r = (Rank)(*(p+1) - '1');
        board->en_passant = make_square(f, r);
        p += 2;
    }
    if (*p == ' ') p++;
    if (isdigit(*p)) {
        board->halfmove_clock = 0;
        while (isdigit(*p)) {
            board->halfmove_clock = board->halfmove_clock * 10 + (*p - '0');
            p++;
        }
    }
    if (*p == ' ') p++;
    if (isdigit(*p)) {
        board->fullmove_number = 0;
        while (isdigit(*p)) {
            board->fullmove_number = board->fullmove_number * 10 + (*p - '0');
            p++;
        }
    }
}

void board_get_fen(const Board* board, char* fen) {
    char* p = fen;
    for (Rank r = RANK_8; r >= RANK_1; r--) {
        int empty_count = 0;
        for (File f = FILE_A; f <= FILE_H; f++) {
            Square sq = make_square(f, r);
            int found = 0;
            char piece_char = ' ';
            for (Color c = WHITE; c < COLOR_COUNT && !found; c++)
                for (PieceType pt = PAWN; pt < PIECE_COUNT && !found; pt++)
                    if (board->pieces[c][pt] & (1ULL << sq)) {
                        found = 1;
                        switch (pt) {
                            case PAWN: piece_char = 'p'; break;
                            case KNIGHT: piece_char = 'n'; break;
                            case BISHOP: piece_char = 'b'; break;
                            case ROOK: piece_char = 'r'; break;
                            case QUEEN: piece_char = 'q'; break;
                            case KING: piece_char = 'k'; break;
                        }
                        if (c == WHITE) piece_char = toupper(piece_char);
                    }
            if (found) {
                if (empty_count > 0) { *p++ = '0' + empty_count; empty_count = 0; }
                *p++ = piece_char;
            } else {
                empty_count++;
            }
        }
        if (empty_count > 0) *p++ = '0' + empty_count;
        if (r > RANK_1) *p++ = '/';
    }
    *p++ = ' ';
    *p++ = (board->side_to_move == WHITE) ? 'w' : 'b';
    *p++ = ' ';
    if (board->castling_rights & WHITE_KINGSIDE) *p++ = 'K';
    if (board->castling_rights & WHITE_QUEENSIDE) *p++ = 'Q';
    if (board->castling_rights & BLACK_KINGSIDE) *p++ = 'k';
    if (board->castling_rights & BLACK_QUEENSIDE) *p++ = 'q';
    if (!board->castling_rights) *p++ = '-';
    *p++ = ' ';
    if (board->en_passant == A1) *p++ = '-';
    else {
        File f = file_of(board->en_passant);
        Rank r = rank_of(board->en_passant);
        *p++ = 'a' + f;
        *p++ = '1' + r;
    }
    sprintf(p, " %d %d", board->halfmove_clock, board->fullmove_number);
}

void board_make_move(Board* board, Move move) {
    Square from = move_from(move);
    Square to = move_to(move);
    uint8_t flags = move_flags(move);
    PieceType piece = PAWN;
    Color color = board->side_to_move;
    for (PieceType pt = PAWN; pt < PIECE_COUNT; pt++)
        if (board->pieces[color][pt] & (1ULL << from)) { piece = pt; break; }
    board->pieces[color][piece] &= ~(1ULL << from);
    if (is_capture(move)) {
        if (is_en_passant(move)) {
            Square captured_sq = (color == WHITE) ? (Square)(to - 8) : (Square)(to + 8);
            board->pieces[color_opposite(color)][PAWN] &= ~(1ULL << captured_sq);
        } else {
            for (PieceType pt = PAWN; pt < PIECE_COUNT; pt++)
                if (board->pieces[color_opposite(color)][pt] & (1ULL << to)) {
                    board->pieces[color_opposite(color)][pt] &= ~(1ULL << to);
                    break;
                }
        }
    }
    if (is_promotion(move)) piece = promotion_piece(move);
    board->pieces[color][piece] |= (1ULL << to);
    if (is_castle(move)) {
        Square rook_from, rook_to;
        if (flags == KING_CASTLE) {
            rook_from = (color == WHITE) ? H1 : H8;
            rook_to = (color == WHITE) ? F1 : F8;
        } else {
            rook_from = (color == WHITE) ? A1 : A8;
            rook_to = (color == WHITE) ? D1 : D8;
        }
        board->pieces[color][ROOK] &= ~(1ULL << rook_from);
        board->pieces[color][ROOK] |= (1ULL << rook_to);
    }
    board->occupied = 0;
    for (Color c = WHITE; c < COLOR_COUNT; c++)
        for (PieceType pt = PAWN; pt < PIECE_COUNT; pt++)
            board->occupied |= board->pieces[c][pt];
    board->empty = ~board->occupied;
    if (is_double_pawn_push(move))
        board->en_passant = (color == WHITE) ? (Square)(from + 8) : (Square)(from - 8);
    else
        board->en_passant = A1;
    if (piece == KING) {
        if (color == WHITE) board->castling_rights &= ~(WHITE_KINGSIDE | WHITE_QUEENSIDE);
        else board->castling_rights &= ~(BLACK_KINGSIDE | BLACK_QUEENSIDE);
    } else if (piece == ROOK) {
        if (color == WHITE) {
            if (from == A1) board->castling_rights &= ~WHITE_QUEENSIDE;
            if (from == H1) board->castling_rights &= ~WHITE_KINGSIDE;
        } else {
            if (from == A8) board->castling_rights &= ~BLACK_QUEENSIDE;
            if (from == H8) board->castling_rights &= ~BLACK_KINGSIDE;
        }
    }
    if (piece == PAWN || is_capture(move)) board->halfmove_clock = 0;
    else board->halfmove_clock++;
    if (board->side_to_move == BLACK) board->fullmove_number++;
    board->side_to_move = color_opposite(board->side_to_move);
}

void board_undo_move(Board* board, Move move) {
    // Not implemented in this simple version
}

int board_is_check(const Board* board) {
    Square king_sq = lsb(board->pieces[board->side_to_move][KING]);
    return board_is_square_attacked(board, king_sq, color_opposite(board->side_to_move));
}

int board_is_checkmate(const Board* board) {
    if (!board_is_check(board)) return 0;
    Move moves[256];
    int move_count = 0; // Not implemented: should call generate_moves
    return move_count == 0;
}

int board_is_stalemate(const Board* board) {
    if (board_is_check(board)) return 0;
    int move_count = 0; // Not implemented: should call generate_moves
    return move_count == 0;
}

int board_is_legal_move(const Board* board, Move move) {
    // Not implemented in this simple version
    return 1;
}

Bitboard board_get_pieces(const Board* board, Color color, PieceType piece) {
    return board->pieces[color][piece];
}

Bitboard board_get_all_pieces(const Board* board, Color color) {
    Bitboard result = 0;
    for (PieceType pt = PAWN; pt < PIECE_COUNT; pt++)
        result |= board->pieces[color][pt];
    return result;
}

PieceType board_get_piece_at(const Board* board, Square sq) {
    for (Color c = WHITE; c < COLOR_COUNT; c++)
        for (PieceType pt = PAWN; pt < PIECE_COUNT; pt++)
            if (board->pieces[c][pt] & (1ULL << sq))
                return pt;
    return PAWN;
}

Color board_get_color_at(const Board* board, Square sq) {
    for (Color c = WHITE; c < COLOR_COUNT; c++)
        for (PieceType pt = PAWN; pt < PIECE_COUNT; pt++)
            if (board->pieces[c][pt] & (1ULL << sq))
                return c;
    return WHITE;
}

int board_is_square_occupied(const Board* board, Square sq) {
    return (board->occupied & (1ULL << sq)) != 0;
}

int board_is_square_attacked(const Board* board, Square sq, Color by_color) {
    // Pawn attacks
    if (pawn_attacks[by_color][sq] & board->pieces[by_color][PAWN]) return 1;
    // Knight attacks
    if (knight_attacks[sq] & board->pieces[by_color][KNIGHT]) return 1;
    // King attacks
    if (king_attacks[sq] & board->pieces[by_color][KING]) return 1;
    // Sliding pieces not implemented
    return 0;
}

void board_print(const Board* board) {
    printf("\n");
    for (Rank r = RANK_8; r >= RANK_1; r--) {
        printf("%d ", r + 1);
        for (File f = FILE_A; f <= FILE_H; f++) {
            Square sq = make_square(f, r);
            char piece_char = '.';
            for (Color c = WHITE; c < COLOR_COUNT; c++)
                for (PieceType pt = PAWN; pt < PIECE_COUNT; pt++)
                    if (board->pieces[c][pt] & (1ULL << sq)) {
                        switch (pt) {
                            case PAWN: piece_char = 'p'; break;
                            case KNIGHT: piece_char = 'n'; break;
                            case BISHOP: piece_char = 'b'; break;
                            case ROOK: piece_char = 'r'; break;
                            case QUEEN: piece_char = 'q'; break;
                            case KING: piece_char = 'k'; break;
                        }
                        if (c == WHITE) piece_char = toupper(piece_char);
                    }
            printf("%c ", piece_char);
        }
        printf("\n");
    }
    printf("  a b c d e f g h\n");
    printf("Side to move: %s\n", (board->side_to_move == WHITE) ? "White" : "Black");
}

int board_validate(const Board* board) {
    Bitboard all_pieces = 0;
    for (Color c = WHITE; c < COLOR_COUNT; c++)
        for (PieceType pt = PAWN; pt < PIECE_COUNT; pt++) {
            if (board->pieces[c][pt] & all_pieces) return 0;
            all_pieces |= board->pieces[c][pt];
        }
    return 1;
}