#include "search.h"
#include "movegen.h"
#include <stdio.h>
#include <string.h>

// Global search state
static int search_start_time;
static int search_stop_time;
int history_table[2][6][64];

// Search the position to the given depth
SearchResult search_position(const Board* board, int depth) {
    SearchResult result;
    result.best_move = NULL_MOVE;
    result.score = 0;
    result.depth = depth;
    result.nodes = 0;
    result.time_ms = 0;
    
    if (depth <= 0) return result;
    
    // Generate all moves
    Move moves[MAX_MOVES];
    int move_count = generate_moves(board, moves);
    // Filter to legal moves
    int legal_count = 0;
    for (int i = 0; i < move_count; i++) {
        if (is_legal_move(board, moves[i])) moves[legal_count++] = moves[i];
    }
    move_count = legal_count;
    
    if (move_count == 0) {
        // Checkmate or stalemate
        if (board_is_check(board)) {
            result.score = -MATE_SCORE;
        } else {
            result.score = 0;
        }
        return result;
    }
    
    // Order moves
    order_moves(board, moves, move_count);
    
    // Search with alpha-beta
    int alpha = -INFINITY;
    int beta = INFINITY;
    int best_score = -INFINITY;
    
    for (int i = 0; i < move_count; i++) {
        Board temp_board = *board;
        board_make_move(&temp_board, moves[i]);
        
        int score = -alpha_beta_search(&temp_board, depth - 1, -beta, -alpha, &result.nodes);
        
        if (score > best_score) {
            best_score = score;
            result.best_move = moves[i];
        }
        
        if (score > alpha) {
            alpha = score;
        }
        
        if (alpha >= beta) {
            break; // Beta cutoff
        }
    }
    
    result.score = best_score;
    return result;
}

// Iterative deepening with time management
SearchResult iterative_deepening(const Board* board, int max_depth, const TimeControl* tc) {
    SearchResult result;
    result.best_move = NULL_MOVE;
    result.score = 0;
    result.depth = 0;
    result.nodes = 0;
    result.time_ms = 0;
    
    // Clear history table
    memset(history_table, 0, sizeof(history_table));
    
    // Start with depth 1 and increase
    for (int depth = 1; depth <= max_depth; depth++) {
        SearchResult current_result = search_position(board, depth);
        
        // Check if we should stop
        if (should_stop_search(tc)) {
            break;
        }
        
        // Update result
        result = current_result;
        result.depth = depth;
        
        // Print info
        printf("info depth %d score %d nodes %d\n", 
               depth, result.score, result.nodes);
    }
    
    return result;
}

// Alpha-beta search with advanced pruning
int alpha_beta_search(const Board* board, int depth, int alpha, int beta, int* nodes) {
    if (nodes) (*nodes)++;
    
    // Check for terminal positions
    if (board_is_checkmate(board)) {
        return -MATE_SCORE;
    }
    
    if (board_is_stalemate(board)) {
        return 0;
    }
    
    // Transposition table lookup
    uint64_t hash = generate_hash(board);
    TTEntry* tt_entry = tt_probe(hash);
    if (tt_entry && tt_entry->depth >= depth) {
        if (tt_entry->flag == TT_EXACT) {
            return tt_entry->score;
        } else if (tt_entry->flag == TT_ALPHA && tt_entry->score <= alpha) {
            return alpha;
        } else if (tt_entry->flag == TT_BETA && tt_entry->score >= beta) {
            return beta;
        }
    }
    
    // Null move pruning
    if (depth >= NULL_MOVE_R && !board_is_check(board)) {
        int null_score = null_move_search(board, depth - NULL_MOVE_R, alpha, beta, nodes);
        if (null_score >= beta) {
            return beta;
        }
    }
    
    // Futility pruning
    if (depth <= 3 && !board_is_check(board)) {
        int futility_score = futility_pruning(board, depth, alpha, beta);
        if (futility_score != 0) {
            return futility_score;
        }
    }
    
    // Razor pruning
    if (depth <= 2 && !board_is_check(board)) {
        int razor_score = razor_pruning(board, depth, alpha, beta);
        if (razor_score != 0) {
            return razor_score;
        }
    }
    
    // Quiescence search at leaf nodes
    if (depth <= 0) {
        return quiescence_search(board, alpha, beta, nodes);
    }
    
    // Generate moves
    Move moves[MAX_MOVES];
    int move_count = generate_moves(board, moves);
    // Filter illegal moves
    int legal_count = 0;
    for (int i = 0; i < move_count; i++) {
        if (is_legal_move(board, moves[i])) moves[legal_count++] = moves[i];
    }
    move_count = legal_count;
    
    if (move_count == 0) {
        if (board_is_check(board)) {
            return -MATE_SCORE;
        } else {
            return 0;
        }
    }
    
    // Order moves
    order_moves(board, moves, move_count);
    
    int best_score = -INFINITY;
    Move best_move = NULL_MOVE;
    TTFlag tt_flag = TT_ALPHA;
    
    for (int i = 0; i < move_count; i++) {
        Board temp_board = *board;
        board_make_move(&temp_board, moves[i]);
        
        int score = -alpha_beta_search(&temp_board, depth - 1, -beta, -alpha, nodes);
        
        if (score > best_score) {
            best_score = score;
            best_move = moves[i];
        }
        
        if (score > alpha) {
            alpha = score;
            tt_flag = TT_EXACT;
        }
        
        if (alpha >= beta) {
            tt_flag = TT_BETA;
            update_history(board, moves[i], depth);
            break; // Beta cutoff
        }
    }
    
    // Store in transposition table
    tt_store(hash, best_move, best_score, depth, tt_flag);
    
    return best_score;
}

// Null move search
int null_move_search(const Board* board, int depth, int alpha, int beta, int* nodes) {
    Board temp_board = *board;
    temp_board.side_to_move = color_opposite(temp_board.side_to_move);
    
    return -alpha_beta_search(&temp_board, depth, -beta, -alpha, nodes);
}

// Futility pruning
int futility_pruning(const Board* board, int depth, int alpha, int beta) {
    int eval = evaluate_position(board);
    
    if (eval - FUTILITY_MARGIN * depth >= beta) {
        return beta;
    }
    
    return 0; // Continue search
}

// Razor pruning
int razor_pruning(const Board* board, int depth, int alpha, int beta) {
    int eval = evaluate_position(board);
    
    if (eval + RAZOR_MARGIN <= alpha) {
        // Only search captures
        Move moves[MAX_MOVES];
        int move_count = generate_moves(board, moves);
        
        int best_score = eval;
        for (int i = 0; i < move_count; i++) {
            if (is_capture(moves[i])) {
                Board temp_board = *board;
                board_make_move(&temp_board, moves[i]);
                
                int dummy_nodes = 0; // local counter to avoid undeclared 'nodes'
                int score = -alpha_beta_search(&temp_board, depth - 1, -beta, -alpha, &dummy_nodes);
                if (score > best_score) {
                    best_score = score;
                }
            }
        }
        
        if (best_score <= alpha) {
            return alpha;
        }
    }
    
    return 0; // Continue search
}

// Quiescence search (captures only)
int quiescence_search(const Board* board, int alpha, int beta, int* nodes) {
    if (nodes) (*nodes)++;
    
    int stand_pat = evaluate_position(board);
    
    if (stand_pat >= beta) {
        return beta;
    }
    
    if (alpha < stand_pat) {
        alpha = stand_pat;
    }
    
    // Generate only captures
    Move moves[MAX_MOVES];
    int move_count = generate_moves(board, moves);
    
    // Filter to legal captures only
    int capture_count = 0;
    for (int i = 0; i < move_count; i++) {
        if (is_capture(moves[i]) && is_legal_move(board, moves[i])) {
            moves[capture_count++] = moves[i];
        }
    }
    
    if (capture_count == 0) {
        return stand_pat;
    }
    
    // Order captures
    order_moves(board, moves, capture_count);
    
    for (int i = 0; i < capture_count; i++) {
        Board temp_board = *board;
        board_make_move(&temp_board, moves[i]);
        
        int score = -quiescence_search(&temp_board, -beta, -alpha, nodes);
        
        if (score >= beta) {
            return beta;
        }
        
        if (score > alpha) {
            alpha = score;
        }
    }
    
    return alpha;
}

// Order moves for better alpha-beta pruning
void order_moves(const Board* board, Move* moves, int count) {
    // Get transposition table move
    uint64_t hash = generate_hash(board);
    TTEntry* tt_entry = tt_probe(hash);
    Move tt_move = tt_entry ? tt_entry->best_move : NULL_MOVE;
    
    // Score moves
    int scores[MAX_MOVES];
    for (int i = 0; i < count; i++) {
        scores[i] = get_move_score(board, moves[i]);
        
        // Bonus for transposition table move
        if (moves[i].data == tt_move.data) {
            scores[i] += 10000;
        }
        
        // Bonus for history
        Square from = move_from(moves[i]);
        PieceType piece = PAWN;
        for (PieceType pt = PAWN; pt < PIECE_COUNT; pt++) {
            if (test_bit(board->pieces[board->side_to_move][pt], from)) {
                piece = pt;
                break;
            }
        }
        scores[i] += history_table[board->side_to_move][piece][from];
    }
    
    // Sort moves by score (bubble sort for simplicity)
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (scores[j] > scores[i]) {
                // Swap moves
                Move temp_move = moves[i];
                moves[i] = moves[j];
                moves[j] = temp_move;
                
                // Swap scores
                int temp_score = scores[i];
                scores[i] = scores[j];
                scores[j] = temp_score;
            }
        }
    }
}

// Get move score for ordering (MVV-LVA)
int get_move_score(const Board* board, Move move) {
    Square from = move_from(move);
    Square to = move_to(move);

    // Determine moving piece
    PieceType piece = PAWN;
    for (PieceType pt = PAWN; pt < PIECE_COUNT; pt++) {
        if (test_bit(board->pieces[board->side_to_move][pt], from)) {
            piece = pt;
            break;
        }
    }

    // Captures: MVV-LVA
    if (is_capture(move)) {
        PieceType victim = PAWN;
        for (PieceType pt = PAWN; pt < PIECE_COUNT; pt++) {
            if (test_bit(board->pieces[color_opposite(board->side_to_move)][pt], to)) {
                victim = pt;
                break;
            }
        }
        int victim_values[] = {PAWN_VALUE, KNIGHT_VALUE, BISHOP_VALUE, ROOK_VALUE, QUEEN_VALUE, KING_VALUE};
        int attacker_values[] = {PAWN_VALUE, KNIGHT_VALUE, BISHOP_VALUE, ROOK_VALUE, QUEEN_VALUE, KING_VALUE};
        return 6 * victim_values[victim] - attacker_values[piece];
    }

    // Quiet move ordering heuristics
    int score = 0;

    // Castling highly preferred
    if (is_castle(move)) {
        score += 800;
    }

    // Promotions (should be rare in quiet set, but handle anyway)
    if (is_promotion(move)) {
        PieceType promo = promotion_piece(move);
        score += 10000;
        if (promo == QUEEN) score += 300;
        else if (promo == ROOK) score += 200;
        else if (promo == KNIGHT) score += 150;
        else if (promo == BISHOP) score += 100;
        return score;
    }

    // Encourage central pawn pushes and development
    File to_file = file_of(to);
    Rank to_rank = rank_of(to);
    Rank from_rank = rank_of(from);

    if (piece == PAWN) {
        // Prefer central files for pawn advances
        if (to_file == FILE_D || to_file == FILE_E) score += 200;
        if (to_file == FILE_C || to_file == FILE_F) score += 120;
        if (to_file == FILE_A || to_file == FILE_H) score -= 50;
        // Prefer advancing to 4th rank (white) or 5th (black)
        if ((board->side_to_move == WHITE && to_rank == RANK_4) ||
            (board->side_to_move == BLACK && to_rank == RANK_5)) {
            score += 180;
        }
        // Double pawn push gets small bonus
        if (is_double_pawn_push(move)) score += 60;
    } else if (piece == KNIGHT) {
        // Knights to c3/f3 (or c6/f6)
        if ((to == C3) || (to == F3) || (to == C6) || (to == F6)) score += 220;
        // Knights toward center
        if (to_file >= FILE_C && to_file <= FILE_F && to_rank >= RANK_3 && to_rank <= RANK_6) score += 140;
        // Development off back rank
        if ((board->side_to_move == WHITE && from_rank == RANK_1) || (board->side_to_move == BLACK && from_rank == RANK_8)) score += 80;
    } else if (piece == BISHOP) {
        // Develop bishops off back rank
        if ((board->side_to_move == WHITE && from_rank == RANK_1) || (board->side_to_move == BLACK && from_rank == RANK_8)) score += 90;
        // Slight center preference
        if (to_file >= FILE_C && to_file <= FILE_F) score += 40;
    } else if (piece == ROOK) {
        // Avoid early rook moves unless to open files
        if (to_file == FILE_A || to_file == FILE_H) score -= 20;
    } else if (piece == KING) {
        // Non-castle king moves discouraged
        score -= 100;
    }

    // Add a small PST delta to refine quiet ordering
    switch (piece) {
        case PAWN:   score += pawn_table[to]   - pawn_table[from];   break;
        case KNIGHT: score += knight_table[to] - knight_table[from]; break;
        case BISHOP: score += bishop_table[to] - bishop_table[from]; break;
        case ROOK:   score += rook_table[to]   - rook_table[from];   break;
        case QUEEN:  score += queen_table[to]  - queen_table[from];  break;
        case KING:   score += king_table[to]   - king_table[from];   break;
        default: break;
    }

    // Include history heuristic for quiet moves
    score += history_table[board->side_to_move][piece][from];

    return score;
}

// Update history heuristic
void update_history(const Board* board, Move move, int depth) {
    Square from = move_from(move);
    PieceType piece = PAWN;
    for (PieceType pt = PAWN; pt < PIECE_COUNT; pt++) {
        if (test_bit(board->pieces[board->side_to_move][pt], from)) {
            piece = pt;
            break;
        }
    }
    
    history_table[board->side_to_move][piece][from] += depth * depth;
}

// Time management
void init_time_control(TimeControl* tc) {
    tc->wtime = 0;
    tc->btime = 0;
    tc->winc = 0;
    tc->binc = 0;
    tc->movestogo = 0;
    tc->depth = 0;
    tc->nodes = 0;
    tc->movetime = 0;
    tc->infinite = 0;
}

int should_stop_search(const TimeControl* tc) {
    if (tc->infinite) return 0;
    
    // Simplified time management - always continue for now
    return 0;
}

int get_search_time(const TimeControl* tc) {
    if (tc->movetime > 0) return tc->movetime;
    
    // Simplified time calculation
    int time = tc->wtime; // Use white time as default
    int inc = tc->winc;
    
    if (tc->movestogo > 0) {
        return time / tc->movestogo + inc;
    } else {
        return time / 30 + inc; // Assume 30 moves remaining
    }
} 