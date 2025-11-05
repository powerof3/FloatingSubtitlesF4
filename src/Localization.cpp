#include "Localization.h"

#include "RE.h"

std::string to_string(Language lang)
{
	switch (lang) {
	case Language::kChinese:
		return "CN";
	case Language::kGerman:
		return "DE";
	case Language::kSpanish:
		return "ES";
	case Language::kLatinAmericanSpanish:
		return "ESMX";
	case Language::kFrench:
		return "FR";
	case Language::kItalian:
		return "IT";
	case Language::kJapanese:
		return "JA";
	case Language::kPolish:
		return "PL";
	case Language::kPortuguese:
		return "PTBR";
	case Language::kRussian:
		return "RUS";
	default:
		return "EN";
	}
}

Language to_language(std::string_view string)
{
	switch (string::const_hash(string)) {
	case "CN"_h:
		return Language::kChinese;
	case "DE"_h:
		return Language::kGerman;
	case "ES"_h:
		return Language::kSpanish;
	case "ESMX"_h:
		return Language::kLatinAmericanSpanish;
	case "FR"_h:
		return Language::kFrench;
	case "IT"_h:
		return Language::kItalian;
	case "JA"_h:
		return Language::kJapanese;
	case "PL"_h:
		return Language::kPolish;
	case "PTBR"_h:
		return Language::kPortuguese;
	case "RUS"_h:
		return Language::kRussian;
	default:
		return Language::kEnglish;
	}
}

bool LocalizedSubtitles::LoadGlobalSettings()
{
	bool rebuildSubs = false;
	rebuildSubs |= primaryLanguage.LoadSettings(gameLanguage);
	rebuildSubs |= secondaryLanguage.LoadSettings(gameLanguage);
	return rebuildSubs;
}

void LocalizedSubtitles::PostSettingsLoad()
{
	if (primaryLanguage.language.get() == Language::kNative) {
		primaryLanguage.language.set(gameLanguage);
	}
	if (secondaryLanguage.language.get() == Language::kNative) {
		secondaryLanguage.language.set(gameLanguage);
	}
}

void LocalizedSubtitles::ReadILStringFiles(MultiSubtitleToIDMap& a_multiSubToID, MultiIDToSubtitleMap& a_multiIDToSub) const
{
	const auto& ilStringMap = RE::GetILStringMap();
	for (const auto& [fileName, info] : ilStringMap) {
		auto mod = RE::TESDataHandler::GetSingleton()->LookupModByName(fileName);
		if (!mod) {
			continue;
		}

		std::string_view baseName = fileName;
		baseName.remove_suffix(4);  // remove ".esm"

		for (auto language : stl::enum_range(Language::kChinese, Language::kTotal)) {
			std::string                  path = std::format("STRINGS\\{}_{}.ILSTRINGS", baseName, to_string(language));
			RE::BSResourceNiBinaryStream stream(path.c_str());

			if (!stream.good() || stream.stream->totalSize < 8) {
				continue;
			}

			std::vector<std::byte> buffer(stream.stream->totalSize);
			stream.read(buffer.data(), static_cast<std::uint32_t>(buffer.size()));

			RE::ILStringTable stringTable(buffer);

			for (const auto& [stringID, offset] : stringTable.directory) {
				std::string str = stringTable.GetStringAtOffset(offset);
				if (str.empty() || string::is_only_space(str)) {
					continue;
				}
				auto hashedStringID = hash::szudzik_pair(mod->compileIndex, stringID);
				if (language == gameLanguage) {
					a_multiSubToID[str].emplace(hashedStringID);
				}
				a_multiIDToSub[hashedStringID][language].emplace(str);
			}
		}
	}
}

void LocalizedSubtitles::MergeDuplicateSubtitles(const MultiSubtitleToIDMap& a_multiSubToID, const MultiIDToSubtitleMap& a_multiIDToSub)
{
	const auto pick_best_id = [&](const FlatSet<SubtitleID>& ids) {
		SubtitleID best = *ids.begin();

		std::size_t bestCount = std::numeric_limits<std::size_t>::max();
		std::size_t bestTotalLen = 0;

		for (SubtitleID id : ids) {
			const auto& langMap = a_multiIDToSub.at(id);

			std::size_t count = 0;
			std::size_t totalLen = 0;

			for (const auto& [lang, subs] : langMap) {
				count += subs.size();
				for (const auto& s : subs) {
					totalLen += s.size();
				}
			}

			if (count < bestCount || (count == bestCount && totalLen > bestTotalLen)) {
				best = id;
				bestCount = count;
				bestTotalLen = totalLen;
			}
		}
		return best;
	};

	const auto pick_best_subtitle = [&](SubtitleID bestID) {
		FlatMap<Language, std::string> singleStrings;
		for (auto& [lang, set] : a_multiIDToSub.at(bestID)) {
			if (!set.empty()) {
				singleStrings[lang] = *set.begin();  // take the first string
			}
		}
		return singleStrings;
	};

	for (auto& [subtitle, ids] : a_multiSubToID) {
		if (auto [it, result] = subtitleToID.try_emplace(subtitle, pick_best_id(ids)); result) {
			idToSubtitle.try_emplace(it->second, pick_best_subtitle(it->second));
		}
	}
}

void LocalizedSubtitles::BuildLocalizedSubtitles()
{
	gameLanguage = to_language("sLanguage:General"_ini.value_or("EN"));

	Timer timer;
	timer.start();

	MultiSubtitleToIDMap multiSubtitleToID;
	MultiIDToSubtitleMap multiIDToSubtitle;

	ReadILStringFiles(multiSubtitleToID, multiIDToSubtitle);
	MergeDuplicateSubtitles(multiSubtitleToID, multiIDToSubtitle);

	timer.stop();

	logger::info("Parsing .ILSTRINGS files took {}. {} localized strings found", timer.duration(), subtitleToID.size());
}

LocalizedSubtitle LocalizedSubtitles::GetPrimarySubtitle(const char* a_localSubtitle) const
{
	return GetLocalizedSubtitle(a_localSubtitle, primaryLanguage);
}

LocalizedSubtitle LocalizedSubtitles::GetSecondarySubtitle(const char* a_localSubtitle) const
{
	return GetLocalizedSubtitle(a_localSubtitle, secondaryLanguage);
}
