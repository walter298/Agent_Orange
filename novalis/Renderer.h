#ifndef RENDERER_H
#define RENDERER_H

#include <chrono>
#include <map>
#include <string>
#include <filesystem>
#include <iostream>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_sdlrenderer2.h"

#include "RenderTypes.h"
#include "GlobalMacros.h"

namespace nv {
	class Renderer {
	public:
		struct RenderObj {
			int ID = 0;
			virtual void render(SDL_Renderer*) = 0;
		};
		template<typename T>
		struct RenderObjChild : public RenderObj {
			std::remove_cvref_t<T> obj; //Don't store cv/references
			void render(SDL_Renderer* renderer) override {
				using plain = std::remove_cvref_t<std::remove_pointer_t<T>>;
				std::invoke(&plain::render, obj, renderer);
			}
			RenderObjChild(T&& obj, int id) 
				: obj{ std::forward<T>(obj) } 
			{
				ID = id;
			}
		};
	private:
		SDL_Renderer* m_renderer;

		std::map<int, std::vector<std::unique_ptr<RenderObj>>> m_objects;
	public:
		Renderer(SDL_Renderer* renderer); 
		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&)      = delete;

		inline SDL_Renderer* get() {
			return m_renderer;
		}

		void clear() noexcept;

		template<typename T>
		void addObj(T&& t, const RenderObjID& ID, int layer) noexcept {
			m_objects[layer].push_back(
				std::make_unique<RenderObjChild<T>>(std::forward<T>(t), ID.value())
			);
		}

		void reCoordObj(const RenderObjID& ID, int layer);

		void render() noexcept; 
		void renderImgui(ImGuiIO& io);
	};
}

#endif