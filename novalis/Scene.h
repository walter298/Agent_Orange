#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <type_traits>
#include <algorithm>
#include <ranges>
#include <memory>

#include <nlohmann/json.hpp>

#include "Event.h"
#include "Instance.h"
#include "Button.h"
#include "GlobalMacros.h"
#include "RenderTypes.h"

namespace nv {
	class Scene {
	public:
		enum class EndReason {
			Quit,
			NextScene
		};
	private:
		EndReason m_endReason = EndReason::Quit;

		std::vector<Sprite> m_sprites;
		std::vector<Text> m_texts;
		Background m_background;
		
		std::vector<Event> m_events;
		std::vector<Button> m_buttons;
		std::vector<Event*> m_nonOwningEvents;
		std::vector<Button*> m_nonOwningButtons;

		bool m_running = false;

		std::vector<RectPtr> m_collisionBoxes;

		Renderer m_renderer;
	public:
		Scene(Instance& instance);
		Scene(std::string path, Instance& instance);

		EndReason endReason() const noexcept;

		void endScene(EndReason end) noexcept;

		//todo: these functions crash the program!
		void addObj(Sprite obj, int layer) noexcept;
		void addObj(Text obj, int layer) noexcept;

		template<typename T>
		void render(T&& t, const RenderObjID& ID, int layer) {
			m_renderer.addObj(std::forward<T>(t), ID, layer);
		}

		void stopRendering(const RenderObjID& ID, int layer) noexcept;

		void addEvent(Event&& evt) noexcept;
		void addEvent(Event* evt) noexcept; //evt must be non-owning
		void cancelEvent(ID<Event> ID) noexcept;

		void addButton(Button&& btn) noexcept;
		void addButton(Button* btn) noexcept; //btn must be non-owning
		void reCoordButton(ID<Button> ID) noexcept;

		void camCoord(int dx, int dy);

		void execute();
	};
}

#endif