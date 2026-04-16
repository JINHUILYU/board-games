#include "core/turn_engine.h"

#include <stdexcept>

namespace chess {

TurnEngine::SessionResult TurnEngine::Run(IGameState& state,
                                          IPlayer& black_player,
                                          IPlayer& white_player,
                                          const StateCallback& on_state_updated) {
    SessionResult session;
    if (on_state_updated) {
        on_state_updated(state);
    }

    while (state.Result() == GameResult::kOngoing) {
        const Side current_side = state.CurrentSide();
        IPlayer& current_player = (current_side == Side::kBlack) ? black_player : white_player;

        Move move = current_player.ChooseMove(state);
        move.side = current_side;

        if (!state.IsMoveLegal(move)) {
            throw std::runtime_error("Illegal move provided by player: " + current_player.Name());
        }

        state.ApplyMove(move);
        session.moves.push_back(move);

        if (on_state_updated) {
            on_state_updated(state);
        }
    }

    session.result = state.Result();
    return session;
}

}  // namespace chess
