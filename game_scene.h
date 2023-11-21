#pragma once

#include <atomic>
#include <future>

#include "novalis/Scene.h"
#include "novalis/EditorUtil.h"

#include "position.h"

class GameScene : public nv::Scene {
private:
	static constexpr int SQUARE_SELECTION_LAYER = 1;
	static constexpr int PIECE_LAYER = 2;

	static Coord screenCoord(Coord indexCoord);

	std::map<Coord, std::unique_ptr<nv::Sprite>> m_pieces;
	GameState m_state;

	Move m_currentMove;

	std::atomic_bool m_calculatingMove = false;
	bool m_updated = false;

	void updateScreen(Move move);
public:
	GameScene(nv::Instance& instance, GameState state);
};

