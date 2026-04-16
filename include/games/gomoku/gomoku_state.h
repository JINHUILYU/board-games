#pragma once

#include <utility>
#include <vector>

#include "core/game_state.h"

namespace chess {

class GomokuState final : public IGameState {
public:
    explicit GomokuState(int board_size = 15);

    Side CurrentSide() const override;
    GameResult Result() const override;
    bool IsMoveLegal(const Move& move) const override;
    bool ApplyMove(const Move& move) override;
    std::vector<Move> LegalMoves() const override;
    std::string Render() const override;
    std::unique_ptr<IGameState> Clone() const override;

    int BoardSize() const;
    char CellAt(int row, int col) const;
    std::vector<std::pair<int, int>> CandidateMoves(std::size_t limit) const;

private:
    bool InBounds(int row, int col) const;
    bool HasNeighbor(int row, int col, int distance) const;
    bool CheckWinAt(int row, int col) const;
    bool HasAnyStone() const;

    int board_size_;
    std::vector<std::vector<char>> board_;
    Side current_side_;
    GameResult result_;
    int move_count_;
};

}  // namespace chess
