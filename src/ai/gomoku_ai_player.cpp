#include "ai/gomoku_ai_player.h"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <vector>

#include "games/gomoku/gomoku_state.h"

namespace chess {

namespace {

int SegmentScore(int length, int open_ends) {
    if (length >= 5) {
        return 1'000'000;
    }
    if (length == 4) {
        if (open_ends == 2) {
            return 120'000;
        }
        if (open_ends == 1) {
            return 18'000;
        }
    }
    if (length == 3) {
        if (open_ends == 2) {
            return 6'000;
        }
        if (open_ends == 1) {
            return 900;
        }
    }
    if (length == 2) {
        if (open_ends == 2) {
            return 280;
        }
        if (open_ends == 1) {
            return 80;
        }
    }
    if (length == 1 && open_ends == 2) {
        return 20;
    }
    return 0;
}

int EvaluateSide(const GomokuState& state, Side side) {
    const int n = state.BoardSize();
    const char stone = side == Side::kBlack ? 'X' : 'O';
    int score = 0;
    static const int dirs[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};

    for (int r = 0; r < n; ++r) {
        for (int c = 0; c < n; ++c) {
            if (state.CellAt(r, c) != stone) {
                continue;
            }
            for (const auto& d : dirs) {
                const int pr = r - d[0];
                const int pc = c - d[1];
                if (pr >= 0 && pr < n && pc >= 0 && pc < n && state.CellAt(pr, pc) == stone) {
                    continue;
                }

                int length = 0;
                int nr = r;
                int nc = c;
                while (nr >= 0 && nr < n && nc >= 0 && nc < n && state.CellAt(nr, nc) == stone) {
                    ++length;
                    nr += d[0];
                    nc += d[1];
                }

                int open_ends = 0;
                if (pr >= 0 && pr < n && pc >= 0 && pc < n && state.CellAt(pr, pc) == '.') {
                    ++open_ends;
                }
                if (nr >= 0 && nr < n && nc >= 0 && nc < n && state.CellAt(nr, nc) == '.') {
                    ++open_ends;
                }
                score += SegmentScore(length, open_ends);
            }
        }
    }
    return score;
}

}  // namespace

GomokuAiPlayer::GomokuAiPlayer(int difficulty)
    : config_(ConfigForDifficulty(difficulty)),
      rng_(std::random_device{}()) {}

std::string GomokuAiPlayer::Name() const {
    return "AI";
}

bool GomokuAiPlayer::IsHuman() const {
    return false;
}

GomokuAiPlayer::DifficultyConfig GomokuAiPlayer::ConfigForDifficulty(int difficulty) const {
    if (difficulty <= 1) {
        return DifficultyConfig{1, 8, 3};
    }
    if (difficulty == 2) {
        return DifficultyConfig{2, 12, 2};
    }
    return DifficultyConfig{3, 16, 1};
}

int GomokuAiPlayer::TerminalScore(GameResult result, Side ai_side, int depth) const {
    if (result == GameResult::kDraw) {
        return 0;
    }
    if ((result == GameResult::kBlackWin && ai_side == Side::kBlack) ||
        (result == GameResult::kWhiteWin && ai_side == Side::kWhite)) {
        return 5'000'000 + depth;
    }
    return -5'000'000 - depth;
}

int GomokuAiPlayer::EvaluatePosition(const GomokuState& state, Side ai_side) const {
    const int my_score = EvaluateSide(state, ai_side);
    const int opp_score = EvaluateSide(state, Opponent(ai_side));
    return my_score - static_cast<int>(opp_score * 1.1);
}

int GomokuAiPlayer::Search(const GomokuState& state, int depth, int alpha, int beta, Side ai_side) const {
    const GameResult result = state.Result();
    if (result != GameResult::kOngoing) {
        return TerminalScore(result, ai_side, depth);
    }
    if (depth == 0) {
        return EvaluatePosition(state, ai_side);
    }

    const auto candidates = state.CandidateMoves(config_.width);
    if (candidates.empty()) {
        return EvaluatePosition(state, ai_side);
    }

    const bool maximizing = state.CurrentSide() == ai_side;
    int best = maximizing ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();

    for (const auto& cell : candidates) {
        GomokuState next = state;
        next.ApplyMove(Move{-1, -1, cell.first, cell.second, state.CurrentSide()});
        const int value = Search(next, depth - 1, alpha, beta, ai_side);
        if (maximizing) {
            best = std::max(best, value);
            alpha = std::max(alpha, best);
        } else {
            best = std::min(best, value);
            beta = std::min(beta, best);
        }
        if (beta <= alpha) {
            break;
        }
    }
    return best;
}

Move GomokuAiPlayer::ChooseMove(const IGameState& state) {
    const auto* gomoku = dynamic_cast<const GomokuState*>(&state);
    if (!gomoku) {
        throw std::runtime_error("GomokuAiPlayer requires GomokuState.");
    }
    if (gomoku->Result() != GameResult::kOngoing) {
        return Move{};
    }

    const Side ai_side = gomoku->CurrentSide();
    const auto candidates = gomoku->CandidateMoves(config_.width);
    if (candidates.empty()) {
        return Move{};
    }

    struct RankedMove {
        int row;
        int col;
        int score;
    };
    std::vector<RankedMove> ranked;
    ranked.reserve(candidates.size());

    for (const auto& cell : candidates) {
        GomokuState next = *gomoku;
        next.ApplyMove(Move{-1, -1, cell.first, cell.second, ai_side});

        if (next.Result() == WinResultFor(ai_side)) {
            return Move{-1, -1, cell.first, cell.second, ai_side};
        }

        const int score = Search(next,
                                 config_.depth - 1,
                                 std::numeric_limits<int>::min(),
                                 std::numeric_limits<int>::max(),
                                 ai_side);
        ranked.push_back(RankedMove{cell.first, cell.second, score});
    }

    std::sort(ranked.begin(), ranked.end(), [](const RankedMove& a, const RankedMove& b) {
        if (a.score != b.score) {
            return a.score > b.score;
        }
        if (a.row != b.row) {
            return a.row < b.row;
        }
        return a.col < b.col;
    });

    const std::size_t top_k = std::min<std::size_t>(std::max(1, config_.random_top_k), ranked.size());
    std::uniform_int_distribution<std::size_t> dist(0, top_k - 1);
    const RankedMove picked = ranked[dist(rng_)];
    return Move{-1, -1, picked.row, picked.col, ai_side};
}

}  // namespace chess
