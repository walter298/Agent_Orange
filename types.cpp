#include "types.h"

Moves Piece::getKingMoves(const GameState& state) const {
	printf("Finding moves\n");
	Moves ret;
	ret.reserve(8);

	auto validSquare = [&state, this](int squareX, int squareY) {
		const auto& square = state.board[squareX][squareY];
		return inBounds(squareX, squareX) &&
			*square.pressuringEnemyPieces == 0 &&
			(!square.occupyingPiece || square.occupyingPiece->color != color);
	};
	for (int dx = -1; dx < 2; dx++) {
		for (int dy = -1; dy < 2; dy++) {
			int squareX = x + dx;
			int squareY = y + dy;
			if ((squareX != x && squareY != y) && validSquare(squareX, squareY)) {
				ret.emplace_back(x, y);
			}
		}
	}
	return ret;
}

void from_json(const nlohmann::json& j, Piece& p) {
	auto type = j.at("type").get<PieceType>();
	auto color = j.at("color").get<Color>();
	auto x = j.at("x").get<int>();
	auto y = j.at("y").get<int>();

	switch (type) {
	case PieceType::King:
		p = Piece::makeKing(std::move(color), std::move(x), std::move(y));
		break;
	}
	std::unreachable();
}

void to_json(nlohmann::json& j, const Piece& p) {
	j["color"] = p.color;
	j["x"] = p.x;
	j["y"] = p.y;
	j["value"] = p.value;
}

Piece Piece::makeKing(Color color, int squareX, int squareY) {
	Piece ret;
	ret.moveFinder = &Piece::getKingMoves;
	ret.type = PieceType::King;
	ret.color = color;
	ret.value = kingValue;
	ret.x = squareX;
	ret.y = squareY;
	return ret;
}

Moves Piece::calcMoves() const {
	return std::invoke(&Piece::possibleMoves, *this);
}

void to_json(nlohmann::json& j, const Square& s) {
	if (!s.occupyingPiece) {
		j["has_piece"] = false;
	} else {
		j["has_piece"] = true;
		j["piece"] = *s.occupyingPiece;
	}
	j["pressuring_white_pieces"] = s.pressuringWhitePieces;
	j["pressuring_black_pieces"] = s.pressuringBlackPieces;
}

void from_json(const nlohmann::json& j, Square& s) {
	if (j.at("has_piece").get<bool>()) {
		s.occupyingPiece = j.at("piece").get<Piece>();
	}
	s.pressuringWhitePieces = j.at("pressuring_white_pieces").get<int>();
	s.pressuringBlackPieces = j.at("pressuring_black_pieces").get<int>();
}

Piece& GameState::getKing(Pieces& side) {
	return *std::ranges::find_if(side,
		[](const auto& p) { return p.type == PieceType::King; });
}

void GameState::removeKingMoveOverlap() {
	auto& whiteKing = getKing(whitePieces);
	auto& blackKing = getKing(blackPieces);
	
	std::ranges::sort(whiteKing.possibleMoves);
	std::ranges::sort(blackKing.possibleMoves);

	auto removeOverlappingSquares = [](Piece& k1, const Piece& k2) {
		Moves uniqueMoves;
		std::ranges::set_difference(
			k1.possibleMoves, k2.possibleMoves,
			std::back_inserter(uniqueMoves)
		);
		k1.possibleMoves = uniqueMoves;
	};
	removeOverlappingSquares(whiteKing, blackKing);
	removeOverlappingSquares(blackKing, whiteKing);
}

void from_json(const nlohmann::json& j, GameState& g) {
	g.board = j.at("board").get<Board>();
	g.whitePieces = j.at("white_pieces").get<Pieces>();
	g.blackPieces = j.at("black_pieces").get<Pieces>();

	int Square::*enemyPieces;
	if (j.at("turn_color").get<Color>() == Color::White) {
		enemyPieces = &Square::pressuringBlackPieces;
	} else {
		enemyPieces = &Square::pressuringWhitePieces;
	}
	for (auto& row : g.board) {
		for (auto& square : row) {
			square.pressuringEnemyPieces = &(square.*enemyPieces);
		}
	}
}

void to_json(nlohmann::json& j, const GameState& g) {
	j["board"] = g.board;
	j["white_pieces"] = g.whitePieces;
	j["black_pieces"] = g.blackPieces;
	j["turn_color"] = g.turnColor;
}

bool inBounds(int x, int y) {
	return x >= 0 && x <= 7 && y >= 0 && y <= 7;
}