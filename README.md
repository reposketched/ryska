# Chess Engine

A UCI-compliant chess engine written in C using bitboards for efficient move generation and evaluation.

## Features

- **UCI Protocol**: Full UCI (Universal Chess Interface) compliance
- **Bitboard Representation**: Efficient 64-bit board representation
- **Move Generation**: Legal move generation for all piece types
- **Advanced Search**: Alpha-beta search with quiescence, null move pruning, futility pruning, and razor pruning
- **Evaluation**: Material and positional evaluation with piece-square tables
- **Move Ordering**: MVV-LVA move ordering with history heuristic
- **Transposition Table**: 1M entry hash table for position caching
- **Opening Book**: Human-editable opening book support
- **Time Management**: Basic time control and move time management
- **Iterative Deepening**: Progressive depth search with time constraints

## Building

### Prerequisites

- GCC compiler (version 4.9 or later)
- Make (for Unix/Linux/macOS)
- Windows Command Prompt or PowerShell (for Windows)
- Git (installation)

### Build Instructions

#### Windows
```bash
git clone https://github.com/reposketched/ryska.git # or download the code directly
# Using the provided batch file
./build.bat

# Or manually with GCC
gcc -Wall -Wextra -O2 -std=c99 -c src/*.c -o obj/*.o
gcc obj/*.o -o chess_engine.exe
```

#### Unix/Linux/macOS
```bash
# Clone the repository
git clone https://github.com/reposketched/ryska.git
cd chess-engine

# Build using Make
make

# Or manually with GCC
mkdir -p obj
gcc -Wall -Wextra -O2 -std=c99 -c src/*.c -o obj/*.o
gcc obj/*.o -o chess_engine
```

### Build Output

The build process creates:
- `chess_engine` (Unix/Linux/macOS) or `chess_engine.exe` (Windows)
- `obj/` directory with compiled object files
- `book.txt` opening book file (if missing)

## Usage

### Running the Engine

```bash
# Start the engine
./chess_engine          # Unix/Linux/macOS
chess_engine.exe        # Windows
```

The engine will start and wait for UCI commands on stdin.

### Testing with a Chess GUI

You can use the engine with any UCI-compatible chess GUI:
- **Arena** - Free chess GUI
- **Cutechess** - Cross-platform chess GUI
- **Fritz** - Commercial chess software
- **Chessbase** - Professional chess database
- **Lichess** - Online chess platform (via UCI engine)

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
- `go [depth <x>] [movetime <x>] [wtime <x>] [btime <x>] [winc <x>] [binc <x>]` - Start search
- `stop` - Stop search
- `quit` - Quit engine

## Architecture

### Core Components

1. **Bitboards** (`bitboard.h/c`): 64-bit board representation and operations
2. **Board** (`board.h/c`): Board state management and FEN parsing
3. **Move Generation** (`movegen.h/c`): Legal move generation for all pieces
4. **Evaluation** (`evaluation.h/c`): Position evaluation with piece-square tables and mobility
5. **Search** (`search.h/c`): Advanced search with multiple pruning techniques
6. **Transposition Table** (`transposition.h/c`): Position caching for improved performance
7. **Opening Book** (`book.h/c`): Human-editable opening move database
8. **UCI Interface** (`uci.h/c`): UCI protocol implementation

### Advanced Features

- **Null Move Pruning**: Reduces search depth for null moves
- **Futility Pruning**: Prunes moves unlikely to improve alpha
- **Razor Pruning**: Aggressive pruning for deep searches
- **History Heuristic**: Tracks move success for better ordering
- **Iterative Deepening**: Progressive depth search with time management
- **Piece-Square Tables**: Positional bonuses for piece placement
- **Mobility Evaluation**: Considers piece mobility in evaluation

### Search Parameters

- **Maximum Depth**: 64 plies
- **Transposition Table**: 1M entries
- **Null Move Reduction**: 2 plies
- **Futility Margin**: 150 centipawns
- **Razor Margin**: 400 centipawns

## Performance

The engine is optimized for:
- **Speed**: Efficient bitboard operations and move generation
- **Memory**: Compact data structures and efficient caching
- **Accuracy**: Legal move generation and proper evaluation

## Limitations

Current limitations include:
- No parallel search (single-threaded)
- Basic time management
- No endgame tablebases
- Simplified evaluation compared to commercial engines

## Future Improvements

Planned enhancements:
- Multi-threading support
- Advanced time management
- Endgame tablebase integration
- Neural network evaluation
- Improved opening book format
- Better move ordering heuristics

## Development

### Project Structure
```
chess-engine/
├── src/           # Source code
├── obj/           # Compiled objects
├── build.bat      # Windows build script
├── Makefile       # Unix build configuration
├── book.txt       # Opening book
└── README.md      # This file
```

### Compilation Flags
- `-Wall -Wextra`: Enable all warnings
- `-O2`: Optimize for speed
- `-std=c99`: Use C99 standard

## License

This project is open source and available under the MIT License. 