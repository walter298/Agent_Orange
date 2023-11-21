#include "types.h"

Coords Piece::getKingCoords(const GameState& state) const {
	Coords ret;
	ret.reserve(8);

	auto validSquare = [&state, this](int squareX, int squareY) {
		if (!inBounds(x, y)) {
			return false;
		}
		const auto& square = state(squareX, squareY);
		return (!square.occupyingPiece || square.occupyingPiece->color != color);
	};
	for (int dx = -1; dx < 2; dx++) {
		for (int dy = -1; dy < 2; dy++) {
			int squareX = x + dx;
			int squareY = y + dy;
			std::cout << squareX << ", " << squareY << std::endl;
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
	
	p.m_possibleMoves = j.at("possible_moves").get<Coords>();

	switch (type) {
	case PieceType::King:
		p = Piece::makeKing(color, x, y);
		break;
	}
}

void to_json(nlohmann::json& j, const Piece& p) {
	j["type"] = p.type;
	j["color"] = p.color;
	j["x"] = p.x;
	j["y"] = p.y;
	j["possible_moves"] = p.getPossibleMoves();
}

Piece Piece::makeKing(Color color, int squareX, int squareY) {
	Piece ret;
	ret.moveFinder = &Piece::getKingCoords;
	ret.type = PieceType::King;
	ret.color = color;
	ret.value = kingValue;
	ret.x = squareX;
	ret.y = squareY;
	return ret;
}

const Coords& Piece::getPossibleMoves() const {
	return m_possibleMoves;
}

void Piece::setPossibleMoves(Coords moves) {
	m_possibleMoves = std::move(moves);
}

const Coords& Piece::calcMoves(const GameState& state) {
	m_blockingSquares.clear();
	m_possibleMoves = std::invoke(moveFinder, *this, state);
	std::ranges::sort(m_possibleMoves);
	return m_possibleMoves;
}

void Piece::recalcUponPieceMovement(const GameState& state, Coord freedCoord,
	Coord takenSquare) 
{
	if (std::ranges::contains(m_blockingSquares, freedCoord) || 
		std::ranges::contains(m_possibleMoves, takenSquare)) 
	{
		calcMoves(state);
	}
}

void to_json(nlohmann::json& j, const Square& s) {
	if (!s.occupyingPiece) {
		j["has_piece"] = false;
	} else {
		j["has_piece"] = true;
		j["piece"] = *s.occupyingPiece;
	}
}

void from_json(const nlohmann::json& j, Square& s) {
	if (j.at("has_piece").get<bool>()) {
		s.occupyingPiece = j.at("piece").get<Piece>();
	}
}

const Piece& GameState::getKing(const Pieces& side) {
	return *std::ranges::find_if(side,
		[](const auto& p) { return p.type == PieceType::King; });
}

const Pieces& GameState::getWhitePieces() const {
	return m_whitePieces;
}

const Pieces& GameState::getBlackPieces() const {
	return m_blackPieces;
}

Pieces::iterator GameState::findPiece(Pieces& pieces, int x, int y) {
	return std::ranges::find_if(
		pieces,
		[&x, &y](const auto& p) { return std::tie(p.x, p.y) == std::tie(x, y); }
	);
}

bool GameState::addPieceIfPossible(Piece& newPiece) {
	if (m_board[newPiece.x][newPiece.y].occupyingPiece) {
		std::cerr << "Error: " << newPiece.x << ", " << newPiece.y << " is already occupied\n";
		return false;
	}

	m_board[newPiece.x][newPiece.y].occupyingPiece = newPiece;

	int friendlyCheckCount = 0; //# of checks against the friendly king

	/*keep track of where pieces could move before so we don't have
	to recalculate them if the position ends being impossible*/
	std::vector<Coords> oldFriendlyMoves, oldEnemyMoves;
	oldFriendlyMoves.reserve(m_allies->size());
	oldEnemyMoves.reserve(m_enemies->size());
	
	//auto revertToOldMoves = [](Pieces& pieces, )
	auto tolerateChecks = [this, &newPiece](Pieces& pieces, std::vector<Coords>& oldMoves,
								   Piece& king, int checkThreshold) {
		int checkCount = 0;
		for (auto [idx, piece] : std::views::enumerate(pieces)) {
			std::cout << "calculating moves\n";
			oldMoves.push_back(piece.getPossibleMoves());
			if (std::ranges::binary_search(
				piece.calcMoves(*this), std::tie(king.x, king.y)))
			{
				std::cout << "there is a check\n";
				if (checkCount++ > checkThreshold) {
					return false;
				}
			}
			std::cout << "there isn't a check\n";
		}
		return true;
	};

	auto revertToOldMoves = [](std::vector<Coords>& oldMoves, Pieces& pieces) {
		for (auto [oldMoveSet, piece] : std::views::zip(oldMoves, pieces)) {
			piece.setPossibleMoves(std::move(oldMoveSet));
		}
	};
	
	if (!tolerateChecks(*m_allies, oldFriendlyMoves, *m_enemyKing, 0)) {
		std::cerr << "Error: the enemy king can't be in check when it isn't the enemy's turn\n";
		revertToOldMoves(oldFriendlyMoves, *m_allies);
		m_board[newPiece.x][newPiece.y].occupyingPiece = std::nullopt;
		return false;
	} else if (!tolerateChecks(*m_enemies, oldEnemyMoves, *m_allyKing, 2)) {
		std::cerr << "Error: there can be no more than 2 checks against the friendly king in a position\n";
		revertToOldMoves(oldFriendlyMoves, *m_allies);
		revertToOldMoves(oldEnemyMoves, *m_enemies);
		m_board[newPiece.x][newPiece.y].occupyingPiece = std::nullopt;
		return false;
	}
	if (newPiece.color == Color::White) {
		m_whitePieces.push_back(std::move(newPiece));
	} else {
		m_blackPieces.push_back(std::move(newPiece));
	}
	return true;
}

bool GameState::isAttacked(const Piece& p) {
	for (const auto& enemy : *m_enemies) {
		if (intersects(p.getPossibleMoves(), enemy.getPossibleMoves())) {
			return true;
		}
	}
	return false;
}

void GameState::recalcMoves(Pieces& pieces) {
	for (auto& p : pieces) {
		p.calcMoves(*this);
	}
}

GameState GameState::makeCustomPosition() {
	GameState ret;

	auto forceValidInput = [](auto& input, auto validPred, std::string inputMsg) {
		while (true) {
			std::cout << inputMsg << ' ';
			std::cin >> input;
			if (!validPred()) {
				std::cerr << "Error: " << input << " is invalid\n";
			} else {
				break;
			}
		}
	};

	std::cout << "Whose turn is it? ";
	std::string turnColorInput;
	std::cin >> turnColorInput;
	if (turnColorInput == "white") {
		ret.setTurn(Color::White);
	} else {
		ret.setTurn(Color::Black);
	}

	//input the white king and black king positions
	auto inputKing = [&ret, &forceValidInput](Piece& king, std::string color) {
		while (true) {
			std::cout << "Where is the " << color << " king?\n";
			int x = 0;
			int y = 0;

			forceValidInput(x,
				[&x] { return inBounds(x); }, "Enter the x Coordinate");
			forceValidInput(y,
				[&y] { return inBounds(y); }, "Enter the y Coordinate");

			std::tie(king.x, king.y) = std::tie(x, y); //set king position
			std::cout << "Adding piece if possible\n";
			if (ret.addPieceIfPossible(king)) {
				break;
			}
		}
	};

	inputKing(ret.m_whiteKing, "white");
	inputKing(ret.m_blackKing, "black");

	auto inputPiece = [&forceValidInput] {
		static const std::map<std::string, PieceType> types;

		std::string typeInput;
		forceValidInput(typeInput,
			[&] { return types.contains(typeInput); },
			"Enter the piece's type:"
		);
		auto type = types.at(typeInput);

		std::string colorInput;
		Color color;

		std::cout << "Enter the piece color: ";
		std::cin >> colorInput;
		if (colorInput == "white") {
			color = Color::White;
		} else {
			color = Color::Black;
		}

		int x, y;
		forceValidInput(x, [&x] { return x >= 0 && x < 8; },
			"Enter the x coordinate:");
		forceValidInput(y, [&y] { return y >= 0 && y < 8; },
			"Enter the y coordinate:");

		switch (type) {
		case PieceType::King:
			return Piece::makeKing(color, x, y);
			break;
		}
		std::unreachable();
	};

	while (true) {
		std::string doneInput;
		std::cout << "Are you done adding pieces (yes or no): ";
		std::cin >> doneInput;
		if (doneInput == "no") {
			
		}
		else {
			break;
		}
	}

	std::cout << "What is the name of your position? ";
	std::string fileNameInput;
	std::cin >> fileNameInput;

	//save the position file
	std::ofstream file{ fileNameInput };
	nlohmann::json j = ret;
	file << j.dump(2);
	file.close();

	return ret;
}

GameState::GameState(std::string jsonPath) {
	//open state file
	std::ifstream file;
	file.open(jsonPath);
	assert(file.is_open());

	//parse the json
	auto j = nlohmann::json::parse(file);
	*this = j.get<GameState>();

	file.close(); //always close files
}

const Square& GameState::operator()(int x, int y) const {
	return m_board[x][y];
}

void GameState::setTurn(Color color) {
	if (color == Color::White) {
		m_allies = &m_whitePieces;
		m_allyKing = &m_whiteKing;
		m_enemies = &m_blackPieces;
		m_enemyKing = &m_blackKing;
	} else {
		m_allies = &m_blackPieces;
		m_allyKing = &m_blackKing;
		m_enemies = &m_whitePieces;
		m_enemyKing = &m_whiteKing;
	}
}

std::vector<MovePossibilities> GameState::possibleMoves() {
	std::vector<MovePossibilities> ret;
	ret.reserve(m_allies->size());

	//get all possible squares from a side, without "checking" for checks
	auto movesFromSide = [this](Pieces& side) {
		std::vector<Coords> ret;
		ret.reserve(side.size());
		for (auto& p : side) {
			ret.push_back(p.getPossibleMoves());
		}
		return ret;
	};

	/*if the ally king is in check, remove any squares from each ally piece
	that does not block that check or any squares the king cannot go to*/
	auto removeNonSavingSquares = [this](Coords moves, bool escaping) {
		for (auto& enemy : m_checkingPieces) {
			std::erase_if(moves,
				[&, this](const auto& move) {
					auto shared = std::ranges::contains(enemy.calcMoves(*this), move);
					return (escaping) ? shared : !shared;
				}
			);
		}
		return moves;
	};

	/*get every move from the enemy pieces and don't 
	let the king go to any of those squares*/
	auto enemyMoves = movesFromSide(*m_enemies);
	const auto& kingMoves = m_allyKing->getPossibleMoves();
	ret.emplace_back(
		std::pair{ m_allyKing->x, m_allyKing->y },
		removeNonSavingSquares(kingMoves, true)
	);

	//add moves of all other ally pieces
	for (auto& p : *m_allies) {
		const auto& moves = p.calcMoves(*this);
		if (!m_checkingPieces.empty()) {
			ret.emplace_back(
				std::pair{ p.x, p.y },
				removeNonSavingSquares(moves, false)
			);
		} else {
			ret.emplace_back(std::pair{ p.x, p.y }, moves);
		}
	}

	return ret;
}

void GameState::update(const Move& move) {
	const auto& [start, dest] = move;
	
	//get piece to move
	auto& movedPiece = *std::ranges::find_if(
		*m_allies, [&start](const auto& p) {
			return std::tie(p.x, p.y) == start;
		}
	);

	const auto& [startX, startY] = start;
	const auto& [destX, destY] = dest;

	//if there is a piece on the destination square, capture it 
	if (m_board[startX][startY].occupyingPiece) {
		m_enemies->erase(findPiece(*m_enemies, destX, destY));
	}

	//update pieces that are checking the enemy king
	m_checkingPieces.clear();
	m_checkingPieces.reserve(2);
	int checkCount = 0;
	for (const auto& enemy : *m_enemies) {
		const auto& enemySquares = enemy.getPossibleMoves();
		if (std::ranges::contains(enemySquares, std::tie(m_allyKing->x, m_allyKing->y))) {
			m_checkingPieces.push_back(enemy);
			//at most only two pieces can deliver a check
			if (checkCount++ == 2) {
				break;
			}
		}
	}

	//recalculate destination squares for pieces
	auto recalulateMovesUponMovement = [this, &start, &dest](Pieces& pieces) {
		for (auto& p : pieces) {
			p.recalcUponPieceMovement(*this, start, dest);
		}
	};
	recalulateMovesUponMovement(*m_allies);
	recalulateMovesUponMovement(*m_enemies);
}

void from_json(const nlohmann::json& j, GameState& g) {
	g.m_board = j.at("board").get<Board>();
	g.m_whitePieces = j.at("white_pieces").get<Pieces>();
	g.m_whiteKing = j.at("white_king").get<Piece>();
	g.m_blackPieces = j.at("black_pieces").get<Pieces>();
	g.m_blackKing = j.at("black_king").get<Piece>();
	g.m_turnColor = j.at("turn_color").get<Color>();

	g.setTurn(g.m_turnColor);
}

void to_json(nlohmann::json& j, const GameState& g) {
	j["board"] = g.m_board;
	j["white_pieces"] = g.m_whitePieces;
	j["white_king"] = g.m_whiteKing;
	j["black_pieces"] = g.m_blackPieces;
	j["black_king"] = g.m_blackKing;
	j["turn_color"] = g.m_turnColor;
}

bool inBounds(int c) {
	return c >= 0 && c <= 7;
}

bool inBounds(int x, int y) {
	return inBounds(x) && inBounds(y);
}