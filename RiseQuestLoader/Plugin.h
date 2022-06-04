#pragma once

#include "imgui/font_robotomedium.hpp"
#include "reframework/API.hpp"

#define REDIRECT_COUT 0

#if REDIRECT_COUT
#include <iostream>

#include "OutputDebugStringBuf.h"
#endif

extern "C" lua_State* g_lua;
inline bool g_initialized = false;

void set_imgui_style() noexcept;
void redirect_cout();
