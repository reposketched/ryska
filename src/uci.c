#include "uci.h"
#include "bitboard.h"
#include "evaluation.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Global board state
static Board current_board;
static int engine_ready = 0;

// Main UCI loop
void uci_loop(void) {
    char command[1024];
    
    while (fgets(command, sizeof(command), stdin)) {
        // Remove newline
        command[strcspn(command, "\n")] = 0;
        
        if (strncmp(command, "uci", 3) == 0) {
            uci_uci();
        } else if (strncmp(command, "isready", 7) == 0) {
            uci_isready();
        } else if (strncmp(command, "ucinewgame", 10) == 0) {
            uci_newgame();
        } else if (strncmp(command, "position", 8) == 0) {
            uci_position(command);
        } else if (strncmp(command, "go", 2) == 0) {
            uci_go(command);
        } else if (strncmp(command, "quit", 4) == 0) {
            uci_quit();
        } else if (strncmp(command, "stop", 4) == 0) {
            uci_stop();
        }
    }
}

// Respond to uci command
void uci_uci(void) {
    printf("id name SimpleChessEngine\n");
    printf("id author AI Assistant\n");
    printf("option name Hash type spin default 32 min 1 max 1024\n");
    printf("option name Threads type spin default 1 min 1 max 8\n");
    printf("uciok\n");
}

// Respond to isready command
void uci_isready(void) {
    if (!engine_ready) {
        // Initialize engine components
        init_bitboards();
        init_evaluation_tables();
        board_init(&current_board);
        engine_ready = 1;
    }
    printf("readyok\n");
}

// Respond to newgame command
void uci_newgame(void) {
    board_init(&current_board);
}

// Parse position command
void uci_position(const char* command) {
    char* fen_start = strstr(command, "fen");
    char* moves_start = strstr(command, "moves");
    
    if (fen_start) {
        // Parse FEN position
        fen_start += 4; // Skip "fen "
        board_set_fen(&current_board, fen_start);
    } else {
        // Start from initial position
        board_init(&current_board);
    }
    
    // Parse moves
    if (moves_start) {
        moves_start += 6; // Skip "moves "
        char* move_str = strtok(moves_start, " ");
        while (move_str) {
            Move move = parse_move(move_str);
            if (!move_equal(move, NULL_MOVE)) {
                board_make_move(&current_board, move);
            }
            move_str = strtok(NULL, " ");
        }
    }
}

// Parse go command and start search
void uci_go(const char* command) {
    int depth = 4; // Default depth
    int movetime = 0;
    
    // Parse search parameters
    if (strstr(command, "depth")) {
        char* depth_str = strstr(command, "depth");
        depth_str += 6; // Skip "depth "
        depth = atoi(depth_str);
    }
    
    if (strstr(command, "movetime")) {
        char* movetime_str = strstr(command, "movetime");
        movetime_str += 9; // Skip "movetime "
        movetime = atoi(movetime_str);
    }
    
    // Perform search
    SearchResult result = search_position(&current_board, depth);
    
    // Output best move
    printf("bestmove ");
    print_move(result.best_move);
    printf("\n");
}

// Parse move from UCI format (e.g., "e2e4")
Move parse_move(const char* move_str) {
    if (strlen(move_str) < 4) return NULL_MOVE;
    
    // Parse squares
    File from_file = (File)(move_str[0] - 'a');
    Rank from_rank = (Rank)(move_str[1] - '1');
    File to_file = (File)(move_str[2] - 'a');
    Rank to_rank = (Rank)(move_str[3] - '1');
    
    if (from_file < 0 || from_file > 7 || from_rank < 0 || from_rank > 7 ||
        to_file < 0 || to_file > 7 || to_rank < 0 || to_rank > 7) {
        return NULL_MOVE;
    }
    
    Square from = make_square(from_file, from_rank);
    Square to = make_square(to_file, to_rank);
    
    // Determine move flags
    uint8_t flags = QUIET;
    
    // Check for captures
    if (board_is_square_occupied(&current_board, to)) {
        flags = CAPTURE;
    }
    
    // Check for en passant
    if (current_board.en_passant != A1 && to == current_board.en_passant) {
        flags = EN_PASSANT;
    }
    
    // Check for double pawn push
    if (board_get_piece_at(&current_board, from) == PAWN) {
        if ((current_board.side_to_move == WHITE && from_rank == RANK_2 && to_rank == RANK_4) ||
            (current_board.side_to_move == BLACK && from_rank == RANK_7 && to_rank == RANK_5)) {
            flags = DOUBLE_PAWN_PUSH;
        }
    }
    
    // Check for castling
    if (board_get_piece_at(&current_board, from) == KING) {
        if ((from == E1 && to == G1) || (from == E8 && to == G8)) {
            flags = KING_CASTLE;
        } else if ((from == E1 && to == C1) || (from == E8 && to == C8)) {
            flags = QUEEN_CASTLE;
        }
    }
    
    // Check for promotions
    if (board_get_piece_at(&current_board, from) == PAWN) {
        if ((current_board.side_to_move == WHITE && to_rank == RANK_8) ||
            (current_board.side_to_move == BLACK && to_rank == RANK_1)) {
            flags = PROMOTION;
            if (flags == CAPTURE) flags = PROMOTION_CAPTURE;
        }
    }
    
    return make_move(from, to, flags);
}

// Print move in UCI format
void print_move(Move move) {
    if (move_equal(move, NULL_MOVE)) {
        printf("0000");
        return;
    }
    
    Square from = move_from(move);
    Square to = move_to(move);
    
    File from_file = file_of(from);
    Rank from_rank = rank_of(from);
    File to_file = file_of(to);
    Rank to_rank = rank_of(to);
    
    printf("%c%c%c%c", 
           'a' + from_file, '1' + from_rank,
           'a' + to_file, '1' + to_rank);
    
    // Add promotion piece if applicable
    if (is_promotion(move)) {
        PieceType promo = promotion_piece(move);
        switch (promo) {
            case KNIGHT: printf("n"); break;
            case BISHOP: printf("b"); break;
            case ROOK: printf("r"); break;
            case QUEEN: printf("q"); break;
            default: break;
        }
    }
}

// Quit the engine
void uci_quit(void) {
    exit(0);
}

// Stop the search (not implemented in this simple version)
void uci_stop(void) {
    // In a real implementation, this would stop the search
    printf("info string Search stopped\n");
}

// Print UCI info
void uci_info(const char* info) {
    printf("info %s\n", info);
} 