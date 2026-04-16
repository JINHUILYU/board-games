#pragma once

#include <string>

#include "core/player.h"

namespace chess {

class HumanConsolePlayer final : public IPlayer {
public:
    explicit HumanConsolePlayer(std::string name);

    std::string Name() const override;
    bool IsHuman() const override;
    Move ChooseMove(const IGameState& state) override;

private:
    std::string name_;
};

}  // namespace chess
