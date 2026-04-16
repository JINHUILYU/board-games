#pragma once

#include <random>
#include <string>

#include "core/player.h"

namespace chess {

class GomokuAiPlayer final : public IPlayer {
public:
    explicit GomokuAiPlayer(int difficulty);

    std::string Name() const override;
    bool IsHuman() const override;
    Move ChooseMove(const IGameState& state) override;

private:
    struct DifficultyConfig {
        int depth = 1;
        std::size_t width = 8;
        int random_top_k = 2;
    };

    DifficultyConfig ConfigForDifficulty(int difficulty) const;

    int EvaluatePosition(const class GomokuState& state, Side ai_side) const;
    int Search(const class GomokuState& state, int depth, int alpha, int beta, Side ai_side) const;
    int TerminalScore(GameResult result, Side ai_side, int depth) const;

    DifficultyConfig config_;
    std::mt19937 rng_;
};

}  // namespace chess
