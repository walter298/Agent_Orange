#include "game_scene.h"

Coord GameScene::screenCoord(Coord indexCoord) {
	const auto& [x, y] = indexCoord;
	return { (x * 100) + 209, (y * 100) + 11 };
}

void GameScene::updateScreen(Move move) {
	const auto& [start, dest] = move;
	
	//auto& movedPiece = m_pieces.at(start);

	//update moved piece's screen position
	//const auto& [x, y] = dest;
	//movedPiece->setRenPos(x, y);

	////if piece is being captured, stop rendering it
	//if (m_pieces.contains(dest)) {
	//	stopRendering(m_pieces.at(dest)->getID(), PIECE_LAYER);
	//}

	////update piece map
	//m_pieces[dest] = std::move(movedPiece);
	//m_pieces.erase(start);
}

GameScene::GameScene(nv::Instance& instance, GameState state)
	: nv::Scene{ instance }, m_state{ std::move(state) }
{
	render(instance.getSprite("board"), nv::RenderObjID(), 0);
	
	auto addPiece = [this](const Piece& p, const nv::Sprite& sp) {
		//copy piece sprite and switch to correct texture
		auto newPieceSprite = std::make_unique<nv::Sprite>(sp);
		
		//render the sprite
		auto& ren = newPieceSprite->getRen().rect;
		std::tie(ren.x, ren.y) = screenCoord({ p.x, p.y });
		render(newPieceSprite.get(), newPieceSprite->getID(), PIECE_LAYER);

		//add piece to map
		m_pieces[{ p.x, p.y }] = std::move(newPieceSprite);
	};

	auto addPieces = [&addPiece, this](const Pieces& pieces, const nv::Sprite& sp) 
	{
		for (const auto& p : pieces) {
			addPiece(p, sp);
		}
		addPiece(m_state.getKing(pieces), sp);
	};

	addPieces(m_state.getWhitePieces(), instance.getSprite("white_pieces"));
	addPieces(m_state.getBlackPieces(), instance.getSprite("black_pieces"));

	addEvent(
		[this] {
			//if (!m_calculatingMove.load()) {
			//	m_updated = false;
			//	m_calculatingMove = true;
			//	std::jthread{
			//		[this] {
			//			//auto move = getBestMove(m_state);
			//			if (move) {
			//				//m_currentMove = *move;
			//				//m_calculatingMove.store(false);
			//			}
			//		}
			//	}.detach();
			//} else if (!m_updated) {
			//	//m_state.update(m_currentMove);
			//	//updateScreen(m_currentMove);
			//	m_updated = true;
			//}
		}
	);
}