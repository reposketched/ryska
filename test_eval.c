#include <stdio.h>
#include "src/board.h"
#include "src/evaluation.h"
#include "src/bitboard.h"

int main() {
    init_bitboards();
    init_evaluation_tables();
    
    Board board;
    board_init(&board);
    
    printf("=== Evaluation Test ===\n");
    printf("Starting position evaluation: %d\n", evaluate_position(&board));
    printf("Starting material balance: %d\n", evaluate_material(&board));
    
    // Test after e2-e4
    board_make_move(&board, make_move(E2, E4, DOUBLE_PAWN_PUSH));
    board.side_to_move = BLACK; // Switch to black
    
    printf("After e2-e4 evaluation: %d\n", evaluate_position(&board));
    printf("After e2-e4 material balance: %d\n", evaluate_material(&board));
    
    return 0;
}