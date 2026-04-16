#include "ui/menu.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#include "ai/gomoku_ai_player.h"
#include "core/turn_engine.h"
#include "games/gomoku/gomoku_state.h"
#include "games/xiangqi/xiangqi_state.h"
#include "storage/history_store.h"
#include "ui/human_console_player.h"

namespace chess::ui {

namespace {

std::string XiangqiResultToString(GameResult result) {
    switch (result) {
        case GameResult::kBlackWin:
            return "BlackWin";
        case GameResult::kWhiteWin:
            return "RedWin";
        case GameResult::kDraw:
            return "Draw";
        case GameResult::kOngoing:
        default:
            return "Ongoing";
    }
}

int ReadIntInRange(const std::string& prompt, int min_value, int max_value) {
    while (true) {
        std::cout << prompt;
        std::string line;
        if (!std::getline(std::cin, line)) {
            throw std::runtime_error("Input stream closed.");
        }
        std::istringstream iss(line);
        int value = 0;
        char trailing = '\0';
        if ((iss >> value) && !(iss >> trailing) && value >= min_value && value <= max_value) {
            return value;
        }
        std::cout << "Invalid choice. Expected range: " << min_value << " - " << max_value << "\n";
    }
}

void StartGomokuGame(bool vs_ai, int difficulty, int board_size, const HistoryStore& history_store) {
    GomokuState state(board_size);
    HumanConsolePlayer black_player("Player-Black");
    std::unique_ptr<IPlayer> white_player;
    if (vs_ai) {
        white_player = std::make_unique<GomokuAiPlayer>(difficulty);
    } else {
        white_player = std::make_unique<HumanConsolePlayer>("Player-White");
    }

    auto result = TurnEngine::Run(
        state,
        black_player,
        *white_player,
        [](const IGameState& current_state) {
            std::cout << "\n" << current_state.Render() << "\n";
        });

    std::cout << "Game finished: " << GameResultToString(result.result) << "\n";
    GomokuHistoryRecord record;
    record.mode = vs_ai ? "human_vs_ai" : "human_vs_human";
    record.board_size = board_size;
    record.difficulty = vs_ai ? difficulty : 0;
    record.result = result.result;
    record.moves = std::move(result.moves);
    const auto saved = history_store.SaveGomoku(record);
    std::cout << "History saved: " << saved << "\n";
}

void ReplayGomokuHistory(const HistoryStore& history_store) {
    const auto files = history_store.ListGomokuRecords();
    if (files.empty()) {
        std::cout << "No gomoku history found.\n";
        return;
    }

    std::cout << "Gomoku history files:\n";
    for (std::size_t i = 0; i < files.size(); ++i) {
        std::cout << "  " << (i + 1) << ") " << files[i].filename().string() << "\n";
    }
    const int index = ReadIntInRange("Select history index: ", 1, static_cast<int>(files.size())) - 1;

    GomokuHistoryRecord record;
    if (!history_store.LoadGomoku(files[index], &record)) {
        std::cout << "Failed to load history.\n";
        return;
    }

    GomokuState replay_state(record.board_size);
    std::cout << "\nReplaying: " << files[index].filename().string()
              << " (board " << record.board_size << "x" << record.board_size << ")\n";
    for (std::size_t i = 0; i < record.moves.size(); ++i) {
        const Move& move = record.moves[i];
        replay_state.ApplyMove(move);
        std::cout << "Move " << (i + 1) << ": " << SideToString(move.side)
                  << " -> (" << (move.row + 1) << ", " << (move.col + 1) << ")\n";
    }

    std::cout << "\nFinal board:\n" << replay_state.Render() << "\n";
    std::cout << "Stored result: " << GameResultToString(record.result) << "\n";
}

void StartXiangqiGame() {
    XiangqiState state;
    HumanConsolePlayer black_player("Player-Black");
    HumanConsolePlayer white_player("Player-Red");

    auto result = TurnEngine::Run(
        state,
        black_player,
        white_player,
        [](const IGameState& current_state) {
            std::cout << "\n" << current_state.Render() << "\n";
        });

    std::cout << "Xiangqi finished: " << XiangqiResultToString(result.result) << "\n";
}

}  // namespace

void RunMainMenu() {
    HistoryStore history_store("output/history");

    while (true) {
        std::cout << "=== Chess Game Platform ===\n";
        std::cout << "1) Gomoku: Human vs Human\n";
        std::cout << "2) Gomoku: Human vs AI\n";
        std::cout << "3) Gomoku: Replay History\n";
        std::cout << "4) Xiangqi (fixed 10x9)\n";
        std::cout << "0) Exit\n";

        const int choice = ReadIntInRange("Choose: ", 0, 4);
        if (choice == 0) {
            std::cout << "Bye.\n";
            return;
        }

        try {
            if (choice == 1) {
                const int board_size = ReadIntInRange("Board size (10-19): ", 10, 19);
                StartGomokuGame(false, 0, board_size, history_store);
            } else if (choice == 2) {
                const int difficulty = ReadIntInRange("AI difficulty (1-3): ", 1, 3);
                const int board_size = ReadIntInRange("Board size (10-19): ", 10, 19);
                StartGomokuGame(true, difficulty, board_size, history_store);
            } else if (choice == 3) {
                ReplayGomokuHistory(history_store);
            } else if (choice == 4) {
                StartXiangqiGame();
            }
        } catch (const std::exception& ex) {
            std::cout << "Error: " << ex.what() << "\n";
        }
    }
}

}  // namespace chess::ui
