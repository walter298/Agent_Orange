#include "game_scene.h"

#include "novalis/Instance.h"

#undef main

void makeTestBoardObj() {
	nv::Instance instance{ "text" };
	std::vector<std::string> paths{ nv::relativePath("images/board.jpeg") };
	nv::Rect ren, world;
	ren.rect = { 200, 0, 900, 900 };

	nlohmann::json j;
	j["name"] = "board";
	j["image_paths"] = paths;
	j["ren"] = ren;
	j["world"] = world;

	std::ofstream file{ nv::relativePath("nv_objects/board.txt") };
	file << j.dump(2);
	file.close();

	nv::Sprite s{
		instance.getRawRenderer(),
		nv::relativePath("nv_objects/board.txt")
	};
}

void makeTestPieceSprite() {
	nv::Instance instance{ "text" };
	std::vector<std::string> paths{ nv::relativePath("images/white_king.png") };
	nv::Rect ren;
	ren.rect = { 0, 0, 50, 50 };

	nlohmann::json j;
	j["name"] = "black_king";
	j["image_paths"] = paths;
	j["ren"] = ren;
	j["world"] = nv::Rect();

	std::ofstream file{ nv::relativePath("nv_objects/black_king.txt") };
	file << j.dump(2);
	file.close();

	nv::Sprite s{
		instance.getRawRenderer(),
		nv::relativePath("nv_objects/black_king.txt")
	};
}

int main() {
	//make the state of the game
	std::ifstream stateFile{ nv::relativePath("king_test_7.json") };
	auto fState = nlohmann::json::parse(stateFile).get<GameState>();
	stateFile.close();

	try {
		nv::Instance instance{ "agent_orange" };
		instance.loadObjsFromDir(nv::relativePath("nv_objects"));
		
		GameScene scene{ instance, std::move(fState) };
		scene.execute();
	} catch (std::runtime_error& e) {
		std::cout << e.what() << std::endl;
	}
}