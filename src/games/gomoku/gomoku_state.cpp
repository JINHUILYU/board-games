#include "games/gomoku/gomoku_state.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

#include "core/types.h"

namespace chess {

namespace {

char StoneFor(Side side) {
    return side == Side::kBlack ? 'X' : 'O';
}

int LocalPriority(const std::vector<std::vector<char>>& board, int row, int col) {
    static const int dirs[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1},
        {0, -1},           {0, 1},
        {1, -1},  {1, 0},  {1, 1},
    };
    const int n = static_cast<int>(board.size());
    int score = 0;
    for (const auto& d : dirs) {
        const int nr = row + d[0];
        const int nc = col + d[1];
        if (nr < 0 || nr >= n || nc < 0 || nc >= n) {
            continue;
        }
        if (board[nr][nc] == 'X' || board[nr][nc] == 'O') {
            score += 1;
        }
    }
    return score;
}

}  // namespace

GomokuState::GomokuState(int board_size)
    : board_size_(board_size),
      board_(board_size, std::vector<char>(board_size, '.')),
      current_side_(Side::kBlack),
      result_(GameResult::kOngoing),
      move_count_(0) {}

Side GomokuState::CurrentSide() const {
    return current_side_;
}

GameResult GomokuState::Result() const {
    return result_;
}

bool GomokuState::InBounds(int row, int col) const {
    return row >= 0 && row < board_size_ && col >= 0 && col < board_size_;
}

bool GomokuState::IsMoveLegal(const Move& move) const {
    const int row = move.row;
    const int col = move.col;
    return result_ == GameResult::kOngoing && InBounds(row, col) && board_[row][col] == '.';
}

bool GomokuState::CheckWinAt(int row, int col) const {
    if (!InBounds(row, col) || board_[row][col] == '.') {
        return false;
    }
    static const int dirs[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
    const char stone = board_[row][col];
    for (const auto& d : dirs) {
        int count = 1;
        for (int k = 1;; ++k) {
            const int nr = row + d[0] * k;
            const int nc = col + d[1] * k;
            if (!InBounds(nr, nc) || board_[nr][nc] != stone) {
                break;
            }
            ++count;
        }
        for (int k = 1;; ++k) {
            const int nr = row - d[0] * k;
            const int nc = col - d[1] * k;
            if (!InBounds(nr, nc) || board_[nr][nc] != stone) {
                break;
            }
            ++count;
        }
        if (count >= 5) {
            return true;
        }
    }
    return false;
}

bool GomokuState::ApplyMove(const Move& move) {
    if (!IsMoveLegal(move)) {
        return false;
    }
    const int row = move.row;
    const int col = move.col;

    const Side side = current_side_;
    board_[row][col] = StoneFor(side);
    ++move_count_;

    if (CheckWinAt(row, col)) {
        result_ = WinResultFor(side);
        return true;
    }

    if (move_count_ == board_size_ * board_size_) {
        result_ = GameResult::kDraw;
        return true;
    }

    current_side_ = Opponent(current_side_);
    return true;
}

std::vector<Move> GomokuState::LegalMoves() const {
    std::vector<Move> moves;
    moves.reserve(board_size_ * board_size_);
    if (result_ != GameResult::kOngoing) {
        return moves;
    }
    for (int r = 0; r < board_size_; ++r) {
        for (int c = 0; c < board_size_; ++c) {
            if (board_[r][c] == '.') {
                moves.push_back(Move{-1, -1, r, c, current_side_});
            }
        }
    }
    return moves;
}

std::string GomokuState::Render() const {
    std::ostringstream oss;
    oss << "   ";
    for (int c = 0; c < board_size_; ++c) {
        oss << std::setw(2) << (c + 1) << ' ';
    }
    oss << '\n';
    for (int r = 0; r < board_size_; ++r) {
        oss << std::setw(2) << (r + 1) << ' ';
        for (int c = 0; c < board_size_; ++c) {
            oss << ' ' << board_[r][c] << ' ';
        }
        oss << '\n';
    }
    if (result_ == GameResult::kOngoing) {
        oss << "Turn: " << SideToString(current_side_) << '\n';
    } else {
        oss << "Result: " << GameResultToString(result_) << '\n';
    }
    return oss.str();
}

std::unique_ptr<IGameState> GomokuState::Clone() const {
    return std::make_unique<GomokuState>(*this);
}

int GomokuState::BoardSize() const {
    return board_size_;
}

char GomokuState::CellAt(int row, int col) const {
    if (!InBounds(row, col)) {
        return '?';
    }
    return board_[row][col];
}

bool GomokuState::HasAnyStone() const {
    return move_count_ > 0;
}

bool GomokuState::HasNeighbor(int row, int col, int distance) const {
    for (int dr = -distance; dr <= distance; ++dr) {
        for (int dc = -distance; dc <= distance; ++dc) {
            if (dr == 0 && dc == 0) {
                continue;
            }
            const int nr = row + dr;
            const int nc = col + dc;
            if (!InBounds(nr, nc)) {
                continue;
            }
            if (board_[nr][nc] == 'X' || board_[nr][nc] == 'O') {
                return true;
            }
        }
    }
    return false;
}

std::vector<std::pair<int, int>> GomokuState::CandidateMoves(std::size_t limit) const {
    std::vector<std::pair<int, int>> candidates;
    if (result_ != GameResult::kOngoing) {
        return candidates;
    }

    if (!HasAnyStone()) {
        const int center = board_size_ / 2;
        candidates.emplace_back(center, center);
        return candidates;
    }

    struct ScoredMove {
        int row;
        int col;
        int score;
    };
    std::vector<ScoredMove> scored;

    for (int r = 0; r < board_size_; ++r) {
        for (int c = 0; c < board_size_; ++c) {
            if (board_[r][c] != '.') {
                continue;
            }
            if (!HasNeighbor(r, c, 1)) {
                continue;
            }
            scored.push_back(ScoredMove{r, c, LocalPriority(board_, r, c)});
        }
    }

    if (scored.empty()) {
        for (int r = 0; r < board_size_; ++r) {
            for (int c = 0; c < board_size_; ++c) {
                if (board_[r][c] == '.') {
                    scored.push_back(ScoredMove{r, c, 0});
                }
            }
        }
    }

    std::sort(scored.begin(), scored.end(), [](const ScoredMove& a, const ScoredMove& b) {
        if (a.score != b.score) {
            return a.score > b.score;
        }
        if (a.row != b.row) {
            return a.row < b.row;
        }
        return a.col < b.col;
    });

    const std::size_t take = std::min(limit, scored.size());
    candidates.reserve(take);
    for (std::size_t i = 0; i < take; ++i) {
        candidates.emplace_back(scored[i].row, scored[i].col);
    }
    return candidates;
}

}  // namespace chess
