#include "Hooks.h"
#include "ImGui/FontStyles.h"
#include "ImGui/Renderer.h"
#include "Manager.h"

void OnInit(F4SE::MessagingInterface::Message* a_msg)
{
	switch (a_msg->type) {
	case F4SE::MessagingInterface::kPostLoad:
		{
			logger::info("{:*^30}", "POST LOAD");
			ImGui::Renderer::Install();
			Hooks::Install();
		}
		break;
	case F4SE::MessagingInterface::kPostPostLoad:
		{
			logger::info("{:*^30}", "POST POST LOAD");
		}
		break;
	case F4SE::MessagingInterface::kGameDataReady:
		{
			logger::info("{:*^30}", "GAME DATA READY");
			ImGui::Renderer::Init();
			ImGui::FontStyles::GetSingleton()->Register();
			Manager::GetSingleton()->OnDataLoaded();
		}
		break;
	default:
		break;
	}
}

void InitializeLog()
{
	auto path = logger::log_directory();
	if (!path) {
		stl::report_and_fail("Failed to find standard logging directory"sv);
	}

	*path /= Version::PROJECT;
	*path += ".log"sv;
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::info);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%H:%M:%S:%e] %v"s);

	logger::info(FMT_STRING("{} v{}"), Version::PROJECT, Version::NAME);
}

extern "C" DLLEXPORT constinit auto F4SEPlugin_Version = []() noexcept {
	F4SE::PluginVersionData data{};

	data.PluginVersion({ Version::MAJOR, Version::MINOR, Version::PATCH });
	data.PluginName(Version::PROJECT.data());
	data.AuthorName("powerofthree");
	data.UsesAddressLibrary(true);
	data.UsesSigScanning(false);
	data.IsLayoutDependent(true);
	data.HasNoStructUse(false);
	data.CompatibleVersions({ F4SE::RUNTIME_LATEST });

	return data;
}();

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Load(const F4SE::LoadInterface* a_f4se)
{
	F4SE::Init(a_f4se, false);

	InitializeLog();

	logger::info("Game version : {}", a_f4se->RuntimeVersion().string());

	F4SE::AllocTrampoline(1 << 7);

	const auto messaging = F4SE::GetMessagingInterface();
	messaging->RegisterListener(OnInit);

	return true;
}
