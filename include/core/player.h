#pragma once

#include <string>

#include "core/game_state.h"

namespace chess {

class IPlayer {
public:
    virtual ~IPlayer() = default;
    virtual std::string Name() const = 0;
    virtual bool IsHuman() const = 0;
    virtual Move ChooseMove(const IGameState& state) = 0;
};

}  // namespace chess
