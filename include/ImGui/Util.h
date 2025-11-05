#pragma once

#define IM_COL32_RED IM_COL32(255, 0, 0, 255) k
#define IM_COL32_YELLOW IM_COL32(255, 255, 0, 255)
#define IM_COL32_GREEN IM_COL32(0, 255, 0, 255)
#define IM_COL32_BLUE IM_COL32(0, 128, 255, 255)

namespace ImGui
{
	float GetResolutionScale();
	
	ImU32 GetColorU32(const ImVec4& col, float alpha_mul);
	ImVec4 GetColor(const RE::NiColor& a_color);

	float WorldToScreenLoc(const RE::NiPoint3& worldLocIn, ImVec2& screenLocOut);

	void DrawCircle(const RE::NiPoint3& a_pos, float radius, ImU32 color = IM_COL32_WHITE);
	void DrawLine(const RE::NiPoint3& a_from, const RE::NiPoint3& a_to, ImU32 color = IM_COL32_WHITE);
	void DrawTextAtPoint(const RE::NiPoint3& a_pos, const char* a_text, ImU32 color = IM_COL32_WHITE);

	ImVec2 GetNativeViewportSize();
	ImVec2 GetNativeViewportPos();
	ImVec2 GetNativeViewportCenter();
}
