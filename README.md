# Simple Chess Engine

A UCI-compliant chess engine written in C using bitboards for efficient move generation and evaluation.

## Features

- **UCI Protocol**: Full UCI (Universal Chess Interface) compliance
- **Bitboard Representation**: Efficient 64-bit board representation
- **Move Generation**: Legal move generation for all piece types
- **Search**: Alpha-beta search with quiescence
- **Evaluation**: Material and positional evaluation
- **Move Ordering**: MVV-LVA move ordering for better pruning

## Building

To build the engine:

```bash
make
```

This will create the `chess_engine` executable.

## Usage

### Running the Engine

```bash
./chess_engine
```

The engine will start and wait for UCI commands on stdin.

### Testing with a Chess GUI

You can use the engine with any UCI-compatible chess GUI like:
- Arena
- Cutechess
- Fritz
- Chessbase

### Manual Testing

```bash
# Test basic UCI commands
echo "uci" | ./chess_engine
echo "isready" | ./chess_engine
echo "position startpos" | ./chess_engine
echo "go depth 4" | ./chess_engine
```

## UCI Commands Supported

- `uci` - Engine identification
- `isready` - Engine ready status
- `ucinewgame` - Start new game
- `position [fen <fenstring> | startpos] moves <move1> ... <movei>` - Set position
- `go [depth <x>] [movetime <x>]` - Start search
- `stop` - Stop search
- `quit` - Quit engine

## Architecture

### Core Components

1. **Bitboards** (`bitboard.h/c`): 64-bit board representation and operations
2. **Board** (`board.h/c`): Board state management and FEN parsing
3. **Move Generation** (`movegen.h/c`): Legal move generation for all pieces
4. **Evaluation** (`evaluation.h/c`): Position evaluation with piece-square tables
5. **Search** (`search.h/c`): Alpha-beta search with quiescence
6. **UCI Interface** (`uci.h/c`): UCI protocol implementation

### Key Features

- **Efficient Move Generation**: Uses bitboards for fast move generation
- **Legal Move Validation**: All generated moves are legal
- **Position Evaluation**: Material and positional scoring
- **Search Algorithm**: Alpha-beta with quiescence search
- **Move Ordering**: Captures ordered by MVV-LVA for better pruning

## Limitations

This is a basic chess engine implementation. Some limitations include:

- No transposition table
- No opening book
- Simplified evaluation function
- No advanced search techniques (null move, futility pruning, etc.)
- Limited time management
- No parallel search

## Future Improvements

- Add transposition table
- Implement more advanced search techniques
- Add opening book support
- Improve evaluation function
- Add parallel search
- Better time management
- Add endgame tablebases

## License

This project is open source and available under the MIT License. 