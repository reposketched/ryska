#include <stdio.h>
#include "src/board.h"
#include "src/movegen.h"
#include "src/bitboard.h"

void check_board_integrity(const Board* board, const char* label) {
    printf("\n=== %s ===\n", label);
    
    // Check for overlapping pieces
    Bitboard all_white = 0, all_black = 0;
    for (PieceType pt = PAWN; pt < PIECE_COUNT; pt++) {
        all_white |= board->pieces[WHITE][pt];
        all_black |= board->pieces[BLACK][pt];
    }
    
    // Check if occupied squares match piece positions
    Bitboard calculated_occupied = all_white | all_black;
    
    printf("Occupied squares match: %s\n", 
           (calculated_occupied == board->occupied) ? "YES" : "NO");
    
    // Check for overlapping white pieces
    Bitboard white_overlap = 0;
    for (PieceType pt1 = PAWN; pt1 < PIECE_COUNT; pt1++) {
        for (PieceType pt2 = pt1 + 1; pt2 < PIECE_COUNT; pt2++) {
            white_overlap |= (board->pieces[WHITE][pt1] & board->pieces[WHITE][pt2]);
        }
    }
    
    if (white_overlap) {
        printf("WHITE PIECE OVERLAP detected on squares: ");
        for (Square sq = A1; sq < SQUARE_COUNT; sq++) {
            if (white_overlap & (1ULL << sq)) {
                printf("%c%d ", 'a' + file_of(sq), rank_of(sq) + 1);
            }
        }
        printf("\n");
    }
    
    // Check for overlapping black pieces
    Bitboard black_overlap = 0;
    for (PieceType pt1 = PAWN; pt1 < PIECE_COUNT; pt1++) {
        for (PieceType pt2 = pt1 + 1; pt2 < PIECE_COUNT; pt2++) {
            black_overlap |= (board->pieces[BLACK][pt1] & board->pieces[BLACK][pt2]);
        }
    }
    
    if (black_overlap) {
        printf("BLACK PIECE OVERLAP detected on squares: ");
        for (Square sq = A1; sq < SQUARE_COUNT; sq++) {
            if (black_overlap & (1ULL << sq)) {
                printf("%c%d ", 'a' + file_of(sq), rank_of(sq) + 1);
            }
        }
        printf("\n");
    }
    
    // Check for pieces on same square between colors
    Bitboard color_overlap = all_white & all_black;
    if (color_overlap) {
        printf("COLOR OVERLAP detected on squares: ");
        for (Square sq = A1; sq < SQUARE_COUNT; sq++) {
            if (color_overlap & (1ULL << sq)) {
                printf("%c%d ", 'a' + file_of(sq), rank_of(sq) + 1);
            }
        }
        printf("\n");
    }
    
    if (!white_overlap && !black_overlap && !color_overlap && calculated_occupied == board->occupied) {
        printf("Board integrity: OK\n");
    }
}

int main() {
    init_bitboards();
    
    Board board;
    board_init(&board);
    
    check_board_integrity(&board, "Initial position");
    
    // Test the critical Qxd1+ Kxd1 sequence
    // Let's focus on just the queen exchange that seems to cause corruption
    
    // Apply: d4 Nc6 d5 Nf6 dxc6 bxc6 Nc3 d5 e4 dxe4 Nxe4
    Move moves[] = {
        make_move(D2, D4, QUIET),         // d4
        make_move(B8, C6, QUIET),         // Nc6
        make_move(D4, D5, QUIET),         // d5
        make_move(G8, F6, QUIET),         // Nf6
        make_move(D5, C6, CAPTURE),       // dxc6
        make_move(B7, C6, CAPTURE),       // bxc6
        make_move(B1, C3, QUIET),         // Nc3
        make_move(D7, D5, QUIET),         // d5
        make_move(E2, E4, QUIET),         // e4
        make_move(D5, E4, CAPTURE),       // dxe4
        make_move(C3, E4, CAPTURE)        // Nxe4
    };
    
    for (int i = 0; i < 11; i++) {
        printf("\nApplying move %d...\n", i + 1);
        board_make_move(&board, moves[i]);
        char label[50];
        sprintf(label, "After move %d", i + 1);
        check_board_integrity(&board, label);
        
        if (i % 2 == 0) {
            board.side_to_move = BLACK;
        } else {
            board.side_to_move = WHITE;
        }
    }
    
    // Now test the critical queen exchange: Qxd1+
    printf("\n=== Testing Qxd1+ ===\n");
    Move qxd1 = make_move(D8, D1, CAPTURE);
    printf("Applying Qxd1+...\n");
    board_make_move(&board, qxd1);
    check_board_integrity(&board, "After Qxd1+");
    
    // And then Kxd1
    printf("\n=== Testing Kxd1 ===\n");
    board.side_to_move = WHITE;
    Move kxd1 = make_move(E1, D1, CAPTURE);
    printf("Applying Kxd1...\n");
    board_make_move(&board, kxd1);
    check_board_integrity(&board, "After Kxd1");
    
    return 0;
}