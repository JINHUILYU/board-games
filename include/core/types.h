#pragma once

#include <string>

namespace chess {

enum class Side {
    kBlack,
    kWhite
};

inline Side Opponent(Side side) {
    return side == Side::kBlack ? Side::kWhite : Side::kBlack;
}

inline std::string SideToString(Side side) {
    return side == Side::kBlack ? "Black" : "White";
}

enum class GameResult {
    kOngoing,
    kBlackWin,
    kWhiteWin,
    kDraw
};

inline std::string GameResultToString(GameResult result) {
    switch (result) {
        case GameResult::kBlackWin:
            return "BlackWin";
        case GameResult::kWhiteWin:
            return "WhiteWin";
        case GameResult::kDraw:
            return "Draw";
        case GameResult::kOngoing:
        default:
            return "Ongoing";
    }
}

inline GameResult WinResultFor(Side side) {
    return side == Side::kBlack ? GameResult::kBlackWin : GameResult::kWhiteWin;
}

struct Move {
    int from_row = -1;
    int from_col = -1;
    int row = -1;
    int col = -1;
    Side side = Side::kBlack;
};

}  // namespace chess
