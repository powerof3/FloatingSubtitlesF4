#pragma once

namespace ImGui
{
	struct Font
	{
		struct FontParams
		{
			std::string name;
			float       spacing{ 1.0f };
		};

		FontParams LoadFontSettings(const CSimpleIniA& a_ini, const char* a_section);
		void       LoadFont(ImFontConfig& config, const FontParams& a_params);

		ImFont* font;
	};

	class FontStyles : public RE::BSTEventSink<RE::ApplyColorUpdateEvent>
	{
	public:
		static FontStyles* GetSingleton()
		{
			return &instance;
		}

		void Register();
		void LoadFontStyles();

		void LoadFonts(const Font::FontParams& a_primaryFont, const Font::FontParams& a_secondaryFont);

		ImVec4 GetGameplayHUDColor() const { return hudGameplayColor; }
		ImVec4 GetSubtitleColor() const { return subtitleColor; }

	private:
		RE::BSEventNotifyControl ProcessEvent(const RE::ApplyColorUpdateEvent& a_event, RE::BSTEventSource<RE::ApplyColorUpdateEvent>* a_source) override;

		// members
		Font primaryFont{};
		Font secondaryFont{};

		ImVec4 subtitleColor;
		ImVec4 hudGameplayColor;

		static FontStyles instance;
	};

	inline constinit FontStyles FontStyles::instance;
}
