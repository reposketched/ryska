#ifndef UCI_H
#define UCI_H

#include "board.h"
#include "search.h"

// UCI commands
void uci_loop(void);
void uci_uci(void);
void uci_isready(void);
void uci_newgame(void);
void uci_position(const char* command);
void uci_go(const char* command);
void uci_quit(void);
void uci_stop(void);

// Move parsing
Move parse_move(const char* move_str);
void print_move(Move move);

// UCI info
void uci_info(const char* info);

#endif // UCI_H 