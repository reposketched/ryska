#include "uci.h"
#include "bitboard.h"
#include "evaluation.h"
#include "movegen.h"
#include "search.h"
#include "book.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>
#include "board.h"

// Global board state
static Board current_board;
static int engine_ready = 0;
static int stop_search = 0;
static char last_moves_string[512] = "";

// Helper: find the exact legal Move object matching from/to (and promotion piece)
static Move find_matching_legal(const Board* board, Square from, Square to, int has_promo, PieceType promo_piece) {
    Move legal_moves[256];
    int n = generate_moves(board, legal_moves);
    for (int i = 0; i < n; i++) {
        if (move_from(legal_moves[i]) == from && move_to(legal_moves[i]) == to) {
            if (!has_promo) {
                // ensure it's not a promotion move
                if (!is_promotion(legal_moves[i])) return legal_moves[i];
            } else {
                if (is_promotion(legal_moves[i]) && promotion_piece(legal_moves[i]) == promo_piece) {
                    return legal_moves[i];
                }
            }
        }
    }
    return NULL_MOVE;
}

// Dummy options
static int hash_size_mb = 32;
static int own_book = 1;

// Forward declaration for uci_setoption
void uci_setoption(const char* command);

// Helper: trim whitespace
static void trim(char* s) {
    char* p = s;
    while (isspace(*p)) p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
    size_t len = strlen(s);
    while (len > 0 && isspace(s[len-1])) s[--len] = 0;
}

// Simple opening book (for startpos and first few moves)
typedef struct { const char* moves; const char* bestmove; } BookEntry;
static const BookEntry opening_book[] = {
    {"", "e2e4"},
    {"e2e4", "e7e5"},
    {"d2d4", "d7d5"},
    {"e2e4 e7e5", "g1f3"},
    {"d2d4 d7d5", "c2c4"},
    {"e2e4 e7e5 g1f3", "b8c6"},
    {"d2d4 d7d5 c2c4", "e7e6"},
    {NULL, NULL}
};

// Helper to get book move
const char* get_book_move(const char* moves) {
    for (int i = 0; opening_book[i].moves; i++) {
        if (strcmp(opening_book[i].moves, moves) == 0) {
            return opening_book[i].bestmove;
        }
    }
    return NULL;
}

// Main UCI loop
void uci_loop(void) {
    char command[1024];
    
    while (fgets(command, sizeof(command), stdin)) {
        command[strcspn(command, "\n")] = 0;
        trim(command);
        
        if (strncmp(command, "ucinewgame", 10) == 0) {
            uci_newgame();
        } else if (strncmp(command, "isready", 7) == 0) {
            uci_isready();
        } else if (strncmp(command, "uci", 3) == 0) {
            uci_uci();
        } else if (strncmp(command, "position", 8) == 0) {
            uci_position(command);
        } else if (strncmp(command, "go", 2) == 0) {
            stop_search = 0;
            uci_go(command);
        } else if (strncmp(command, "eval", 4) == 0) {
            int cp = evaluate_position(&current_board);
            double dec = cp / 100.0;
            printf("info score cp %d\n", cp);
            printf("info string eval %+.2f\n", dec);
            fflush(stdout);
        } else if (strncmp(command, "stop", 4) == 0) {
            stop_search = 1;
            uci_stop();
        } else if (strncmp(command, "quit", 4) == 0) {
            uci_quit();
        } else if (strncmp(command, "setoption", 9) == 0) {
            uci_setoption(command);
        } else if (strncmp(command, "register", 8) == 0) {
            printf("registration ok\n");
        } else if (strncmp(command, "ponderhit", 9) == 0) {
            // Not implemented, just acknowledge
        } else {
            // Ignore unknown commands
        }
    }
}

// Respond to uci command
void uci_uci(void) {
    printf("id name Ryska\n");
    printf("id author Sooryashankar Joy\n");
    printf("option name Hash type spin default 32 min 1 max 1024\n");
    printf("option name Threads type spin default 1 min 1 max 8\n");
    printf("uciok\n");
    fflush(stdout);
}

// Respond to isready command
void uci_isready(void) {
    if (!engine_ready) {
        // Initialize engine components
        init_bitboards();
        init_evaluation_tables();
        board_init(&current_board);
        // Try to load default book file
        book_clear();
        int loaded = book_load("book.txt");
        if (loaded > 0) {
            printf("info string book loaded %d entries\n", loaded);
        }
        engine_ready = 1;
    }
    printf("readyok\n");
    fflush(stdout);
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
        char moves_copy[512];
        strncpy(moves_copy, moves_start, sizeof(moves_copy)-1);
        moves_copy[sizeof(moves_copy)-1] = 0;
        // Save for book use
        strncpy(last_moves_string, moves_copy, sizeof(last_moves_string)-1);
        last_moves_string[sizeof(last_moves_string)-1] = 0;
        size_t l = strlen(last_moves_string);
        while (l > 0 && (last_moves_string[l-1] == '\n' || last_moves_string[l-1] == ' ')) last_moves_string[--l] = 0;
        char* move_str = strtok(moves_copy, " ");
        while (move_str) {
            Move move = parse_move(move_str);
            // Validate parsed move against generated legal moves
            Move legals[256];
            int legal_count = generate_moves(&current_board, legals);
            int ok = 0;
            for (int i = 0; i < legal_count; i++) {
                if (move_equal(move, legals[i]) && is_legal_move(&current_board, legals[i])) { ok = 1; break; }
            }
            if (ok && !move_equal(move, NULL_MOVE)) {
                board_make_move(&current_board, move);
            }
            move_str = strtok(NULL, " ");
        }
    } else {
        last_moves_string[0] = '\0';
    }
    // Optional debug removed for performance
}

// Parse go command and start search
void uci_go(const char* command) {
    int depth = 6;
    int movetime = 0;
    int wtime = 0, btime = 0, winc = 0, binc = 0, movestogo = 0, nodes = 0, mate = 0, infinite = 0;
    char searchmoves[256] = "";
    int ponder = 0;
    // Parse all go options
    const char* p = command;
    while (*p) {
        if (strncmp(p, "depth", 5) == 0) { p += 5; while (*p && isspace(*p)) p++; depth = atoi(p); }
        else if (strncmp(p, "movetime", 8) == 0) { p += 8; while (*p && isspace(*p)) p++; movetime = atoi(p); }
        else if (strncmp(p, "wtime", 5) == 0) { p += 5; while (*p && isspace(*p)) p++; wtime = atoi(p); }
        else if (strncmp(p, "btime", 5) == 0) { p += 5; while (*p && isspace(*p)) p++; btime = atoi(p); }
        else if (strncmp(p, "winc", 4) == 0) { p += 4; while (*p && isspace(*p)) p++; winc = atoi(p); }
        else if (strncmp(p, "binc", 4) == 0) { p += 4; while (*p && isspace(*p)) p++; binc = atoi(p); }
        else if (strncmp(p, "movestogo", 9) == 0) { p += 9; while (*p && isspace(*p)) p++; movestogo = atoi(p); }
        else if (strncmp(p, "nodes", 5) == 0) { p += 5; while (*p && isspace(*p)) p++; nodes = atoi(p); }
        else if (strncmp(p, "mate", 4) == 0) { p += 4; while (*p && isspace(*p)) p++; mate = atoi(p); }
        else if (strncmp(p, "infinite", 8) == 0) { infinite = 1; p += 8; }
        else if (strncmp(p, "searchmoves", 11) == 0) {
            p += 11; while (*p && isspace(*p)) p++;
            strncpy(searchmoves, p, sizeof(searchmoves)-1);
            searchmoves[sizeof(searchmoves)-1] = 0;
        } else if (strncmp(p, "ponder", 6) == 0) { ponder = 1; p += 6; }
        else { p++; }
    }
    // Opening book
    const char* bookmove = own_book ? book_get_move(last_moves_string) : NULL;
    if (bookmove) {
        // Validate book move is legal in the current position; otherwise ignore
        Move book_mv = parse_move(bookmove);
        Move legal_moves[256];
        int legal_count = generate_moves(&current_board, legal_moves);
        int ok = 0;
        for (int i = 0; i < legal_count; i++) {
            if (move_equal(book_mv, legal_moves[i]) && is_legal_move(&current_board, legal_moves[i])) { ok = 1; break; }
        }
        if (ok) {
            printf("bestmove %s\n", bookmove);
            fflush(stdout);
            return;
        }
        // fallthrough: ignore invalid book move for this position
    }
    // Search (interruptible, info lines)
    SearchResult result;
    int nodes_searched = 0;
    int start_time = 0; // TODO: use real time
    for (int d = 1; d <= depth && !stop_search; d++) {
        result = search_position(&current_board, d);
        nodes_searched += result.nodes;
        printf("info depth %d score cp %d nodes %d pv ", d, result.score, nodes_searched);
        print_move(result.best_move); printf("\n");
        fflush(stdout);
        // Human-friendly decimal eval line
        printf("info string eval %+.2f\n", result.score / 100.0);
        fflush(stdout);
        // TODO: add time, nps, pv, etc.
        if (movetime > 0 && d * 100 > movetime) break; // crude time control
    }
    // Check if the move is legal
    Move legal_moves[256];
    int legal_count = generate_moves(&current_board, legal_moves);
    // Pick the PV move if legal; otherwise fall back to first legal
    int found = 0;
    for (int i = 0; i < legal_count; i++) {
        if (move_equal(result.best_move, legal_moves[i]) && is_legal_move(&current_board, legal_moves[i])) { found = 1; break; }
    }
    if (found) {
        // Final evaluation string
        printf("info string finaleval %+.2f\n", result.score / 100.0);
        printf("bestmove "); print_move(result.best_move); printf("\n"); fflush(stdout);
    } else {
        // Fallback: output first legal move if any
        for (int i = 0; i < legal_count; i++) {
            if (is_legal_move(&current_board, legal_moves[i])) {
                int cp = evaluate_position(&current_board);
                printf("info string finaleval %+.2f\n", cp / 100.0);
                printf("bestmove "); print_move(legal_moves[i]); printf("\n"); fflush(stdout);
                return;
            }
        }
        printf("bestmove 0000\n"); fflush(stdout);
    }
}

// Pure parser for UCI move string (no board state)
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
    
    // Promotion
    int has_promo = 0; PieceType promo_piece = PAWN;
    if (strlen(move_str) == 5) {
        has_promo = 1;
        switch (move_str[4]) {
            case 'q': flags = (uint8_t)(PROMOTION | 3); promo_piece = QUEEN; break;
            case 'r': flags = (uint8_t)(PROMOTION | 2); promo_piece = ROOK; break;
            case 'b': flags = (uint8_t)(PROMOTION | 1); promo_piece = BISHOP; break;
            case 'n': flags = (uint8_t)(PROMOTION | 0); promo_piece = KNIGHT; break;
            default: break;
        }
    }
    
    // Determine moving piece and color from current board
    PieceType moving_piece = board_get_piece_at(&current_board, from);
    Color moving_color = board_get_color_at(&current_board, from);
    
    // Castling detection by king move pattern
    if (moving_piece == KING) {
        if ((from == E1 && to == G1) || (from == E8 && to == G8)) {
            flags = KING_CASTLE;
        } else if ((from == E1 && to == C1) || (from == E8 && to == C8)) {
            flags = QUEEN_CASTLE;
        }
    }
    
    // En passant detection: pawn moves diagonally to an empty square
    if (moving_piece == PAWN && from_file != to_file && !board_is_square_occupied(&current_board, to)) {
        if (current_board.en_passant == to) {
            flags = EN_PASSANT;
        }
    }

    // Double pawn push detection
    if (moving_piece == PAWN && from_file == to_file && flags == QUIET) {
        if ((moving_color == WHITE && to_rank - from_rank == 2) ||
            (moving_color == BLACK && from_rank - to_rank == 2)) {
            flags = DOUBLE_PAWN_PUSH;
        }
    }
    
    // Best-effort capture detection using current board (skip if castle or en passant already set)
    if (flags != KING_CASTLE && flags != QUEEN_CASTLE && flags != EN_PASSANT) {
        if (board_is_square_occupied(&current_board, to) &&
            board_get_color_at(&current_board, to) == color_opposite(moving_color)) {
            if (flags == PROMOTION) flags = PROMOTION_CAPTURE;
            else flags = CAPTURE;
        }
    }
    
    // Prefer returning the exact legal move object (handles promotion variants)
    Move exact = find_matching_legal(&current_board, from, to, has_promo, promo_piece);
    if (!move_equal(exact, NULL_MOVE)) return exact;
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
    stop_search = 1;
    printf("info string Search stopped\n");
    fflush(stdout);
}

// Print UCI info
void uci_info(const char* info) {
    printf("info %s\n", info);
} 

void uci_setoption(const char* command) {
    // Example: setoption name Hash value 128
    const char* name = strstr(command, "name");
    if (!name) return;
    name += 4;
    while (*name && isspace(*name)) name++;
    const char* value = strstr(command, "value");
    int val = 0;
    if (value) {
        value += 5;
        while (*value && isspace(*value)) value++;
        val = atoi(value);
    }
    if (strncasecmp(name, "hash", 4) == 0) {
        if (val > 0) hash_size_mb = val;
    } else if (strncasecmp(name, "ownbook", 7) == 0) {
        own_book = val;
    }
    // Accept all options, even if not used
}
