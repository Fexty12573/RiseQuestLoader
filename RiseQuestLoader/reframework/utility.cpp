#include "utility.h"

#include <algorithm>

#include <Windows.h>
#include <Psapi.h>

#define DEBUG_LOG 1

void utility::log(const std::string& msg) {
#if DEBUG_LOG
    OutputDebugStringA(("[QuestLoader] " + msg).c_str());
#endif
}

std::string utility::narrow(std::wstring_view str) {
    const auto length = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.length()), nullptr, 0, nullptr, nullptr);
    std::string narrowStr{};

    narrowStr.resize(length);
    WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.length()), (LPSTR)narrowStr.c_str(), length, nullptr, nullptr);

    return narrowStr;
}

std::wstring utility::widen(std::string_view str) {
    const auto length = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.length()), nullptr, 0);
    std::wstring wideStr{};

    wideStr.resize(length);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.length()), (LPWSTR)wideStr.c_str(), length);

    return wideStr;
}

utility::Pattern utility::make_pattern(std::string_view pattern) {
    auto hexchar_to_int = [](char c) { return c >= 'A' ? c >= 'a' ? c - 'a' + 10 : c - 'A' + 10 : c - '0'; };
    auto to_byte = [&](char upper, char lower) { return static_cast<int16_t>(hexchar_to_int(upper) << 4 | hexchar_to_int(lower)); };

    std::string str{pattern};
    std::erase_if(str, isspace);

    const size_t len = str.size();
    
    Pattern p{};

    for (auto i = 0ull; i < len; ++i) {
        const char c = str[i];

        if (c != '?') {
            if (i + 1u >= len) {
                break;
            }

            const char other = str[i + 1u];

            p.push_back(to_byte(c, other));

            i += 2;
        } else {
            p.push_back(-1i16);
            i += 1;
        }
    }

    return p;
}

std::vector<void*> utility::scanmem(const Pattern& pattern) {
    std::vector<void*> results;
    const auto module = GetModuleHandle(TEXT("MonsterHunterRise.exe"));
    if (module == nullptr) {
        return results;
    }

    MODULEINFO moduleInfo;
    if (!GetModuleInformation(GetCurrentProcess(), module, &moduleInfo, sizeof(moduleInfo)))
        return results;

    const byte* startAddr = reinterpret_cast<byte*>(module);
    const byte* endAddr = startAddr + moduleInfo.SizeOfImage;
    const byte* addr = startAddr;

    constexpr auto pred = [](auto a, auto b) {
        return static_cast<int16_t>(b) != -1i16 && static_cast<uint8_t>(a) == static_cast<uint8_t>(b);
    };

    while (addr < endAddr) {
        MEMORY_BASIC_INFORMATION memInfo;
        if (!VirtualQuery(addr, &memInfo, sizeof(memInfo)) || memInfo.State != MEM_COMMIT || (memInfo.Protect & PAGE_GUARD))
            continue;
        const auto begin = static_cast<byte*>(memInfo.BaseAddress);
        byte* end = begin + memInfo.RegionSize;

        byte* found = std::search(begin, end, pattern.begin(), pattern.end(), pred);
        while (found != end) {
            results.push_back(found);
            found = std::search(found + 1, end, pattern.begin(), pattern.end(), pred);
        }

        addr = end;
        memInfo = {};
    }

    return results;
}

REString* utility::create_managed_string(std::string_view string) {
    using namespace reframework;
    const auto& api = API::get();

    return reinterpret_cast<REString*>(api->sdk()->functions->create_managed_string_normal(string.data()));
}

reframework::API::ManagedObject* utility::create_managed_array(reframework::API::ManagedObject* runtime_type, size_t length) {
    const auto& api = reframework::API::get();

    static auto system_array_type = api->tdb()->find_type("System.Array");
    static auto create_instance_method = system_array_type->find_method("CreateInstance");

    return call(create_instance_method, runtime_type, static_cast<uint32_t>(length));
}

reframework::API::ManagedObject* utility::create_managed_array(std::string_view type, size_t length) {
    return create_managed_array(reframework::API::get()->typeof(type.data()), length);
}

reframework::API::ManagedObject* utility::create_managed_array(const reframework::API::TypeDefinition* type, size_t length) {
    return create_managed_array(type->get_runtime_type(), length);
}

reframework::API::ManagedObject* utility::get_main_view() {
    const auto& api = reframework::API::get();

    static auto scene_manager_type = api->tdb()->find_type("via.SceneManager");
    static auto get_main_view_method = scene_manager_type->find_method("get_MainView");

    return call(get_main_view_method, api->get_native_singleton("via.SceneManager"));
}

reframework::API::ManagedObject* utility::get_current_scene() {
    const auto& api = reframework::API::get();

    static auto scene_manager_type = api->tdb()->find_type("via.SceneManager");
    static auto get_current_scene_method = scene_manager_type->find_method("get_CurrentScene");

    return call(get_current_scene_method, api->get_native_singleton("via.SceneManager"));
}

reframework::API::ManagedObject* utility::get_primary_camera() {
    const auto& api = reframework::API::get();
    const auto main_view = get_main_view();

    if (main_view == nullptr) {
        return nullptr;
    }

    static auto scene_view_type = api->tdb()->find_type("via.SceneView");
    static auto get_primary_camera_method = scene_view_type->find_method("get_PrimaryCamera");

    return call(get_primary_camera_method, main_view);
}
