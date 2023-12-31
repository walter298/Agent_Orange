#include "RenderTypes.h"

nv::Texture::Texture(SDL_Texture* tex) noexcept 
	: raw(tex) {}

nv::Texture::~Texture() noexcept {
	SDL_DestroyTexture(raw);
}

const std::string& nv::detail::NamedIDObj::getName() const noexcept {
	return m_name;
}

const nv::RenderObjID& nv::detail::NamedIDObj::getID() const noexcept {
	return m_ID;
}

nv::Sprite::Sprite(SDL_Renderer* renderer, std::string absFilePath)
{
	std::ifstream spriteFile{ absFilePath };
	assert(spriteFile.is_open());

	auto jsonData = json::parse(spriteFile);

	//load each image as a texture
	auto imgPaths = jsonData.at("image_paths").get<std::vector<std::string>>();
	m_spriteSheet.reserve(imgPaths.size());
	for (const auto& path : imgPaths) {
		m_spriteSheet.push_back(std::make_shared<Texture>(IMG_LoadTexture(renderer, path.c_str())));
	}
	assert(m_spriteSheet.size() > 0);
	m_currentSprite = m_spriteSheet[0]->raw;

	if (m_currentSprite == nullptr) {
		std::cerr << SDL_GetError() << std::endl;
		exit(-1);
	}

	m_name = jsonData.at("name").get<std::string>();
	m_ren = jsonData.at("ren").get<Rect>();
	m_world = jsonData.at("world").get<Rect>();
}

void nv::Sprite::render(SDL_Renderer* renderer) noexcept {
	SDL_RenderCopy(renderer, m_currentSprite, nullptr, &m_ren.rect);
}

void nv::Sprite::Coord(int dx, int dy) noexcept {
	renMove(dx, dy);
	worldMove(dx, dy);
}

nv::Background::Background(SDL_Renderer* renderer, std::string absFilePath) {
	std::ifstream file{ absFilePath };
	auto jsonData = json::parse(file);
	m_name = jsonData.at("name").get<std::string>();
	m_rens = jsonData.at("rens").get<std::vector<Rect>>();
	
	//load textures
	auto imagePaths = jsonData.at("image_paths").get<std::vector<std::string>>();
	for (const auto& path : imagePaths) {
		m_backgrounds.push_back(std::make_shared<Texture>(IMG_LoadTexture(renderer, path.c_str())));
	}
}

void nv::Background::render(SDL_Renderer* renderer) noexcept {
	for (auto [background, ren] : std::ranges::zip_view(m_backgrounds, m_rens)) {
		SDL_RenderCopy(renderer, background->raw, nullptr, &ren.rect);
	}
}

void nv::Background::renMove(int dx, int dy) noexcept {
	for (auto& ren : m_rens) {
		ren.move(dx, dy);
	}
}

nv::Text::Text(SDL_Renderer* renderer, std::string absPath) {
	std::ifstream textFile{ absPath };
	assert(textFile.is_open());
	auto jsonData = json::parse(textFile);
	textFile.close();

	m_name = jsonData.at("name").get<std::string>();
	m_color = jsonData.at("color").get<SDL_Color>();
	m_background = jsonData.at("background").get<Rect>();
	
	changeText(renderer, jsonData.at("text").get<std::string>());
}

void nv::Text::openFonts() noexcept {
	std::string fontPath = relativePath("novalis_assets/fonts/");
	auto loadFont = [&fontPath](std::string name) {
		return TTF_OpenFont((fontPath + name).c_str(), 14);
	};
	fontMap = {
		std::pair{ FontType::Libertine, loadFont("Libertine.ttf") },
		std::pair{ FontType::WorkSans, loadFont("WorkSans.ttf") }
	};
}

void nv::Text::closeFonts() noexcept {
	for (auto& [type, font] : fontMap) {
		TTF_CloseFont(font);
	}
	fontMap.clear();
}

void nv::Text::render(SDL_Renderer* renderer) noexcept {
	m_background.render(renderer);
	SDL_RenderCopy(renderer, m_tex->raw, nullptr, &m_rect.rect);
}

void nv::Text::setRenPos(int x, int y) noexcept {
	int dx = x - m_ren.rect.x;
	int dy = y - m_ren.rect.y;
	m_ren.setPos(x, y);
	m_background.move(dx, dy);
}

void nv::Text::setFontSize(SDL_Renderer* renderer, int fontSize) noexcept {
	TTF_SetFontSize(m_font, fontSize);
	TTF_SizeText(m_font, m_text.c_str(), &m_ren.rect.w, &m_ren.rect.h);
	m_fontSize = std::move(fontSize);
}

void nv::Text::changeText(SDL_Renderer* renderer, std::string text) noexcept {
	m_text = std::move(text);
	if (m_tex->raw != nullptr) {
		SDL_DestroyTexture(m_tex->raw);
		m_tex->raw = nullptr;
	}
	if (m_text.size() > 0) {
		SDL_Surface* surface = TTF_RenderText_Solid(m_font, text.c_str(), m_color);
		m_tex->raw = SDL_CreateTextureFromSurface(renderer, surface);
		SDL_FreeSurface(surface);
	}
}

void nv::Text::setColor(SDL_Color color) noexcept {
	m_color = std::move(color);
}