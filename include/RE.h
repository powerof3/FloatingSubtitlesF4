#pragma once

namespace RE
{
	using TESObjectREFRPtr = NiPointer<TESObjectREFR>;

	struct StringFileInfo
	{
		struct Entry
		{
			std::uint32_t id;
			std::uint32_t offset;
		};

		Entry*                              entryArray;
		std::uint32_t                       numEntries;
		std::uint8_t*                       stringBlock;
		std::uint32_t                       stringBlockSize;
		std::uint32_t                       stringBlockOffset;
		BSTSmartPointer<BSResource::Stream> stream;
		BSSpinLock                          streamLock;
		BSFixedString                       filePath;
	};

	[[nodiscard]] inline static auto GetILStringMap() -> BSTHashMap<BSFixedString, StringFileInfo*>&
	{
		static REL::Relocation<BSTHashMap<BSFixedString, StringFileInfo*>*> map{ REL::ID(2661471), -0x8 };
		return *map;
	}

	// https://en.uesp.net/wiki/Tes5Mod:String_Table_File_Format
	struct ILStringTable
	{
		struct DirectoryEntry
		{
			std::uint32_t stringID;  //	String ID
			std::uint32_t offset;    //	Offset (relative to beginning of data) to the string.
		};

		ILStringTable(const std::vector<std::byte>& a_buffer);
		std::string GetStringAtOffset(std::uint32_t offset) const;

		// members
		std::uint32_t               entryCount;  // Number of entries in the string table.
		std::uint32_t               dataSize;    // Size of string data that follows after header and directory.
		std::vector<DirectoryEntry> directory;
		std::vector<std::byte>      rawData;

	private:
		// increments offset
		static void read_uint32(std::uint32_t& val, const std::vector<std::byte>& a_buffer, std::uint32_t& a_bufferPosition);
	};

	class SubtitleInfoEx
	{
	public:
		enum class Flag : std::uint8_t
		{
			kNone = 0,
			kSkip = 1 << 0,
			kOffscreen = 1 << 1,
			kObscured = 1 << 2,
			kInitialized = 1 << 7,
		};

		std::uint8_t& flagsRaw() { return pad07; }
		bool          isFlagSet(Flag a_flag) const { return (pad07 & static_cast<std::uint8_t>(a_flag)) != 0; }

		void setFlag(Flag a_flag, bool a_set);

		void  setAlphaModifier(float value);
		float getAlphaModifier() const;

		//members
		ObjectRefHandle                             speaker;
		std::uint8_t                                pad04;  // alpha
		std::uint8_t                                pad05;  // alpha
		std::uint8_t                                pad06;  // alpha
		std::uint8_t                                pad07;  // flags
		BSFixedStringCS                             subtitleText;
		TESTopicInfo*                               topicInfo;
		REX::Enum<SUBTITLE_PRIORITY, std::uint32_t> priority;
		float                                       distFromPlayer;
	};
	static_assert(sizeof(SubtitleInfoEx) == 0x20);

	template <typename T, std::uint32_t id>
	struct Global
	{
		Global() = default;
		Global(T value):
			currentValue(value)
		{}
		
		const T& get() const { return currentValue; }
		bool     changed() const { return previousValue != currentValue; }

		void load()
		{
			load_global();
			previousValue = currentValue;
			if (global) {
				currentValue = static_cast<T>(global->GetValue());
			}
		}

		void set(const T& a_value)
		{
			load_global();
			previousValue = currentValue;
			currentValue = a_value;
			if (global) {
				global->value = static_cast<float>(currentValue);
			}
		}

		TESGlobal* global{};
		T          previousValue{};
		T          currentValue{};

	private:
		void load_global()
		{
			if (!global) {
				global = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(id, "FloatingSubtitles.esp");
			}
		}
	};

	template <class... Args>
	bool DispatchStaticCall(BSFixedString a_class, BSFixedString a_fnName, Args&&... a_args)
	{
		if (auto vm = GameVM::GetSingleton()->GetVM()) {
			static BSTSmartPointer<BSScript::IStackCallbackFunctor> callback;
			return vm->DispatchStaticCall(a_class, a_fnName, callback, std::forward<Args>(a_args)...);
		}
		return false;
	}

	bool        IsCrosshairRef(const TESObjectREFRPtr& a_ref);
	NiAVObject* GetHeadNode(const TESObjectREFRPtr& a_ref);
	NiAVObject* GetTorsoNode(const Actor* a_actor);
	void        BroadcastEvent(BSTValueEventSource<HUDSubtitleDisplayEvent>* a_this);
	const char* GetSpeakerName(SubtitleInfoEx& a_subInfo);
}
