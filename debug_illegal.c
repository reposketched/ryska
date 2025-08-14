#include <stdio.h>
#include "src/board.h"
#include "src/movegen.h"
#include "src/bitboard.h"
#include "src/uci.h"

void print_board_state(const Board* board) {
    printf("Board state:\n");
    printf("Side to move: %s\n", board->side_to_move == WHITE ? "WHITE" : "BLACK");
    
    printf("\nPiece positions:\n");
    for (Color c = WHITE; c < COLOR_COUNT; c++) {
        const char* color_name = (c == WHITE) ? "WHITE" : "BLACK";
        for (PieceType pt = PAWN; pt < PIECE_COUNT; pt++) {
            const char* piece_names[] = {"PAWN", "KNIGHT", "BISHOP", "ROOK", "QUEEN", "KING"};
            Bitboard pieces = board->pieces[c][pt];
            if (pieces) {
                printf("%s %s: ", color_name, piece_names[pt]);
                for (Square sq = A1; sq < SQUARE_COUNT; sq++) {
                    if (pieces & (1ULL << sq)) {
                        printf("%c%d ", 'a' + file_of(sq), rank_of(sq) + 1);
                    }
                }
                printf("\n");
            }
        }
    }
    printf("\n");
}

void test_game_position() {
    init_bitboards();
    
    Board board;
    board_init(&board);
    
    printf("=== Testing Game Position ===\n");
    
    // Reproduce the game moves up to the illegal move
    const char* moves[] = {
        "d2d4", "b8c6", "d4d5", "g8f6", "d5c6", "b7c6",
        "b1c3", "d7d5", "e2e4", "d5e4", "c3e4", "d8d1",
        "e1d1", "f6e4", "f2f3", "e7e5", "f3e4", "f7f5",
        "e4f5", "c6c5", "f5f6", "g7f6"
    };
    
    int num_moves = sizeof(moves) / sizeof(moves[0]);
    
    for (int i = 0; i < num_moves; i++) {
        printf("\nMove %d: %s\n", i + 1, moves[i]);
        
        Move move = parse_move(moves[i]);
        if (move.data == 0) {
            printf("ERROR: Failed to parse move %s\n", moves[i]);
            break;
        }
        
        // Check if the move is legal
        if (!is_legal_move(&board, move)) {
            printf("ERROR: Move %s is not legal!\n", moves[i]);
            print_board_state(&board);
            break;
        }
        
        board_make_move(&board, move);
        printf("Move applied successfully\n");
    }
    
    printf("\n=== Final Position ===\n");
    print_board_state(&board);
    
    // Now generate moves for white (who should play move 12)
    printf("\n=== Legal moves for WHITE ===\n");
    Move legal_moves[256];
    int move_count = generate_moves(&board, legal_moves);
    
    printf("Found %d legal moves:\n", move_count);
    for (int i = 0; i < move_count && i < 20; i++) {
        Square from = move_from(legal_moves[i]);
        Square to = move_to(legal_moves[i]);
        printf("  %c%d-%c%d", 
               'a' + file_of(from), rank_of(from) + 1,
               'a' + file_of(to), rank_of(to) + 1);
        
        if (from == C1 && to == B5) {
            printf(" <- This would be Bb5 (the illegal move!)");
        }
        printf("\n");
    }
    
    // Check specifically if Bb5 is in the legal moves
    Move bb5 = make_move(C1, B5, QUIET);
    int bb5_legal = 0;
    for (int i = 0; i < move_count; i++) {
        if (move_equal(legal_moves[i], bb5)) {
            bb5_legal = 1;
            break;
        }
    }
    
    printf("\nBb5 (c1-b5) is %s in the legal move list\n", bb5_legal ? "INCLUDED" : "NOT INCLUDED");
    
    // Check if there's a white bishop on c1
    printf("White bishop on c1: %s\n", (board.pieces[WHITE][BISHOP] & (1ULL << C1)) ? "YES" : "NO");
    printf("Square c1 occupied: %s\n", (board.occupied & (1ULL << C1)) ? "YES" : "NO");
    printf("Square b5 occupied: %s\n", (board.occupied & (1ULL << B5)) ? "YES" : "NO");
}

int main() {
    test_game_position();
    return 0;
}