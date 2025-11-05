#include "ImGui/FontStyles.h"

#include "SettingLoader.h"
#include "ImGui/Util.h"

namespace ImGui
{
	Font::FontParams Font::LoadFontSettings(const CSimpleIniA& a_ini, const char* a_section)
	{
		FontParams params;
		
		params.name = a_ini.GetValue(a_section, "sFont", "");
		params.spacing = static_cast<float>(a_ini.GetDoubleValue(a_section, "fSpacing", 1.0));

		return params;
	}

	void Font::LoadFont(ImFontConfig& config, const Font::FontParams& a_params)
	{
		if (a_params.name.empty() || font) {
			return;
		}

		auto name = R"(Data\Interface\ImGuiFonts\)" + a_params.name;

		std::error_code ec;
		if (!std::filesystem::exists(name, ec)) {
			return;
		}

		const auto& io = ImGui::GetIO();
		config.GlyphExtraAdvanceX = a_params.spacing;
		font = io.Fonts->AddFontFromFileTTF(name.c_str(), 0.0f, &config);

		logger::info("Loaded font {}", a_params.name);
	}

	void FontStyles::LoadFonts(const Font::FontParams& a_primaryFont, const Font::FontParams& a_secondaryFont)
	{
		ImFontConfig config;
		primaryFont.LoadFont(config, a_primaryFont);
		config.MergeMode = true;
		secondaryFont.LoadFont(config, a_secondaryFont);
	}

	RE::BSEventNotifyControl FontStyles::ProcessEvent(const RE::ApplyColorUpdateEvent&, RE::BSTEventSource<RE::ApplyColorUpdateEvent>*)
	{		
		hudGameplayColor = GetColor(RE::HUDMenuUtils::GetGameplayHUDColor());
		
		subtitleColor.x = "uSubtitleR:Interface"_ini.value() / 255.0f;
		subtitleColor.y = "uSubtitleG:Interface"_ini.value() / 255.0f;
		subtitleColor.z = "uSubtitleB:Interface"_ini.value() / 255.0f;
		subtitleColor.w = 1.0f;

		return RE::BSEventNotifyControl::kContinue;
	}

	void FontStyles::Register()
	{
		RE::ApplyColorUpdateEvent::GetEventSource()->RegisterSink(this);
	}

	void FontStyles::LoadFontStyles()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_TextShadowDisabled] = ImVec4(0.0, 0.0, 0.0, 0.5);
		style.TextShadowOffset = ImVec2(1.5f, 1.5f);
		style.ScaleAllSizes(ImGui::GetResolutionScale());

		Font::FontParams primaryFontParams;
		Font::FontParams secondaryFontParams;

		// load fonts
		SettingLoader::GetSingleton()->Load(FileType::kFonts, [&](auto& ini) {
			primaryFontParams = primaryFont.LoadFontSettings(ini, "PrimaryFont");
			secondaryFontParams = secondaryFont.LoadFontSettings(ini, "SecondaryFont");
		});

		LoadFonts(primaryFontParams, secondaryFontParams);
	}
}
