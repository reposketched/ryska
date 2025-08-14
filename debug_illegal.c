#include <stdio.h>
#include "src/board.h"
#include "src/movegen.h"
#include "src/bitboard.h"
#include "src/uci.h"
#include "src/search.h"
#include "src/evaluation.h"

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
    init_evaluation_tables();
    
    Board board;
    board_init(&board);
    
    printf("=== Testing Game Position ===\n");
    
    // Reproduce the game moves up to and INCLUDING the user's Bb5
    const char* moves[] = {
        "d2d4", "b8c6", "d4d5", "g8f6", "d5c6", "b7c6",
        "b1c3", "d7d5", "e2e4", "d5e4", "c3e4", "d8d1",
        "e1d1", "f6e4", "f2f3", "e7e5", "f3e4", "f7f5",
        "e4f5", "c6c5", "f5f6", "g7f6", "c1b5"  // Added the user's move Bb5
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
    
    printf("\n=== Position after Bb5 ===\n");
    print_board_state(&board);
    
    // Now it's BLACK's turn - let's see what the engine wants to play
    printf("\n=== Legal moves for BLACK ===\n");
    Move legal_moves[256];
    int move_count = generate_moves(&board, legal_moves);
    
    printf("Found %d legal moves:\n", move_count);
    for (int i = 0; i < move_count && i < 20; i++) {
        Square from = move_from(legal_moves[i]);
        Square to = move_to(legal_moves[i]);
        printf("  %c%d-%c%d", 
               'a' + file_of(from), rank_of(from) + 1,
               'a' + file_of(to), rank_of(to) + 1);
        
        if (is_capture(legal_moves[i])) {
            printf(" (capture)");
        }
        printf("\n");
    }
    
    // Let's see what the search wants to play
    printf("\n=== Search Result ===\n");
    SearchResult result = search_position(&board, 6);
    printf("Best move from search: ");
    if (result.best_move.data != 0) {
        Square from = move_from(result.best_move);
        Square to = move_to(result.best_move);
        printf("%c%d-%c%d", 
               'a' + file_of(from), rank_of(from) + 1,
               'a' + file_of(to), rank_of(to) + 1);
        
        // Check if this move is actually legal
        int is_legal = is_legal_move(&board, result.best_move);
        printf(" (Legal: %s)", is_legal ? "YES" : "NO");
        
        // Check if it's in our legal moves list
        int in_list = 0;
        for (int i = 0; i < move_count; i++) {
            if (move_equal(legal_moves[i], result.best_move)) {
                in_list = 1;
                break;
            }
        }
        printf(" (In legal list: %s)", in_list ? "YES" : "NO");
        printf("\n");
    } else {
        printf("NULL_MOVE\n");
    }
}

int main() {
    test_game_position();
    return 0;
}