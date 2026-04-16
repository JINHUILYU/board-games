#include "ui/human_console_player.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include "games/gomoku/gomoku_state.h"

namespace chess {

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
    if (const auto* gomoku = dynamic_cast<const GomokuState*>(&state)) {
        board_size = gomoku->BoardSize();
    }

    while (true) {
        std::cout << name_ << " (" << SideToString(state.CurrentSide()) << ") "
                  << "input row col [1-" << board_size << "]: ";

        std::string line;
        if (!std::getline(std::cin, line)) {
            throw std::runtime_error("Input stream closed.");
        }

        std::istringstream iss(line);
        int row = 0;
        int col = 0;
        if (!(iss >> row >> col)) {
            std::cout << "Invalid input, expected: row col\n";
            continue;
        }
        row -= 1;
        col -= 1;
        if (!state.IsMoveLegal(row, col)) {
            std::cout << "Illegal move, please retry.\n";
            continue;
        }
        return Move{row, col, state.CurrentSide()};
    }
}

}  // namespace chess
