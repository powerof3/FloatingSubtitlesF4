#pragma once

#include "Localization.h"
#include "RE.h"
#include "Subtitles.h"

class Manager :
	public REX::Singleton<Manager>,
	public RE::BSTEventSink<RE::MenuOpenCloseEvent>,
	public RE::BSTEventSink<RE::TESLoadGameEvent>,
	public RE::BSTValueEventSink<RE::PlayerCrosshairModeEvent>
{
public:
	void OnDataLoaded();
	void LoadGlobalSettings();

	bool SkipRender() const;
	void Draw();

	void AddSubtitle(RE::SubtitleManager* a_manager, const char* a_subtitle);
	bool UpdateSubtitleInfo(RE::SubtitleManager* a_manager);

private:
	struct GlobalSettings
	{
		bool LoadGlobalSettings();

		RE::Global<bool, 0x800>  showSpeakerName{ true };
		RE::Global<float, 0x801> subtitleSize{ 27.0f };
		RE::Global<bool, 0x802>  showDualSubs{ false };
		RE::Global<float, 0x803> subtitleHeadOffset{ 15.0f };
		RE::Global<bool, 0x804>  requireLOS{ true };
		RE::Global<float, 0x809> subtitleSpacing{ 0.5f };
		RE::Global<float, 0x805> obscuredSubtitleAlpha{ 0.35f };
		RE::Global<float, 0x808> subtitleAlphaPrimary{ 1.0f };
		RE::Global<float, 0x80C> subtitleAlphaSecondary{ 1.0f };
	};

	using SubtitleFlag = RE::SubtitleInfoEx::Flag;
	using RWLock = std::shared_mutex;
	using ReadLocker = std::shared_lock<RWLock>;
	using WriteLocker = std::unique_lock<RWLock>;

	bool                ShowGeneralSubtitles() const;
	bool                ShowDialogueSubtitles() const;
	DualSubtitle        CreateDualSubtitles(const char* subtitle) const;
	void                AddProcessedSubtitle(const char* subtitle);
	const DualSubtitle& GetProcessedSubtitle(const RE::BSFixedStringCS& a_subtitle);
	void                RebuildProcessedSubtitles();
	RE::NiPoint3        CalculateSubtitleAnchorPos(const RE::SubtitleInfoEx& a_subInfo) const;
	static RE::NiPoint3 GetSubtitleAnchorPosImpl(const RE::TESObjectREFRPtr& a_ref, float a_height);
	void                CalculateAlphaModifier(RE::SubtitleInfoEx& a_subInfo) const;
	void                CalculateVisibility(RE::SubtitleInfoEx& a_subInfo);
	std::string         GetScaleformSubtitle(const RE::BSFixedStringCS& a_subtitle);
	void                ClearScaleformSubtitle(RE::BSTValueEventSource<RE::HUDSubtitleDisplayEvent>& a_event, std::string_view a_subtitle);
	void                ClearScaleformSubtitle();

	RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent& a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override;
	RE::BSEventNotifyControl ProcessEvent(const RE::TESLoadGameEvent& a_event, RE::BSTEventSource<RE::TESLoadGameEvent>*) override;
	RE::BSEventNotifyControl ProcessEvent(const RE::PlayerCrosshairModeEvent& a_event, RE::BSTEventSource<RE::PlayerCrosshairModeEvent>*) override;

	// members
	mutable RWLock                     subtitleLock;
	FlatMap<std::string, DualSubtitle> processedSubtitles;
	GlobalSettings                     settings;
	float                              maxDistanceStartSq{ 4194304.0f };
	float                              maxDistanceEndSq{ 4624220.16f };
	LocalizedSubtitles                 localizedSubs;
	std::uint32_t                      crosshairMode{ 0 };
};
