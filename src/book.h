#ifndef BOOK_H
#define BOOK_H

#include "constants.h"

// Simple opening book API
// Human-editable format: each non-empty line not starting with '#' has the form
//   <moves> -> <bestmove> [# weight]
// Examples:
//   (empty) -> e2e4 # 100
//   e2e4 -> e7e5 # 90
//   e2e4 e7e5 -> g1f3 # 80

void book_clear(void);
int  book_load(const char* path);          // returns number of entries loaded, or -1 on error
const char* book_get_move(const char* moves_sequence); // returns static pointer valid until next load

#endif // BOOK_H


