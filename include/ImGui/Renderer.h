#pragma once

namespace ImGui::Renderer
{
	void Init();
	void Install();

	// members
	inline std::atomic initialized{ false };
}
