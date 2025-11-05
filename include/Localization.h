#pragma once

enum class Language
{
	kNative = static_cast<std::underlying_type_t<Language>>(-1),
	kChinese,
	kGerman,
	kEnglish,
	kSpanish,
	kLatinAmericanSpanish,
	kFrench,
	kItalian,
	kJapanese,
	kPolish,
	kPortuguese,
	kRussian,

	kTotal
};

std::string to_string(Language lang);
Language    to_language(std::string_view string);

template <std::uint32_t id1, std::uint32_t id2>
struct LanguageSetting
{
	bool operator==(const Language rhs) const { return language.get() == rhs; }

	bool LoadSettings(Language gameLanguage)
	{
		language.load();
		if (language.get() == Language::kNative) {
			language.set(gameLanguage);
		}
		maxCharsPerLine.load();

		return language.changed() || maxCharsPerLine.changed();  // rebuild subtitles
	}

	RE::Global<Language, id1>      language{ Language::kNative };
	RE::Global<std::uint32_t, id2> maxCharsPerLine{ 80 };
};

struct LocalizedSubtitle
{
	bool empty() const { return subtitle.empty(); }
	bool operator==(const LocalizedSubtitle& rhs) const { return subtitle == rhs.subtitle; }
	bool operator!=(const LocalizedSubtitle& rhs) const { return subtitle != rhs.subtitle; }

	std::string   subtitle;
	std::uint32_t maxCharsPerLine;
	Language      language{ Language::kEnglish };
};

class LocalizedSubtitles
{
public:
	void BuildLocalizedSubtitles();

	bool LoadGlobalSettings();
	void PostSettingsLoad();

	LocalizedSubtitle GetPrimarySubtitle(const char* a_localSubtitle) const;
	LocalizedSubtitle GetSecondarySubtitle(const char* a_localSubtitle) const;

private:
	using SubtitleID = std::uint64_t;  // hashed id (string id + mod index)

	using MultiSubtitleToIDMap = FlatMap<std::string, FlatSet<SubtitleID>>;
	using MultiIDToSubtitleMap = FlatMap<SubtitleID, FlatMap<Language, FlatSet<std::string>>>;

	using SubtitleToIDMap = FlatMap<std::string, SubtitleID>;
	using IDToSubtitleMap = FlatMap<SubtitleID, FlatMap<Language, std::string>>;

	void ReadILStringFiles(MultiSubtitleToIDMap& a_multiSubToID, MultiIDToSubtitleMap& a_multiIDToSub) const;
	void MergeDuplicateSubtitles(const MultiSubtitleToIDMap& a_multiSubToID, const MultiIDToSubtitleMap& a_multiIDToSub);

	template <std::uint32_t id1, std::uint32_t id2>
	std::string ResolveSubtitle(const char* a_localSubtitle, const LanguageSetting<id1, id2>& a_language) const
	{
		if (a_language == gameLanguage) {
			return a_localSubtitle;
		}

		if (const auto idIt = subtitleToID.find(a_localSubtitle); idIt != subtitleToID.end()) {
			if (const auto mapIt = idToSubtitle.find(idIt->second); mapIt != idToSubtitle.end()) {
				if (const auto subtitleIt = mapIt->second.find(a_language.language.get()); subtitleIt != mapIt->second.end()) {
					return subtitleIt->second;
				}
			}
		}

		return a_localSubtitle;
	}

	template <std::uint32_t id1, std::uint32_t id2>
	LocalizedSubtitle GetLocalizedSubtitle(const char* a_localSubtitle, const LanguageSetting<id1, id2>& a_setting) const
	{
		auto localizedSub = ResolveSubtitle(a_localSubtitle, a_setting);
		return { localizedSub, a_setting.maxCharsPerLine.get(), localizedSub == a_localSubtitle ? gameLanguage : a_setting.language.get() };
	}

	// members
	Language                      gameLanguage{ Language::kEnglish };
	LanguageSetting<0x806, 0x807> primaryLanguage;
	LanguageSetting<0x80A, 0x80B> secondaryLanguage;
	SubtitleToIDMap               subtitleToID;
	IDToSubtitleMap               idToSubtitle;
};
