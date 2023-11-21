#include "Renderer.h"

nv::Renderer::Renderer(SDL_Renderer* renderer) 
	: m_renderer(renderer)
{
}

void nv::Renderer::clear() noexcept {
	m_objects.clear();
}

void nv::Renderer::reCoordObj(const RenderObjID& ID, int layer) {
	auto objPos = std::ranges::find_if(m_objects.at(layer),
		[&ID](const auto& obj) { return obj->ID == ID.value(); }
	);
	if (objPos != m_objects.at(layer).end()) {
		m_objects.at(layer).erase(objPos);
	} else {
		throw std::logic_error("Error: tried to reCoord a non-existant object from renderer");
	}
}

void nv::Renderer::render() noexcept {
	SDL_RenderClear(m_renderer);

	for (auto& [layer, objs] : m_objects) {
		for (auto& obj : objs) {
			obj->render(m_renderer);
		}
	}

	SDL_RenderPresent(m_renderer);
}

void nv::Renderer::renderImgui(ImGuiIO& io) {
	ImVec4 color{ 0.45f, 0.55f, 0.60f, 1.00f };

	ImGui::Render();
	SDL_RenderSetScale(m_renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
	SDL_SetRenderDrawColor(m_renderer,
		//unfortunately SDL uses ints for screen pixels and ImGui uses floats 
		static_cast<Uint8>(color.x * 255), static_cast<Uint8>(color.y * 255),
		static_cast<Uint8>(color.z * 255), static_cast<Uint8>(color.w * 255));
	SDL_RenderClear(m_renderer);
	ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());

	
	for (auto& [layer, objs] : m_objects) {
		for (auto& obj : objs) {
			obj->render(m_renderer);
		}
	}

	SDL_RenderPresent(m_renderer);
}