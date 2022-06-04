#include "Plugin.h"
#include "QuestLoader.h"

#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_dx12.h"
#include "imgui/imgui_impl_win32.h"

#include "rendering/d3d11.hpp"
#include "rendering/d3d12.hpp"


using namespace reframework;
using REGenericFunction = void*(*)(...);


bool initialize() {
    if (g_initialized) {
        return false;
    }

    ImGui::CreateContext();

    const auto renderer = API::get()->param()->renderer_data;

    DXGI_SWAP_CHAIN_DESC desc{};
    const auto swapchain = static_cast<IDXGISwapChain*>(renderer->swapchain);
    swapchain->GetDesc(&desc);

    if (!ImGui_ImplWin32_Init(desc.OutputWindow)) {
        return false;
    }

    if (renderer->renderer_type == REFRAMEWORK_RENDERER_D3D11) {
        if (!g_d3d11.initialize()) {
            return false;
        }
    } else if (renderer->renderer_type == REFRAMEWORK_RENDERER_D3D12) {
        if (!g_d3d12.initialize()) {
            return false;
        }
    }

    set_imgui_style();

    g_initialized = true;
    return true;
}

inline void imgui_render() {
    QuestLoader::get()->render_ui();
}

void on_present() {
    if (!g_initialized) {
        if (!initialize()) {
            return;
        }
    }

    const auto renderer = API::get()->param()->renderer_data;

    if (renderer->renderer_type == REFRAMEWORK_RENDERER_D3D11) {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        imgui_render();

        ImGui::EndFrame();
        ImGui::Render();

        g_d3d11.render_imgui();
    } else if (renderer->renderer_type == REFRAMEWORK_RENDERER_D3D12) {
        const auto command_queue = static_cast<ID3D12CommandQueue*>(renderer->command_queue);

        if (command_queue == nullptr) {
            return;
        }

        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        imgui_render();

        ImGui::EndFrame();
        ImGui::Render();

        g_d3d12.render_imgui();
    }
}

void on_device_reset() {
    const auto renderer_data = API::get()->param()->renderer_data;

    if (renderer_data->renderer_type == REFRAMEWORK_RENDERER_D3D11) {
        ImGui_ImplDX11_Shutdown();
        g_d3d11 = {};
    }

    if (renderer_data->renderer_type == REFRAMEWORK_RENDERER_D3D12) {
        ImGui_ImplDX12_Shutdown();
        g_d3d12 = {};
    }

    g_initialized = false;
}

bool on_message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);

    return !ImGui::GetIO().WantCaptureMouse && !ImGui::GetIO().WantCaptureKeyboard;
}

extern "C" {

__declspec(dllexport) void reframework_plugin_required_version(REFrameworkPluginVersion* version) {
    version->major = REFRAMEWORK_PLUGIN_VERSION_MAJOR;
    version->minor = REFRAMEWORK_PLUGIN_VERSION_MINOR;
    version->patch = REFRAMEWORK_PLUGIN_VERSION_PATCH;

    version->game_name = "MHRISE";
}

__declspec(dllexport) bool reframework_plugin_initialize(const REFrameworkPluginInitializeParam* param) {
    OutputDebugString(TEXT("[FEXTY] REFramework Initialize"));
    API::initialize(param);
    ImGui::CreateContext();

    const auto funcs = param->functions;
    funcs->on_message(reinterpret_cast<REFOnMessageCb>(on_message));
    funcs->on_device_reset(on_device_reset);
    funcs->on_present(on_present);

    return true;
}

}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        redirect_cout();
    }

    return TRUE;
}


void set_imgui_style() noexcept {
    ImGui::StyleColorsDark();

    auto& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.ScrollbarRounding = 2.0f;
    style.GrabRounding = 0.0f;
    style.TabRounding = 0.0f;
    style.WindowBorderSize = 2.0f;
    style.WindowPadding = ImVec2(2.0f, 0.0f);

    auto& colors = ImGui::GetStyle().Colors;
    // Window BG
    colors[ImGuiCol_WindowBg] = ImVec4{0.1f, 0.105f, 0.11f, 1.0f};

    // Navigatation highlight
    colors[ImGuiCol_NavHighlight] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};

    // Headers
    colors[ImGuiCol_Header] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
    colors[ImGuiCol_HeaderHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
    colors[ImGuiCol_HeaderActive] = ImVec4{0.55f, 0.5505f, 0.551f, 1.0f};

    // Buttons
    colors[ImGuiCol_Button] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
    colors[ImGuiCol_ButtonHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
    colors[ImGuiCol_ButtonActive] = ImVec4{0.55f, 0.5505f, 0.551f, 1.0f};

    // Checkbox
    colors[ImGuiCol_CheckMark] = ImVec4(0.55f, 0.5505f, 0.551f, 1.0f);

    // Frame BG
    colors[ImGuiCol_FrameBg] = ImVec4{0.211f, 0.210f, 0.25f, 1.0f};
    colors[ImGuiCol_FrameBgHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
    colors[ImGuiCol_FrameBgActive] = ImVec4{0.55f, 0.5505f, 0.551f, 1.0f};

    // Tabs
    colors[ImGuiCol_Tab] = ImVec4{0.25f, 0.2505f, 0.251f, 1.0f};
    colors[ImGuiCol_TabHovered] = ImVec4{0.38f, 0.3805f, 0.381f, 1.0f};
    colors[ImGuiCol_TabActive] = ImVec4{0.28f, 0.2805f, 0.281f, 1.0f};
    colors[ImGuiCol_TabUnfocused] = ImVec4{0.25f, 0.2505f, 0.251f, 1.0f};
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4{0.8f, 0.805f, 0.81f, 1.0f};

    // Resize Grip
    colors[ImGuiCol_ResizeGrip] = ImVec4{0.2f, 0.205f, 0.21f, 0.0f};
    colors[ImGuiCol_ResizeGripHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
    colors[ImGuiCol_ResizeGripActive] = ImVec4{0.55f, 0.5505f, 0.551f, 1.0f};

    // Title
    colors[ImGuiCol_TitleBg] = ImVec4{0.25f, 0.2505f, 0.251f, 1.0f};
    colors[ImGuiCol_TitleBgActive] = ImVec4{0.55f, 0.5505f, 0.551f, 1.0f};
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.25f, 0.2505f, 0.251f, 1.0f};

    const auto& fonts = ImGui::GetIO().Fonts;

    fonts->Clear();
    fonts->AddFontFromMemoryCompressedTTF(RobotoMedium_compressed_data, RobotoMedium_compressed_size, 16.0f);
    fonts->Build();
}

void redirect_cout() {
#if REDIRECT_COUT && defined(_WIN32)
    static OutputDebugStringBuf<char> charStringBuf;
    static OutputDebugStringBuf<wchar_t> wcharStringBuf;

    std::cout.rdbuf(&charStringBuf);
    std::cerr.rdbuf(&charStringBuf);

    std::wcout.rdbuf(&wcharStringBuf);
    std::wcerr.rdbuf(&wcharStringBuf);
#endif
}
