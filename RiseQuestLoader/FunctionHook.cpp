#include "FunctionHook.h"
#include "Plugin.h"

#include <MinHook.h>

bool utility::FunctionHook::s_minhook_initialized = false;

utility::FunctionHook::FunctionHook(void* target, void* hook)
    : m_target(target)
    , m_original(nullptr)
    , m_hook(hook) {

    const auto& api = reframework::API::get();
    if (!s_minhook_initialized && MH_Initialize() == MH_OK) {
        s_minhook_initialized = true;
    }

    if (MH_CreateHook(target, hook, &m_original) != MH_OK) {
        api->log_error("Failed to create hook %p -> %p", target, hook);
    }
}

void utility::FunctionHook::create() const {
    if (!is_valid()) {
        return;
    }

    const auto& api = reframework::API::get();
    if (MH_EnableHook(m_target) != MH_OK) {
        api->log_error("Failed to enable hook %p -> %p", m_target, m_hook);
    }
}

void utility::FunctionHook::destroy() const {
    if (!is_valid()) {
        return;
    }

    const auto& api = reframework::API::get();
    if (MH_DisableHook(m_target) != MH_OK) {
        api->log_error("Failed to disable hook %p -> %p", m_target, m_hook);
    }
}
