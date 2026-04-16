#pragma once

#include <vector>

#include "core/game_state.h"

namespace chess {

class XiangqiState final : public IGameState {
public:
    XiangqiState();

    Side CurrentSide() const override;
    GameResult Result() const override;
    bool IsMoveLegal(const Move& move) const override;
    bool ApplyMove(const Move& move) override;
    std::vector<Move> LegalMoves() const override;
    std::string Render() const override;
    std::unique_ptr<IGameState> Clone() const override;

    static constexpr int kRows = 10;
    static constexpr int kCols = 9;

private:
    bool InBounds(int row, int col) const;
    bool IsCurrentSidePiece(char piece, Side side) const;
    bool InPalace(Side side, int row, int col) const;
    bool HasCrossedRiver(Side side, int row) const;
    int CountBetween(int from_row, int from_col, int to_row, int to_col) const;
    bool IsValidPieceMove(const Move& move, bool capture_only) const;
    std::pair<int, int> FindKing(Side side) const;
    bool IsSquareAttacked(Side by_side, int row, int col) const;
    bool IsInCheck(Side side) const;

    std::vector<std::vector<char>> board_;
    Side current_side_;
    GameResult result_;
};

}  // namespace chess
