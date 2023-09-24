#include "position.h"

GameState makeGameState() {
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
		ret.turnColor = Color::White;
	} else {
		ret.turnColor = Color::Black;
	}

	auto inputPiece = [&forceValidInput] {
		static const std::map<std::string, PieceType> types = {
			{ "king", PieceType::King }
		};

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
			std::cout << "color of new piece is white\n";
			color = Color::White;
		} else {
			std::cout << "color of new piece is black\n";
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

	bool whiteKingExists = false;
	bool blackKingExists = false;

	auto addPiece = [&](Pieces& side, Piece p, bool& kingExists) {
		if (p.type == PieceType::King && kingExists) {
			std::cerr << "Error: there can only be one king per side\n";
		}  
		if (p.type == PieceType::King) {
			kingExists = true;
		}
		side.push_back(std::move(p));
	};
	while (true) {
		std::string doneInput;
		std::cout << "Are you done adding pieces (yes or no): ";
		std::cin >> doneInput;
		if (doneInput == "no") {
			auto newPiece = inputPiece();
			if (newPiece.color == Color::White) {
				addPiece(ret.whitePieces, std::move(newPiece), whiteKingExists);
			} else {
				addPiece(ret.blackPieces, std::move(newPiece), blackKingExists);
			}
		} else {
			if (!whiteKingExists) {
				std::cerr << "Error: white must have a king\n";
				continue;
			} else if (!blackKingExists) {
				std::cerr << "Error: black must have a king\n";
				continue;
			}
			break;
		}
	}

	//give each piece pressure over squares
	auto signifyPressure = [&ret](const Pieces& pieces, 
								  int Square::*pressuringPieces) 
	{
		for (const auto& p : pieces) {
			auto squares = p.calcMoves();
			for (const auto& [x, y] : squares) {
				(ret.board[x][y].*pressuringPieces)++;
			}
		}
	};

	signifyPressure(ret.whitePieces, &Square::pressuringWhitePieces);
	signifyPressure(ret.blackPieces, &Square::pressuringBlackPieces);

	ret.removeKingMoveOverlap();

	std::cout << "What is the name of your position? ";
	std::string fileNameInput;
	std::cin >> fileNameInput;

	std::ofstream file{ fileNameInput };
	file << nlohmann::json(ret).dump(2);
	file.close();

	return ret;
}