#ifndef SEARCH_H
#define SEARCH_H

#include "constants.h"
#include "board.h"
#include "move.h"
#include "evaluation.h"
#include "transposition.h"

#define MAX_DEPTH 64
#define MAX_MOVES 256
#define MATE_SCORE 10000
#define INFINITY 20000

// Search parameters
#define NULL_MOVE_R 3
#define FUTILITY_MARGIN 100
#define RAZOR_MARGIN 300

typedef struct {
    Move best_move;
    int score;
    int depth;
    int nodes;
    int time_ms;
} SearchResult;

typedef struct {
    int wtime;
    int btime;
    int winc;
    int binc;
    int movestogo;
    int depth;
    int nodes;
    int movetime;
    int infinite;
} TimeControl;

// Main search functions
SearchResult search_position(const Board* board, int depth);
SearchResult iterative_deepening(const Board* board, int max_depth, const TimeControl* tc);
int alpha_beta_search(const Board* board, int depth, int alpha, int beta, int* nodes);
int quiescence_search(const Board* board, int alpha, int beta, int* nodes);

// Advanced search techniques
int null_move_search(const Board* board, int depth, int alpha, int beta, int* nodes);
int futility_pruning(const Board* board, int depth, int alpha, int beta);
int razor_pruning(const Board* board, int depth, int alpha, int beta);

// Move ordering
void order_moves(const Board* board, Move* moves, int count);
int get_move_score(const Board* board, Move move);
void update_history(const Board* board, Move move, int depth);

// Time management
void init_time_control(TimeControl* tc);
int should_stop_search(const TimeControl* tc);
int get_search_time(const TimeControl* tc);

// History heuristic
extern int history_table[2][6][64];

#endif // SEARCH_H 