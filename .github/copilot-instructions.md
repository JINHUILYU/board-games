# Copilot Instructions for `board-games`

## Build and run

Use CMake from the repository root:

```bash
cmake -S . -B build
cmake --build build
./build/chess
```

## Test and lint

There are currently no automated test targets (`add_test`) and no lint configuration in this repository.

If tests are added later via CTest, run all tests with:

```bash
ctest --test-dir build
```

Run a single test by name with:

```bash
ctest --test-dir build -R "<test-name>"
```

## High-level architecture

The executable starts in `main.cpp` and immediately delegates control to `chess::ui::RunMainMenu()` (`src/ui/menu.cpp`). The menu is the integration point that wires modules together:

- **Game loop orchestration**: `TurnEngine::Run(...)` (`src/core/turn_engine.cpp`) drives turn-by-turn execution through the `IGameState` and `IPlayer` interfaces.
- **Game implementation (current)**: Gomoku is implemented in `GomokuState` (`src/games/gomoku/gomoku_state.cpp`) and can be played human-vs-human or human-vs-AI.
- **Players**:
  - `HumanConsolePlayer` handles input parsing/validation.
  - `GomokuAiPlayer` performs bounded minimax with alpha-beta pruning plus candidate-move pruning from `GomokuState::CandidateMoves(...)`.
- **Persistence/replay**: `HistoryStore` (`src/storage/history_store.cpp`) writes/reads Gomoku records under `output/history`, and replay is implemented in menu flow.
- **Planned module**: Xiangqi is currently a stub (`src/games/xiangqi/xiangqi_stub.cpp`). The target game uses a fixed 10x9 board, and CLI gameplay is not implemented yet.

## Codebase-specific conventions

- **Namespace layout**: core domain code uses `namespace chess`; menu entrypoints are in `namespace chess::ui`.
- **Interface-first turn engine**: new games/players should integrate through `IGameState` and `IPlayer` so they can be executed by `TurnEngine` without special-case branching.
- **Coordinate convention**: internal board coordinates are zero-based (`row`, `col`), while console input/output is one-based. UI/player code is responsible for conversion.
- **Side and stone mapping**:
  - `Side::kBlack` moves first.
  - Gomoku stones are encoded as `'X'` (black) and `'O'` (white).
- **Move integrity**: `TurnEngine` sets `move.side` from `state.CurrentSide()` and throws on illegal moves. Player implementations should return positions only and rely on engine/state legality checks.
- **History file contract**:
  - file name prefix: `gomoku_YYYYMMDD_HHMMSS.json`
  - top-level keys include `version`, `game`, `mode`, `board_size`, `difficulty`, `result`, `moves`
  - each move is stored as `{"r": <int>, "c": <int>, "p": "B"|"W"}`
  - replay and loader logic depends on this shape
- **UI scope**: current product scope is CLI menu + text board rendering; GUI is out of scope for current milestones.
