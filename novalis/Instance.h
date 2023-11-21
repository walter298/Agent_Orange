#pragma once

#include <any>

#include "Renderer.h"
#include "RenderTypes.h"

#include "Sound.h"

namespace nv {
	class Instance {
	private:
		SDL_Window* m_SDLWindow;
		SDL_Renderer* m_SDLRenderer;

		std::map<std::string, Sprite> m_spriteMap;
		std::map<std::string, Background> m_backgroundMap;
		std::map<std::string, Text> m_textMap;

		//std::string param is the filepath
		using ObjLoader = std::move_only_function<std::pair<std::string, std::any>(std::string)>;

		std::map<std::string, ObjLoader> m_typeLoaders;
		std::map<std::string, std::any> m_customTypeMap;

		void quit();

		Instance() = default;
	public:
		Instance(std::string windowTitle);
		~Instance();

		SDL_Window* getRawWindow() noexcept;
		SDL_Renderer* getRawRenderer() noexcept;

		Sprite getSprite(std::string name) const;
		Background getBackground(std::string name) const;
		Text getText(std::string name) const;

		void setCustomObjLoader(std::string fileExt, ObjLoader loader);

		template<typename T>
		T getCustomObj(std::string name) {
			return std::any_cast<T>(m_customTypeMap.at(name));
		}

		void loadObjsFromDir(std::string absDirPath);
	};
}