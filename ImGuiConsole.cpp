#include "ImGuiConsole.h"

ImGuiConsole::ImGuiConsole()
{
	ClearLog();
	memset(InputBuf, 0, sizeof(InputBuf));
	HistoryPos = -1;

	Commands.push_back("help");
	Commands.push_back("history");
	Commands.push_back("clear");
	AutoScroll = true;
	WrapText = true;
	ScrollToBottom = false;
}

ImGuiConsole::~ImGuiConsole()
{
	ClearLog();
	for (int i = 0; i < History.Size; i++)
		ImGui::MemFree(History[i]);
}

// Portable helpers
int   ImGuiConsole::Stricmp(const char* s1, const char* s2) { int d; while ((d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; } return d; }
int   ImGuiConsole::Strnicmp(const char* s1, const char* s2, int n) { int d = 0; while (n > 0 && (d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; n--; } return d; }
char* ImGuiConsole::Strdup(const char* s) { IM_ASSERT(s); size_t len = strlen(s) + 1; void* buf = ImGui::MemAlloc(len); IM_ASSERT(buf); return (char*)memcpy(buf, (const void*)s, len); }
void  ImGuiConsole::Strtrim(char* s) { char* str_end = s + strlen(s); while (str_end > s && str_end[-1] == ' ') str_end--; *str_end = 0; }

void ImGuiConsole::ClearLog()
{
	for (int i = 0; i < Items.Size; i++)
		free(Items[i]);
	Items.clear();
}

void ImGuiConsole::AddLog(const std::string& msg, UTILS::ConColor color, UTILS::ConType type) {
	Items.push_back(new ConsoleLine{ type, color, msg });
}

/*void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
{
	// FIXME-OPT
	char buf[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
	buf[IM_ARRAYSIZE(buf) - 1] = 0;
	va_end(args);
	Items.push_back(Strdup(buf));
}
*/

void ImGuiConsole::Draw()
{
	// Options menu
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("Options")) {
			ImGui::Checkbox("Executor", &ConsoleWindow::ShowExecutorWindow);
			ImGui::Checkbox("Dev Flag", &*UTILS::DevFlag);
			ImGui::Checkbox("Always on top", &ConsoleWindow::ConsoleAlwaysOnTop);
			ImGui::Checkbox("Auto scroll", &AutoScroll);
			ImGui::Checkbox("Wrap text", &WrapText);
			ImGui::SliderFloat("Low Intensity", &UTILS::IntensityLow, 0.0f, 1.0f);
			ImGui::SliderFloat("High Intensity", &UTILS::IntensityHigh, 0.0f, 1.0f);

			ImGui::EndMenu();
		}

		ImGui::SameLine();
		if (ImGui::Button("Clear")) ClearLog();

		ImGui::SameLine();
		Filter.Draw("Filter", 180);

		ImGui::EndMenuBar();
	}

	ImGui::Separator();
	// Reserve enough left-over height for 1 separator + 1 input text
	const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
	if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
	{
		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::Selectable("Clear")) ClearLog();
			ImGui::EndPopup();
		}

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4)); // Tighten spacing
		for (const ConsoleLine* item : Items)
		{
			if (!Filter.PassFilter(item->msg.c_str()))
				continue;

			ImGui::PushStyleColor(ImGuiCol_Text, UTILS::colorFromEnum((UTILS::ConColor)item->color));

			if (WrapText) {
				ImGui::TextWrapped(item->msg.c_str());
				ImGui::Separator();
			}
			else {
				ImGui::TextUnformatted(item->msg.c_str());
			}

			ImGui::PopStyleColor();
		}

		if (ScrollToBottom || (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
			ImGui::SetScrollHereY(1.0f);
		ScrollToBottom = false;
		ImGui::PopStyleVar();
	}
	ImGui::EndChild();
	ImGui::Separator();
	// Command-line
	bool reclaim_focus = false;
	ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
	if (ImGui::InputText("Input", InputBuf, IM_ARRAYSIZE(InputBuf), input_text_flags, &TextEditCallbackStub, (void*)this))
	{
		char* s = InputBuf;
		Strtrim(s);
		if (s[0])
			ExecCommand(s);
		strcpy(s, "");
		reclaim_focus = true;
	}
	// Auto-focus on window apparition
	ImGui::SetItemDefaultFocus();
	if (reclaim_focus)
		ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
}

void ImGuiConsole::ExecCommand(const char* command_line)
{
	AddLog(std::format("# %s\n", command_line));
	// Insert into history. First find match and delete it so it can be pushed to the back.
	// This isn't trying to be smart or optimal.
	HistoryPos = -1;
	for (int i = History.Size - 1; i >= 0; i--)
		if (Stricmp(History[i], command_line) == 0)
		{
			ImGui::MemFree(History[i]);
			History.erase(History.begin() + i);
			break;
		}
	History.push_back(Strdup(command_line));
	// Process command
	if (Stricmp(command_line, "clear") == 0)
	{
		ClearLog();
	}
	else if (Stricmp(command_line, "help") == 0)
	{
		AddLog("Commands:");
		for (int i = 0; i < Commands.Size; i++)
			AddLog(std::format("- %s", Commands[i]));
	}
	else if (Stricmp(command_line, "history") == 0)
	{
		int first = History.Size - 10;
		for (int i = first > 0 ? first : 0; i < History.Size; i++)
			AddLog(std::format("%3d: %s\n", i, History[i]));
	}
	else
	{
		AddLog(std::format("Unknown command: '%s'\n", command_line));
	}
	// On command input, we scroll to bottom even if AutoScroll==false
	ScrollToBottom = true;
}

// In C++11 you'd be better off using lambdas for this sort of forwarding callbacks
int ImGuiConsole::TextEditCallbackStub(ImGuiInputTextCallbackData* data)
{
	ImGuiConsole* console = (ImGuiConsole*)data->UserData;
	return console->TextEditCallback(data);
}

int ImGuiConsole::TextEditCallback(ImGuiInputTextCallbackData* data)
{
	//AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
	switch (data->EventFlag)
	{
	case ImGuiInputTextFlags_CallbackCompletion:
	{
		// Example of TEXT COMPLETION
		// Locate beginning of current word
		const char* word_end = data->Buf + data->CursorPos;
		const char* word_start = word_end;
		while (word_start > data->Buf)
		{
			const char c = word_start[-1];
			if (c == ' ' || c == '\t' || c == ',' || c == ';')
				break;
			word_start--;
		}
		// Build a list of candidates
		ImVector<const char*> candidates;
		for (int i = 0; i < Commands.Size; i++)
			if (Strnicmp(Commands[i], word_start, (int)(word_end - word_start)) == 0)
				candidates.push_back(Commands[i]);
		if (candidates.Size == 0)
		{
			// No match
			AddLog(std::format("No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start));
		}
		else if (candidates.Size == 1)
		{
			// Single match. Delete the beginning of the word and replace it entirely so we've got nice casing.
			data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
			data->InsertChars(data->CursorPos, candidates[0]);
			data->InsertChars(data->CursorPos, " ");
		}
		else
		{
			// Multiple matches. Complete as much as we can..
			// So inputing "C"+Tab will complete to "CL" then display "CLEAR" and "CLASSIFY" as matches.
			int match_len = (int)(word_end - word_start);
			for (;;)
			{
				int c = 0;
				bool all_candidates_matches = true;
				for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
					if (i == 0)
						c = toupper(candidates[i][match_len]);
					else if (c == 0 || c != toupper(candidates[i][match_len]))
						all_candidates_matches = false;
				if (!all_candidates_matches)
					break;
				match_len++;
			}
			if (match_len > 0)
			{
				data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
				data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
			}
			// List matches
			AddLog("Possible matches:\n");
			for (int i = 0; i < candidates.Size; i++)
				AddLog(std::format("- %s\n", candidates[i]));
		}
		break;
	}
	case ImGuiInputTextFlags_CallbackHistory:
	{
		// Example of HISTORY
		const int prev_history_pos = HistoryPos;
		if (data->EventKey == ImGuiKey_UpArrow)
		{
			if (HistoryPos == -1)
				HistoryPos = History.Size - 1;
			else if (HistoryPos > 0)
				HistoryPos--;
		}
		else if (data->EventKey == ImGuiKey_DownArrow)
		{
			if (HistoryPos != -1)
				if (++HistoryPos >= History.Size)
					HistoryPos = -1;
		}
		// A better implementation would preserve the data on the current input line along with cursor position.
		if (prev_history_pos != HistoryPos)
		{
			const char* history_str = (HistoryPos >= 0) ? History[HistoryPos] : "";
			data->DeleteChars(0, data->BufTextLen);
			data->InsertChars(0, history_str);
		}
	}
	}
	return 0;
}
