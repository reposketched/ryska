@echo off
echo Building Chess Engine...

if not exist obj mkdir obj

gcc -Wall -Wextra -O2 -std=c99 -c src/constants.c -o obj/constants.o
gcc -Wall -Wextra -O2 -std=c99 -c src/move.c -o obj/move.o
gcc -Wall -Wextra -O2 -std=c99 -c src/bitboard.c -o obj/bitboard.o
gcc -Wall -Wextra -O2 -std=c99 -c src/board.c -o obj/board.o
gcc -Wall -Wextra -O2 -std=c99 -c src/movegen.c -o obj/movegen.o
gcc -Wall -Wextra -O2 -std=c99 -c src/evaluation.c -o obj/evaluation.o
gcc -Wall -Wextra -O2 -std=c99 -c src/search.c -o obj/search.o
gcc -Wall -Wextra -O2 -std=c99 -c src/uci.c -o obj/uci.o
gcc -Wall -Wextra -O2 -std=c99 -c src/main.c -o obj/main.o

gcc obj/*.o -o chess_engine.exe

echo Build complete! Run chess_engine.exe to start the engine. 