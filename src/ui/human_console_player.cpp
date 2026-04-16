#include "ui/human_console_player.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include "games/gomoku/gomoku_state.h"
#include "games/xiangqi/xiangqi_state.h"

namespace chess {

namespace {

std::string DisplaySideName(const IGameState& state, Side side) {
    if (dynamic_cast<const XiangqiState*>(&state) != nullptr) {
        return side == Side::kBlack ? "Black" : "Red";
    }
    return SideToString(side);
}

}  // namespace

HumanConsolePlayer::HumanConsolePlayer(std::string name)
    : name_(std::move(name)) {}

std::string HumanConsolePlayer::Name() const {
    return name_;
}

bool HumanConsolePlayer::IsHuman() const {
    return true;
}

Move HumanConsolePlayer::ChooseMove(const IGameState& state) {
    int board_size = 15;
    const bool is_xiangqi = dynamic_cast<const XiangqiState*>(&state) != nullptr;
    if (const auto* gomoku = dynamic_cast<const GomokuState*>(&state)) {
        board_size = gomoku->BoardSize();
    }

    while (true) {
        std::cout << name_ << " (" << DisplaySideName(state, state.CurrentSide()) << ") ";
        if (is_xiangqi) {
            std::cout << "input from_row from_col to_row to_col [row 1-10, col 1-9]: ";
        } else {
            std::cout << "input row col [1-" << board_size << "]: ";
        }

        std::string line;
        if (!std::getline(std::cin, line)) {
            throw std::runtime_error("Input stream closed.");
        }

        std::istringstream iss(line);
        Move move;
        if (is_xiangqi) {
            int from_row = 0;
            int from_col = 0;
            int to_row = 0;
            int to_col = 0;
            if (!(iss >> from_row >> from_col >> to_row >> to_col)) {
                std::cout << "Invalid input, expected: from_row from_col to_row to_col\n";
                continue;
            }
            move = Move{from_row - 1, from_col - 1, to_row - 1, to_col - 1, state.CurrentSide()};
        } else {
            int row = 0;
            int col = 0;
            if (!(iss >> row >> col)) {
                std::cout << "Invalid input, expected: row col\n";
                continue;
            }
            move = Move{-1, -1, row - 1, col - 1, state.CurrentSide()};
        }

        if (!state.IsMoveLegal(move)) {
            std::cout << "Illegal move, please retry.\n";
            continue;
        }
        return move;
    }
}

}  // namespace chess
