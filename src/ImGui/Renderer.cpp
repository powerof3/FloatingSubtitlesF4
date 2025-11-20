#include "ImGui/Renderer.h"

#include "ImGui/FontStyles.h"
#include "Manager.h"

namespace ImGui::Renderer
{
	void Init()
	{
		if (const auto renderer = RE::BSGraphics::RendererData::GetSingleton()) {
			const auto swapChain = (IDXGISwapChain*)renderer->renderWindow[0].swapChain;
			if (!swapChain) {
				logger::error("couldn't find swapChain");
				return;
			}

			DXGI_SWAP_CHAIN_DESC desc{};
			if (FAILED(swapChain->GetDesc(std::addressof(desc)))) {
				logger::error("IDXGISwapChain::GetDesc failed.");
				return;
			}

			const auto device = (ID3D11Device*)renderer->device;
			const auto context = (ID3D11DeviceContext*)renderer->context;

			logger::info("Initializing ImGui..."sv);

			ImGui::CreateContext();

			auto& io = ImGui::GetIO();
			io.IniFilename = nullptr;

			if (!ImGui_ImplWin32_Init(desc.OutputWindow)) {
				logger::error("ImGui initialization failed (Win32)");
				return;
			}
			if (!ImGui_ImplDX11_Init(device, context)) {
				logger::error("ImGui initialization failed (DX11)"sv);
				return;
			}

			RECT rect = { 0, 0, 0, 0 };
			GetClientRect((HWND)renderer->renderWindow[0].hwnd, &rect);
			ImGui::GetIO().DisplaySize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));

			ImGui::FontStyles::GetSingleton()->LoadFontStyles();

			logger::info("ImGui initialized.");

			initialized.store(true);
		}
	}
	
	// IMenu::PostDisplay
	struct PostDisplay
	{
		static void thunk(RE::IMenu* a_menu)
		{
			// Skip if Imgui is not loaded
			if (!initialized.load() || Manager::GetSingleton()->SkipRender()) {
				func(a_menu);
				return;
			}

			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			{
				// disable windowing
				GImGui->NavWindowingTarget = nullptr;

				Manager::GetSingleton()->Draw();
			}
			ImGui::EndFrame();
			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

			func(a_menu);
		}
		static inline REL::Relocation<decltype(thunk)> func;
		static inline std::size_t                      idx{ 0x6 };
	};

	void Install()
	{
		//REL::Relocation<std::uintptr_t> target{ REL::ID(2276814), 0x31D };  // BSGraphics::InitD3D
		//stl::write_thunk_call<CreateD3DAndSwapChain>(target.address());

		stl::write_vfunc<RE::HUDMenu, PostDisplay>();
	}
}
