cmake_minimum_required(VERSION 3.16)
project(board_games_cli LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(chess_cli
    main.cpp
    src/core/turn_engine.cpp
    src/games/gomoku/gomoku_state.cpp
    src/ai/gomoku_ai_player.cpp
    src/ui/human_console_player.cpp
    src/storage/history_store.cpp
    src/ui/menu.cpp
    src/games/xiangqi/xiangqi_stub.cpp
)

target_include_directories(chess_cli PRIVATE include)
