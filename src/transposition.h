#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H

#include "constants.h"
#include "board.h"
#include "move.h"

#define TT_SIZE (1 << 20)  // 1M entries
#define TT_MASK (TT_SIZE - 1)

typedef enum {
    TT_EXACT = 0,
    TT_ALPHA = 1,
    TT_BETA = 2
} TTFlag;

typedef struct {
    uint64_t key;
    Move best_move;
    int score;
    int depth;
    TTFlag flag;
    uint8_t age;
} TTEntry;

void tt_init(void);
void tt_clear(void);
void tt_store(uint64_t key, Move best_move, int score, int depth, TTFlag flag);
TTEntry* tt_probe(uint64_t key);
void tt_age(void);

#endif // TRANSPOSITION_H 