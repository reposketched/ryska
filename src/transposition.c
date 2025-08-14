#include "transposition.h"
#include <stddef.h>
#include <string.h>

static TTEntry tt_table[TT_SIZE];
static uint8_t tt_age_counter = 0;

// Zobrist keys for hashing
static uint64_t zobrist_pieces[2][6][64];
static uint64_t zobrist_side;
static uint64_t zobrist_castling[16];
static uint64_t zobrist_ep[64];

// Initialize transposition table and Zobrist keys
void tt_init(void) {
    tt_clear();
    
    // Initialize Zobrist keys with simple hash values
    uint64_t seed = 0x123456789ABCDEF0ULL;
    for (Color c = WHITE; c < COLOR_COUNT; c++) {
        for (PieceType pt = PAWN; pt < PIECE_COUNT; pt++) {
            for (Square sq = A1; sq < SQUARE_COUNT; sq++) {
                seed = seed * 1103515245ULL + 12345ULL;
                zobrist_pieces[c][pt][sq] = seed;
            }
        }
    }
    
    seed = seed * 1103515245ULL + 12345ULL;
    zobrist_side = seed;
    
    for (int i = 0; i < 16; i++) {
        seed = seed * 1103515245ULL + 12345ULL;
        zobrist_castling[i] = seed;
    }
    
    for (Square sq = A1; sq < SQUARE_COUNT; sq++) {
        seed = seed * 1103515245ULL + 12345ULL;
        zobrist_ep[sq] = seed;
    }
}

void tt_clear(void) {
    for (int i = 0; i < TT_SIZE; i++) {
        tt_table[i].key = 0;
        tt_table[i].best_move = NULL_MOVE;
        tt_table[i].score = 0;
        tt_table[i].depth = 0;
        tt_table[i].flag = TT_EXACT;
        tt_table[i].age = 0;
    }
}

void tt_store(uint64_t key, Move best_move, int score, int depth, TTFlag flag) {
    uint32_t index = key & TT_MASK;
    TTEntry* entry = &tt_table[index];
    
    // Replace if entry is empty, older, or shallower
    if (entry->key == 0 || entry->age < tt_age_counter || entry->depth <= depth) {
        entry->key = key;
        entry->best_move = best_move;
        entry->score = score;
        entry->depth = depth;
        entry->flag = flag;
        entry->age = tt_age_counter;
    }
}

TTEntry* tt_probe(uint64_t key) {
    uint32_t index = key & TT_MASK;
    TTEntry* entry = &tt_table[index];
    
    if (entry->key == key) {
        return entry;
    }
    
    return NULL;
}

void tt_age(void) {
    tt_age_counter++;
}

// Generate Zobrist hash for a position
uint64_t generate_hash(const Board* board) {
    uint64_t hash = 0;
    
    // Hash pieces
    for (Color c = WHITE; c < COLOR_COUNT; c++) {
        for (PieceType pt = PAWN; pt < PIECE_COUNT; pt++) {
            Bitboard pieces = board->pieces[c][pt];
            while (pieces) {
                Square sq = pop_lsb(&pieces);
                hash ^= zobrist_pieces[c][pt][sq];
            }
        }
    }
    
    // Hash side to move
    if (board->side_to_move == BLACK) {
        hash ^= zobrist_side;
    }
    
    // Hash castling rights
    hash ^= zobrist_castling[board->castling_rights];
    
    // Hash en passant square
    if (board->en_passant != A1) {
        hash ^= zobrist_ep[board->en_passant];
    }
    
    return hash;
} 