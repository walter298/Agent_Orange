#pragma once

#include <algorithm>
#include <iostream>
#include <optional>
#include <random>
#include <ranges>

#include "types.h"

namespace merck {
	template<std::integral T>
	T makeRandomNum(T min, T max) {
		static std::random_device dev;
		static std::mt19937 mt{ dev() };
		std::uniform_int_distribution<T> dist{ min, max };
		return dist(mt);
	}
}

std::optional<Move> getBestMove(GameState& state);