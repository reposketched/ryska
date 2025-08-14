#include "book.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

typedef struct {
    char moves[256];
    char bestmove[16];
    int weight;
} BookEntry;

#define MAX_BOOK_ENTRIES 4096
static BookEntry g_book[MAX_BOOK_ENTRIES];
static int g_book_count = 0;
static char g_last_move_buf[16];

static void trim_str(char* s) {
    // left trim
    char* p = s;
    while (*p && isspace((unsigned char)*p)) p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
    // right trim
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n-1])) s[--n] = 0;
}

void book_clear(void) {
    g_book_count = 0;
}

static int parse_line(const char* line, char* out_moves, char* out_move, int* out_weight) {
    // expected: <moves> -> <bestmove> [# weight]
    const char* arrow = strstr(line, "->");
    if (!arrow) return 0;
    size_t left_len = (size_t)(arrow - line);
    if (left_len >= 255) left_len = 255;
    strncpy(out_moves, line, left_len);
    out_moves[left_len] = 0;
    trim_str(out_moves);

    const char* right = arrow + 2;
    while (*right && isspace((unsigned char)*right)) right++;
    // copy until space or end or '#'
    size_t i = 0;
    while (right[i] && !isspace((unsigned char)right[i]) && right[i] != '#') {
        if (i < 15) out_move[i] = right[i];
        i++;
    }
    if (i == 0) return 0;
    out_move[i < 15 ? i : 15] = 0;

    int weight = 100;
    const char* hash = strchr(right, '#');
    if (hash) {
        // try to parse integer after '#'
        while (*hash && !isdigit((unsigned char)*hash)) hash++;
        if (*hash) {
            weight = atoi(hash);
        }
    }
    if (out_weight) *out_weight = weight;
    return 1;
}

int book_load(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) return -1;
    g_book_count = 0;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        // skip comments and blank
        char* p = line;
        while (*p && isspace((unsigned char)*p)) p++;
        if (*p == '\0' || *p == '#') continue;
        char moves[256];
        char best[16];
        int weight = 100;
        if (!parse_line(p, moves, best, &weight)) continue;
        if (g_book_count < MAX_BOOK_ENTRIES) {
            strncpy(g_book[g_book_count].moves, moves, sizeof(g_book[g_book_count].moves)-1);
            g_book[g_book_count].moves[sizeof(g_book[g_book_count].moves)-1] = 0;
            strncpy(g_book[g_book_count].bestmove, best, sizeof(g_book[g_book_count].bestmove)-1);
            g_book[g_book_count].bestmove[sizeof(g_book[g_book_count].bestmove)-1] = 0;
            g_book[g_book_count].weight = weight;
            g_book_count++;
        }
    }
    fclose(f);
    return g_book_count;
}

const char* book_get_move(const char* moves_sequence) {
    // exact match first
    for (int i = 0; i < g_book_count; i++) {
        if (strcmp(g_book[i].moves, moves_sequence) == 0) {
            strncpy(g_last_move_buf, g_book[i].bestmove, sizeof(g_last_move_buf)-1);
            g_last_move_buf[sizeof(g_last_move_buf)-1] = 0;
            return g_last_move_buf;
        }
    }
    // try trimmed whitespace normalization
    char norm[256];
    strncpy(norm, moves_sequence, sizeof(norm)-1);
    norm[sizeof(norm)-1] = 0;
    trim_str(norm);
    for (int i = 0; i < g_book_count; i++) {
        if (strcmp(g_book[i].moves, norm) == 0) {
            strncpy(g_last_move_buf, g_book[i].bestmove, sizeof(g_last_move_buf)-1);
            g_last_move_buf[sizeof(g_last_move_buf)-1] = 0;
            return g_last_move_buf;
        }
    }
    return NULL;
}


