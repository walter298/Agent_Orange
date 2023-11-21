#include "position.h"

std::optional<Move> getBestMove(GameState& state)
{
	auto possibleMoves = state.possibleMoves();
	if (possibleMoves.empty()) {
		return std::nullopt;
	}


	auto& [start, possiblities] = 
		possibleMoves[merck::makeRandomNum(0ull, possibleMoves.size() - 1)];

	if (possiblities.empty()) {
		return std::nullopt;
	} else {
		return std::pair{ 
			start, 
			possiblities[merck::makeRandomNum(0ull, possiblities.size() - 1)] 
		};
	}
}