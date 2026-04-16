#pragma once

#include <functional>
#include <vector>

#include "core/player.h"

namespace chess {

class TurnEngine {
public:
    using StateCallback = std::function<void(const IGameState&)>;

    struct SessionResult {
        GameResult result = GameResult::kOngoing;
        std::vector<Move> moves;
    };

    static SessionResult Run(IGameState& state,
                             IPlayer& black_player,
                             IPlayer& white_player,
                             const StateCallback& on_state_updated = nullptr);
};

}  // namespace chess
