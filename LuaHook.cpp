#include "LuaHook.h"
#include "ImGuiColorTextEdit/TextEditor.h">
#include "ConsoleWindow.h"
#include <iostream>

namespace LuaHook {
	int (*original) (lua_State* L, int narg) = nullptr;
	lua_State* lState = nullptr;

	int hooked_luaL_checknumber(lua_State* L, int narg) {
		if (!lState) {
			LOG_DEFAULT_WHITE("[SMCL] Got lua state!\n");
			lState = L;
		}

		return original(L, narg);
	}

	std::queue<std::string> scripts;

	//TODO: error check this, sm might not be valid in some states :(
	bool isServerMode(lua_State* L) {
		lua_getglobal(L, "sm");
		lua_getfield(L, -1, "isServerMode");
		lua_call(L, 0, 1);
		bool isServerMode = lua_toboolean(L, -1);
		lua_pop(L, 2);

		return isServerMode;
	}

	void luahook_ExecQueue(lua_State* L, lua_Debug* ar)
	{
		lua_sethook(L, nullptr, NULL, NULL);
		TextEditor::ErrorMarkers markers;

		// cursed recursive code 👉👈🤯
		//TODO: Make it not infinitley recurse :)
		if (isServerMode(L) != ConsoleWindow::PreferrServerState) {
			lua_sethook(lState, luahook_ExecQueue, LUA_MASKLINE, NULL);
			return;
		}

		while (!scripts.empty()) {
			if (luaL_loadstring(L, scripts.front().c_str()))
			{
				static const std::regex re(R"x(\[.+\]:(\d+): (.+))x");
				if (lua_type(L, -1) == LUA_TSTRING) {
					std::string msg = lua_tostring(L, -1);
					std::smatch match;
					if (std::regex_search(msg, match, re))
					{
						markers.insert(std::pair<int, std::string>(std::stoi(match[1].str()), match[2].str()));
						ConsoleWindow::editor.SetErrorMarkers(markers);
					}
				}

				lua_pop(L, 1);
			}
			else if (lua_pcall(L, 0, 0, 0))
			{
				static const std::regex re(R"x(\[.+\]:(\d+): (.+))x");
				if (lua_type(L, -1) == LUA_TSTRING) {
					std::string msg = lua_tostring(L, -1);
					std::smatch match;
					if (std::regex_search(msg, match, re))
					{
						markers.insert(std::pair<int, std::string>(std::stoi(match[1].str()), match[2].str()));
						ConsoleWindow::editor.SetErrorMarkers(markers);
					}
				}

				lua_pop(L, 1);
			}

			scripts.pop();
		}
	}

	void runstring(std::string str) {
		if (lState != nullptr) {
			scripts.push(str);

			lua_sethook(lState, luahook_ExecQueue, LUA_MASKLINE, NULL);
		}
		else {
			LOG_WARNING("[SMCL] Lua state not initialized yet!");
		}
	}

	void compilestring(std::string str) {
		static lua_State* state = luaL_newstate();

		TextEditor::ErrorMarkers markers;

		if (luaL_loadstring(state, str.c_str()))
		{
			static const std::regex re(R"x(\[.+\]:(\d+): (.+))x");
			if (lua_type(state, -1) == LUA_TSTRING) {
				std::string msg = lua_tostring(state, -1);
				std::smatch match;

				if (std::regex_search(msg, match, re))
				{
					markers.insert(std::pair<int, std::string>(std::stoi(match[1].str()), match[2].str()));
					ConsoleWindow::editor.SetErrorMarkers(markers);
				}
			}
		}
		lua_pop(state, 1);
	}

	void Hook() {
		MH_CreateHookApi(L"lua51.dll", "luaL_checknumber", hooked_luaL_checknumber, (LPVOID*)&original);
	}
}
