#pragma once

#include <memory>
#include <string>
#include <vector>

#include "core/types.h"

namespace chess {

class IGameState {
public:
    virtual ~IGameState() = default;

    virtual Side CurrentSide() const = 0;
    virtual GameResult Result() const = 0;
    virtual bool IsMoveLegal(const Move& move) const = 0;
    virtual bool ApplyMove(const Move& move) = 0;
    virtual std::vector<Move> LegalMoves() const = 0;
    virtual std::string Render() const = 0;
    virtual std::unique_ptr<IGameState> Clone() const = 0;
};

}  // namespace chess
