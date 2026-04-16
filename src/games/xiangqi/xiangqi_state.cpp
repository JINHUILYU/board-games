#include "games/xiangqi/xiangqi_state.h"

#include <cmath>
#include <cctype>
#include <iomanip>
#include <sstream>

namespace chess {

namespace {

bool IsEmpty(char piece) {
    return piece == '.';
}

bool IsBlackPiece(char piece) {
    return piece >= 'A' && piece <= 'Z';
}

bool IsWhitePiece(char piece) {
    return piece >= 'a' && piece <= 'z';
}

int Sign(int value) {
    if (value > 0) {
        return 1;
    }
    if (value < 0) {
        return -1;
    }
    return 0;
}

std::string XiangqiSideToString(Side side) {
    return side == Side::kBlack ? "Black" : "Red";
}

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

}  // namespace

XiangqiState::XiangqiState()
    : board_{
          {'R', 'H', 'E', 'A', 'K', 'A', 'E', 'H', 'R'},
          {'.', '.', '.', '.', '.', '.', '.', '.', '.'},
          {'.', 'C', '.', '.', '.', '.', '.', 'C', '.'},
          {'P', '.', 'P', '.', 'P', '.', 'P', '.', 'P'},
          {'.', '.', '.', '.', '.', '.', '.', '.', '.'},
          {'.', '.', '.', '.', '.', '.', '.', '.', '.'},
          {'p', '.', 'p', '.', 'p', '.', 'p', '.', 'p'},
          {'.', 'c', '.', '.', '.', '.', '.', 'c', '.'},
          {'.', '.', '.', '.', '.', '.', '.', '.', '.'},
          {'r', 'h', 'e', 'a', 'k', 'a', 'e', 'h', 'r'},
      },
      current_side_(Side::kBlack),
      result_(GameResult::kOngoing) {}

Side XiangqiState::CurrentSide() const {
    return current_side_;
}

GameResult XiangqiState::Result() const {
    return result_;
}

bool XiangqiState::InBounds(int row, int col) const {
    return row >= 0 && row < kRows && col >= 0 && col < kCols;
}

bool XiangqiState::IsCurrentSidePiece(char piece, Side side) const {
    if (IsEmpty(piece)) {
        return false;
    }
    return side == Side::kBlack ? IsBlackPiece(piece) : IsWhitePiece(piece);
}

bool XiangqiState::InPalace(Side side, int row, int col) const {
    if (col < 3 || col > 5) {
        return false;
    }
    if (side == Side::kBlack) {
        return row >= 0 && row <= 2;
    }
    return row >= 7 && row <= 9;
}

bool XiangqiState::HasCrossedRiver(Side side, int row) const {
    if (side == Side::kBlack) {
        return row >= 5;
    }
    return row <= 4;
}

int XiangqiState::CountBetween(int from_row, int from_col, int to_row, int to_col) const {
    if (from_row != to_row && from_col != to_col) {
        return -1;
    }
    const int step_row = Sign(to_row - from_row);
    const int step_col = Sign(to_col - from_col);
    int row = from_row + step_row;
    int col = from_col + step_col;
    int count = 0;
    while (row != to_row || col != to_col) {
        if (!IsEmpty(board_[row][col])) {
            ++count;
        }
        row += step_row;
        col += step_col;
    }
    return count;
}

bool XiangqiState::IsValidPieceMove(const Move& move, bool capture_only) const {
    if (!InBounds(move.from_row, move.from_col) || !InBounds(move.row, move.col)) {
        return false;
    }
    if (move.from_row == move.row && move.from_col == move.col) {
        return false;
    }

    const char piece = board_[move.from_row][move.from_col];
    const char target = board_[move.row][move.col];
    if (IsEmpty(piece)) {
        return false;
    }
    if (IsCurrentSidePiece(target, current_side_)) {
        return false;
    }
    if (capture_only && IsEmpty(target)) {
        return false;
    }

    const Side side = IsBlackPiece(piece) ? Side::kBlack : Side::kWhite;
    const int dr = move.row - move.from_row;
    const int dc = move.col - move.from_col;
    const int abs_dr = std::abs(dr);
    const int abs_dc = std::abs(dc);
    const char kind = static_cast<char>(std::tolower(static_cast<unsigned char>(piece)));

    switch (kind) {
        case 'r':
            return (dr == 0 || dc == 0) && CountBetween(move.from_row, move.from_col, move.row, move.col) == 0;
        case 'h':
            if (!((abs_dr == 2 && abs_dc == 1) || (abs_dr == 1 && abs_dc == 2))) {
                return false;
            }
            if (abs_dr == 2) {
                return IsEmpty(board_[move.from_row + Sign(dr)][move.from_col]);
            }
            return IsEmpty(board_[move.from_row][move.from_col + Sign(dc)]);
        case 'e':
            if (abs_dr != 2 || abs_dc != 2) {
                return false;
            }
            if (!IsEmpty(board_[move.from_row + Sign(dr)][move.from_col + Sign(dc)])) {
                return false;
            }
            if (side == Side::kBlack && move.row > 4) {
                return false;
            }
            if (side == Side::kWhite && move.row < 5) {
                return false;
            }
            return true;
        case 'a':
            return abs_dr == 1 && abs_dc == 1 && InPalace(side, move.row, move.col);
        case 'k': {
            const bool step_one = (abs_dr + abs_dc == 1) && InPalace(side, move.row, move.col);
            if (step_one) {
                return true;
            }
            const char enemy_king = side == Side::kBlack ? 'k' : 'K';
            return target == enemy_king && move.from_col == move.col &&
                   CountBetween(move.from_row, move.from_col, move.row, move.col) == 0;
        }
        case 'c': {
            if (dr != 0 && dc != 0) {
                return false;
            }
            const int between = CountBetween(move.from_row, move.from_col, move.row, move.col);
            if (IsEmpty(target)) {
                return between == 0 && !capture_only;
            }
            return between == 1;
        }
        case 'p': {
            const int forward = side == Side::kBlack ? 1 : -1;
            if (dr == forward && dc == 0) {
                return true;
            }
            if (!HasCrossedRiver(side, move.from_row)) {
                return false;
            }
            return dr == 0 && abs_dc == 1;
        }
        default:
            return false;
    }
}

std::pair<int, int> XiangqiState::FindKing(Side side) const {
    const char king = side == Side::kBlack ? 'K' : 'k';
    for (int row = 0; row < kRows; ++row) {
        for (int col = 0; col < kCols; ++col) {
            if (board_[row][col] == king) {
                return {row, col};
            }
        }
    }
    return {-1, -1};
}

bool XiangqiState::IsSquareAttacked(Side by_side, int row, int col) const {
    XiangqiState probe = *this;
    probe.current_side_ = by_side;
    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            if (!probe.IsCurrentSidePiece(probe.board_[r][c], by_side)) {
                continue;
            }
            const Move attack{r, c, row, col, by_side};
            if (probe.IsValidPieceMove(attack, true)) {
                return true;
            }
        }
    }
    return false;
}

bool XiangqiState::IsInCheck(Side side) const {
    const auto [king_row, king_col] = FindKing(side);
    if (king_row < 0) {
        return true;
    }
    return IsSquareAttacked(Opponent(side), king_row, king_col);
}

bool XiangqiState::IsMoveLegal(const Move& move) const {
    if (result_ != GameResult::kOngoing) {
        return false;
    }
    if (!InBounds(move.from_row, move.from_col) || !InBounds(move.row, move.col)) {
        return false;
    }
    const char piece = board_[move.from_row][move.from_col];
    if (!IsCurrentSidePiece(piece, current_side_)) {
        return false;
    }
    if (!IsValidPieceMove(move, false)) {
        return false;
    }

    XiangqiState next = *this;
    next.board_[move.row][move.col] = next.board_[move.from_row][move.from_col];
    next.board_[move.from_row][move.from_col] = '.';
    return !next.IsInCheck(current_side_);
}

std::vector<Move> XiangqiState::LegalMoves() const {
    std::vector<Move> moves;
    if (result_ != GameResult::kOngoing) {
        return moves;
    }
    for (int from_row = 0; from_row < kRows; ++from_row) {
        for (int from_col = 0; from_col < kCols; ++from_col) {
            if (!IsCurrentSidePiece(board_[from_row][from_col], current_side_)) {
                continue;
            }
            for (int to_row = 0; to_row < kRows; ++to_row) {
                for (int to_col = 0; to_col < kCols; ++to_col) {
                    Move move{from_row, from_col, to_row, to_col, current_side_};
                    if (IsMoveLegal(move)) {
                        moves.push_back(move);
                    }
                }
            }
        }
    }
    return moves;
}

bool XiangqiState::ApplyMove(const Move& move) {
    if (!IsMoveLegal(move)) {
        return false;
    }
    const char captured = board_[move.row][move.col];
    board_[move.row][move.col] = board_[move.from_row][move.from_col];
    board_[move.from_row][move.from_col] = '.';

    if (captured == 'k') {
        result_ = GameResult::kBlackWin;
        return true;
    }
    if (captured == 'K') {
        result_ = GameResult::kWhiteWin;
        return true;
    }

    current_side_ = Opponent(current_side_);
    if (LegalMoves().empty()) {
        result_ = WinResultFor(Opponent(current_side_));
    }
    return true;
}

std::string XiangqiState::Render() const {
    std::ostringstream oss;
    oss << "    ";
    for (int col = 0; col < kCols; ++col) {
        oss << std::setw(2) << (col + 1) << ' ';
    }
    oss << '\n';
    for (int row = 0; row < kRows; ++row) {
        oss << std::setw(2) << (row + 1) << "  ";
        for (int col = 0; col < kCols; ++col) {
            oss << ' ' << board_[row][col] << ' ';
        }
        oss << '\n';
    }
    oss << "Pieces: R/H/E/A/K/C/P (black, uppercase), r/h/e/a/k/c/p (red, lowercase)\n";
    if (result_ == GameResult::kOngoing) {
        oss << "Turn: " << XiangqiSideToString(current_side_) << '\n';
    } else {
        oss << "Result: " << XiangqiResultToString(result_) << '\n';
    }
    return oss.str();
}

std::unique_ptr<IGameState> XiangqiState::Clone() const {
    return std::make_unique<XiangqiState>(*this);
}

}  // namespace chess
