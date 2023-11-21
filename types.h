#pragma once

#include <array>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <tuple>
#include <vector>

#include "custom_array.hpp"

template<std::ranges::viewable_range Range>
Range getIntersection(Range&& r1, Range&& r2) {
	Range ret;
	std::ranges::set_intersection(
		std::forward<Range>(r1), std::forward<Range>(r2),
		std::back_inserter(ret)
	);
	return ret;
}

template<std::ranges::viewable_range SortedRange>
bool intersects(const SortedRange& r1, const SortedRange& r2) {
	for (const auto& elem : r1) {
		if (std::ranges::binary_search(r2, elem)) {
			return true;
		}
 	}
	return false;
}

class GameState;
struct Square;

using Board = merck::custom_array<merck::custom_array<Square, int, 8>, int, 8>;

using Squares = std::vector<Square>;

using Coord = std::pair<int, int>;
using Coords  = std::vector<Coord>;

using Move = std::pair<Coord, Coord>;
using Moves = std::vector<Move>;
using MovePossibilities = std::pair<Coord, Coords>;

enum class Color {
	White,
	Black,
	None
};

enum class PieceType {
	King,
	Queen,
	Knight,
	Bishop,
	Rook,
	Pawn
};

struct Piece {
private:
	static constexpr int kingValue = 1000;

	Coords getKingCoords(const GameState&) const;

	using MoveFinder = Coords(Piece::*)(const GameState&) const;
	MoveFinder moveFinder = nullptr;

	//squares with pieces blocking this piece's entry
	Coords m_blockingSquares; 
	
	Coords m_possibleMoves;
public:
	PieceType type;
	Color color;

	int value;

	int x = 0;
	int y = 0;

	bool pinned = false;
	
	const Coords& getPossibleMoves() const;
	void setPossibleMoves(Coords moves);
	const Coords& calcMoves(const GameState& state);

	void recalcUponPieceMovement(const GameState& state, Coord freedCoord, Coord takenSquare);

	static Piece makeKing(Color color, int squareX, int squareY);

	friend void from_json(const nlohmann::json& j, Piece& p);
	friend void to_json(nlohmann::json& j, const Piece& p);
};

void from_json(const nlohmann::json& j, Piece& p);
void to_json(nlohmann::json& j, const Piece& p);

using Pieces = std::vector<Piece>;

struct Square {
	std::optional<Piece> occupyingPiece;
};

void to_json(nlohmann::json& j, const Square& p);
void from_json(const nlohmann::json& j, Square& p);

class GameState {
private:
	Pieces m_checkingPieces;

	Pieces* m_allies   = nullptr;
	Pieces* m_enemies  = nullptr;
	Piece* m_allyKing  = nullptr;
	Piece* m_enemyKing = nullptr;

	Color m_turnColor = Color::White;

	Pieces m_whitePieces;
	Pieces m_blackPieces;
	Piece m_whiteKing = Piece::makeKing(Color::White, 0, 0);
	Piece m_blackKing = Piece::makeKing(Color::Black, 7, 7);

	Board m_board;

	Pieces::iterator findPiece(Pieces& pieces, int x, int y);

	bool addPieceIfPossible(Piece& piece);
	bool isAttacked(const Piece& p);
	void recalcMoves(Pieces& pieces);
public:
	static GameState makeCustomPosition();

	GameState() = default;
	GameState(std::string jsonPath);

	const Square& operator()(int x, int y) const;
	
	const Piece& getKing(const Pieces& side);

	const Pieces& getWhitePieces() const;
	const Pieces& getBlackPieces() const;

	std::vector<MovePossibilities> possibleMoves();
	void setTurn(Color color);
	void update(const Move& move);

	friend void from_json(const nlohmann::json&, GameState&);
	friend void to_json(nlohmann::json&, const GameState&);
};

void from_json(const nlohmann::json& j, GameState& g);
void to_json(nlohmann::json& j, const GameState& g);

bool inBounds(int c);
bool inBounds(int x, int y);





