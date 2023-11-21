#ifndef Sprite_H
#define Sprite_H

#include <fstream>
#include <filesystem>
#include <iostream>
#include <memory>
#include <ranges>
#include <regex>
#include <string>
#include <vector>

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

#include "Rect.h"
#include "DataUtil.h"
#include "ID.h"

namespace nv {
	struct Texture {
		SDL_Texture* raw = nullptr;

		Texture() = default;
		explicit Texture(SDL_Texture* texture) noexcept;

		Texture(const Texture&)            = delete;
		Texture& operator=(const Texture&) = delete;

		Texture(Texture&&) noexcept            = default;
		Texture& operator=(Texture&&) noexcept = default;

		~Texture() noexcept;
	};

	using TexturePtr = std::shared_ptr<Texture>;
	
	using RenderObjID = ID<int>; //arbitrary type parameter

	namespace detail {
		class NamedIDObj {
		protected:
			std::string m_name;
			RenderObjID m_ID;
		public:
			const std::string& getName() const noexcept;
			const RenderObjID& getID() const noexcept;
		};
	}

	class Sprite : public detail::NamedIDObj {
	private:
		std::vector<TexturePtr> m_spriteSheet;

		size_t m_spriteIndex = 0;

		SDL_Texture* m_currentSprite = nullptr;

		using CollisionBoxes = std::vector<std::vector<Rect>>;
		using RenderBoxes    = std::vector<Rect>;

		CollisionBoxes m_collisionBoxes;
		std::vector<Rect> m_renderBoxes;

		Rect m_ren, m_world;
	public:
		Sprite() = default;
		Sprite(SDL_Renderer* renderer, std::string path);

		inline SDL_Texture* getSprite() {
			return m_currentSprite;
		}

		inline Rect& getRen() {
			return m_ren;
		}

		inline Rect& getWorld() {
			return m_world;
		}

		inline void renMove(int dx, int dy) noexcept {
			m_ren.move(dx, dy);
		}

		inline void worldMove(int dx, int dy) noexcept {
			m_world.move(dx, dy);
		}

		void Coord(int dx, int dy) noexcept;

		inline void setRenPos(int x, int y) noexcept {
			m_ren.setPos(x, y);
		}

		inline void setWorldPos(int x, int y) noexcept {
			m_world.setPos(x, y);
		}

		inline void renScale(int dw, int dh) noexcept {
			m_ren.scale(dw, dh);
		}

		inline void worldScale(int dw, int dh) noexcept {
			m_world.scale(dw, dh);
		}

		inline void setRenSize(int w, int h) noexcept {
			m_ren.setSize(w, h);
		}

		inline void setWorldSize(int w, int h) noexcept {
			m_world.setSize(w, h);
		}

		inline void scale(int dw, int dh) noexcept {
			renScale(dw, dh);
			worldScale(dw, dh);
		}

		inline void changeTexture(size_t idx) {
			m_currentSprite = m_spriteSheet[idx]->raw;
			m_ren = m_renderBoxes[idx];
		}

		void render(SDL_Renderer* renderer) noexcept;
	};

	struct Background : public detail::NamedIDObj {
	private:
		std::vector<nv::Rect> m_rens;
		std::vector<TexturePtr> m_backgrounds;
	public:
		Background() = default;
		Background(SDL_Renderer* renderer, std::string absFilePath);

		void render(SDL_Renderer* renderer) noexcept;

		void renMove(int dx, int dy) noexcept;
	};

	using SpritePtr = std::unique_ptr<Sprite>;

	enum class FontType {
		WorkSans,
		Libertine,
	};

	using FontMap = std::unordered_map<FontType, TTF_Font*>;

	namespace editor {
		class TextEditor;
	}

	class Text : public detail::NamedIDObj {
	private:
		TexturePtr m_tex = std::make_shared<Texture>(nullptr);
		Rect m_rect;
		TTF_Font* m_font = nullptr;
		std::string m_text;
		SDL_Color m_color{ 255, 255, 255, 0 };
		int m_fontSize = 14;

		nv::Rect m_background{ 0, 0, 0, 0 };
		nv::Rect m_ren{ 0, 0, 0, 0 };

		static inline FontMap fontMap;
	public:
		//these methods should ONLY be called by nv::Instance
		static void openFonts() noexcept;
		static void closeFonts() noexcept;

		Text() = default; 
		Text(SDL_Renderer* renderer, std::string absPath);

		virtual void render(SDL_Renderer* renderer) noexcept;

		void setRenPos(int x, int y) noexcept;
		void setFontSize(SDL_Renderer* renderer, int fontSize) noexcept;
		void changeText(SDL_Renderer* renderer, std::string text) noexcept;
		void setColor(SDL_Color color) noexcept;
		
		friend class editor::TextEditor;
	};

	using TextPtr = std::unique_ptr<Text>;

	template<typename Stream> 
	Stream& operator<<(Stream& stream, const Text& text) {
		json j;
		j["type"]        = "text"s;
		j["name"]        = text.m_name;
		j["text"]        = text.m_text;
		j["color"]       = text.m_color;
		j["background"]  = text.m_background;
		j["ren"]         = text.m_ren;
		stream << j.dump(2);
		return stream;
	}
}

#endif