#pragma once

enum class FileType
{
	kFonts
};

class SettingLoader
{
public:
	using INIFunc = std::function<void(CSimpleIniA&)>;

	static SettingLoader* GetSingleton()
	{
		return &instance;
	}

	void Load(FileType type, INIFunc a_func, bool a_generate = false) const;

private:
	static void LoadINI(const wchar_t* a_path, INIFunc a_func, bool a_generate = false);
	static void LoadINI(const wchar_t* a_defaultPath, const wchar_t* a_userPath, INIFunc a_func);

	// members
	const wchar_t* fontsPath{ L"Data/Interface/FloatingSubtitles/fonts.ini" };

	static SettingLoader instance;
};

inline constinit SettingLoader SettingLoader::instance;
