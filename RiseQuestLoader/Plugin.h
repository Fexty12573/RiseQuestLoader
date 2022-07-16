#pragma once

#include "imgui/font_robotomedium.hpp"
#include "reframework/API.hpp"


extern "C" lua_State* g_lua;
inline bool g_initialized = false;

void set_imgui_style() noexcept;
void redirect_cout();
