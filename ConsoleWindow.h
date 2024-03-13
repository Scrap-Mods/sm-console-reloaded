#pragma once
#include <atomic>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <imgui_internal.h>
#include <d3d11.h>
#include <tchar.h>
#include "ImGuiColorTextEdit/TextEditor.h"
#include <nfd.h>
#include "LuaHook.h"
#include "Console.hpp"
#include "resource.h"
#include "crc32c/crc32c.h"
#include "ImGuiConsole.h"
#include <dwmapi.h>

namespace ConsoleWindow {
	int consoleWindow();
	extern TextEditor editor;

	extern bool ShowConsoleWindow;
	extern bool ShowExecutorWindow;
	extern bool ConsoleAlwaysOnTop;
};

extern std::atomic<bool> exitThreads;