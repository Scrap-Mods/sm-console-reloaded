#pragma once

#include "MinHook.h"
#include "lua/includes/lua.hpp"
#include <string>
#include <queue>
#include "Console.hpp"

namespace LuaHook {
	extern lua_State* lState;

	void Hook();
	void runstring(std::string);
	void compilestring(std::string);
}