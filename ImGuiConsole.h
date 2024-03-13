#pragma once

#include <imgui.h>
#include <ctype.h>
#include <stdio.h>
#include "Console.hpp"
#include "ConsoleWindow.h"
#include <format>

extern struct ImGuiConsole {
	struct ConsoleLine {
		UTILS::ConType type = UTILS::ConsoleLogType_Default;
		UTILS::ConColor color = UTILS::ConColor_WHITE;
		std::string msg;
	};

	char                  InputBuf[256];
	ImVector<ConsoleLine*>Items;
	ImVector<const char*> Commands;
	ImVector<char*>       History;
	int                   HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
	ImGuiTextFilter       Filter;
	bool                  AutoScroll;
	bool				  WrapText;
	bool                  ScrollToBottom;

	ImGuiConsole();
	~ImGuiConsole();
	void ClearLog();
	void AddLog(const std::string& msg, UTILS::ConColor color = UTILS::ConColor_WHITE, UTILS::ConType type = UTILS::ConsoleLogType_Default);;

	static int   Stricmp(const char* s1, const char* s2);
	static int   Strnicmp(const char* s1, const char* s2, int n);
	static char* Strdup(const char* s);
	static void  Strtrim(char* s);

	void Draw();

	void ExecCommand(const char* command_line);

	static int TextEditCallbackStub(ImGuiInputTextCallbackData* data);
	int TextEditCallback(ImGuiInputTextCallbackData* data);
};