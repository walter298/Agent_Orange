#pragma once

#include <array>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <tuple>
#include <vector>

#include <nlohmann/json.hpp>

struct GameState;
struct Square;

using Squares = std::vector<Square>;
using Moves  = std::vector<std::pair<int, int>>;

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

	Moves getKingMoves(const GameState&) const;

	using MoveFinder = Moves(Piece::*)(const GameState&) const;
	MoveFinder moveFinder;
public:
	PieceType type;
	Color color;

	int value;

	int x = 0;
	int y = 0;
	
	Moves possibleMoves;

	Moves calcMoves() const;

	static Piece makeKing(Color color, int squareX, int squareY);
};

void from_json(const nlohmann::json& j, Piece& p);
void to_json(nlohmann::json& j, const Piece& p);

using Pieces = std::vector<Piece>;

struct Square {
	std::optional<Piece> occupyingPiece;
	int pressuringWhitePieces = 0;
	int pressuringBlackPieces = 0;
	int* pressuringEnemyPieces = &pressuringBlackPieces;
};

void to_json(nlohmann::json& j, const Square& p);
void from_json(const nlohmann::json& j, Square& p);

using Board = std::array<std::array<Square, 8>, 8>;

struct GameState {
	Color turnColor = Color::White;

	Pieces whitePieces;
	Pieces blackPieces;

	Board board;

	Piece& getKing(Pieces& side);

	void removeKingMoveOverlap();
};

void from_json(const nlohmann::json& j, GameState& g);
void to_json(nlohmann::json& j, const GameState& g);

bool inBounds(int x, int y);





