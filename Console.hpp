#pragma once

#include <unordered_set>
#include <functional>
#include <sstream>
#include <fstream>
#include <string>
#include <mutex>
#include <Windows.h>
#include <imgui.h>

#define ConsoleOffset 0x128D6E0

namespace UTILS
{
	class Contraption;

	enum ConColor : WORD
	{
		ConColor_RED = FOREGROUND_RED,
		ConColor_GREEN = FOREGROUND_GREEN,
		ConColor_BLUE = FOREGROUND_BLUE,
		ConColor_YELLOW = ConColor_RED | ConColor_GREEN,
		ConColor_WHITE = ConColor_RED | ConColor_GREEN | ConColor_BLUE,

		ConColor_i = FOREGROUND_INTENSITY,
		ConColor_iRED = ConColor_RED | ConColor_i,
		ConColor_iGREEN = ConColor_GREEN | ConColor_i,
		ConColor_iBLUE = ConColor_BLUE | ConColor_i,
		ConColor_iYELLOW = ConColor_YELLOW | ConColor_i,
		ConColor_iWHITE = ConColor_WHITE | ConColor_i,
	};

	extern bool* DevFlag;

	extern float IntensityHigh;
	extern float IntensityLow;
	extern ImVec4 colorFromEnum(ConColor);

	enum ConType : std::uint32_t
	{
		ConsoleLogType_Default = 0x1,
		ConsoleLogType_Profile = 0x2,
		ConsoleLogType_Resource = 0x4,
		ConsoleLogType_Shader = 0x8,
		ConsoleLogType_Buffer = 0x10,
		ConsoleLogType_Render = 0x20,
		ConsoleLogType_Network = 0x40,
		ConsoleLogType_System = 0x80,
		ConsoleLogType_Terrain = 0x100,
		ConsoleLogType_World = 0x200,
		ConsoleLogType_Audio = 0x400,
		ConsoleLogType_Lua = 0x800,
		ConsoleLogType_Print = 0x1000,
		ConsoleLogType_UGC = 0x2000,
		ConsoleLogType_Steam = 0x4000,
		ConsoleLogType_Warning = 0x40000000,
		ConsoleLogType_Error = 0x80000000,
	};

	using fConsoleLog = std::add_pointer< void(void*, const std::string&, ConColor, ConType) >::type;

	class ConsoleLog__vftable {
		uintptr_t function0;
	public:
		fConsoleLog m_Log;
	private:
		uintptr_t function1;
	};

	using fCreateDebugConsole = std::add_pointer< int64_t(void*) >::type;

	void CreateDebugConsole();

	class Console
	{
	public:
		virtual ~Console() = default;
		virtual void log(const std::string& msg, WORD color, std::uint32_t type) {};
		virtual bool logNoRepeat(const std::string& msg, WORD color, std::uint32_t type) {};

		static Console* getInstance() {
			return *reinterpret_cast<Console**>(std::uintptr_t(GetModuleHandle(NULL)) + ConsoleOffset);
		}
	private:
		template<typename CurArg>
		inline void variadic_internal(std::stringstream& stream, const CurArg& cur_arg)
		{
			stream << cur_arg;
		}

		template<typename CurArg, typename ...Args>
		inline void variadic_internal(std::stringstream& stream, const CurArg& cur_arg, const Args& ...arg_list)
		{
			this->variadic_internal(stream, cur_arg);
			this->variadic_internal(stream, arg_list...);
		}

	public:
		template<typename ...Args>
		inline void log_variadic(WORD color, std::uint32_t type, const Args& ...args)
		{
			std::stringstream v_stream;
			this->variadic_internal(v_stream, args...);

			this->log(v_stream.str(), color, type);
		}

		template<typename ...Args>
		inline void log_variadic_no_repeat(WORD color, std::uint32_t type, const Args& ...args)
		{
			std::stringstream v_stream;
			this->variadic_internal(v_stream, args...);

			this->logNoRepeat(v_stream.rdbuf()->str(), color, type);
		}

		inline static std::function<void(const char*, const char*, unsigned int)> sm_assertHandler = nullptr;

	private:
		inline static CRITICAL_SECTION sm_logCritSection;
		inline static CRITICAL_SECTION sm_logNoRepeatCritSection;
		inline static std::unordered_set<std::uint64_t> sm_messageCache = {};

		/* 0x0008 */ std::function<void(const char*, unsigned int)> unknown_function;
		/* 0x0048 */ std::uint32_t m_uConOutMask;
		/* 0x004C */ std::uint32_t m_uFileOutMask;
		/* 0x0050 */ std::uint32_t m_uCallbackOutMask;
		/* 0x0054 */ char pad_0x54[0x4];
		/* 0x0058 */ HANDLE m_hConsole = NULL;
		/* 0x0060 */ std::ofstream m_logWriter;
		/* 0x0168 */ char pad_0x168[0x10];
		/* 0x0178 */ bool m_bCodePageSet = false;
		/* 0x0179 */ bool m_bConsoleAllocated = false;
		/* 0x017A */ char pad_0x17A[0x2];
		/* 0x017C */ std::uint32_t m_uWarningCounter = 0;
		/* 0x0180 */ std::uint32_t m_uErrorCounter = 0;
		/* 0x0184 */ char pad_0x184[0x4];
	};
}

#define LOG_PTR UTILS::Console::getInstance
#define LOG_FILE_INFO __FILE__, ":", (unsigned int)(__LINE__), " "

#define LOG_ERROR_TEMPLATE(func, con_type, ...) LOG_PTR()->func(UTILS::ConColor_iRED, UTILS::ConsoleLogType_Error | con_type, LOG_FILE_INFO, __VA_ARGS__)
#define LOG_WARNING_TEMPLATE(func, con_type, ...) LOG_PTR()->func(UTILS::ConColor_iYELLOW, UTILS::ConsoleLogType_Warning | con_type, LOG_FILE_INFO, __VA_ARGS__)

#define LOG_RENDER_ERROR(...) LOG_ERROR_TEMPLATE(log_variadic, UTILS::ConsoleLogType_Render, __VA_ARGS__)
#define LOG_RENDER_WHITE(...) LOG_PTR()->log_variadic(UTILS::ConColor_WHITE, UTILS::ConsoleLogType_Render, __VA_ARGS__)
#define LOG_RESOURCE_ERROR(...) LOG_ERROR_TEMPLATE(log_variadic, UTILS::ConsoleLogType_Resource, __VA_ARGS__)
#define LOG_SYSTEM_YELLOW(...) LOG_PTR()->log_variadic(UTILS::ConColor_iYELLOW, UTILS::ConsoleLogType_System, __VA_ARGS__)
#define LOG_SYSTEM_WHITE(...) LOG_PTR()->log_variadic(UTILS::ConColor_WHITE, UTILS::ConsoleLogType_System, __VA_ARGS__)
#define LOG_DEFAULT_WHITE(...) LOG_PTR()->log_variadic(UTILS::ConColor_WHITE, UTILS::ConsoleLogType_Default, __VA_ARGS__)
#define LOG_DEFAULT_GREEN(...) LOG_PTR()->log_variadic(UTILS::ConColor_iGREEN, UTILS::ConsoleLogType_Default, __VA_ARGS__)
#define LOG_SHADER_INTENSITY(...) LOG_PTR()->log_variadic(UTILS::ConColor_i, UTILS::ConsoleLogType_Shader, __VA_ARGS__)
#define LOG_SHADER_ERROR(...) LOG_ERROR_TEMPLATE(log_variadic, UTILS::ConsoleLogType_Shader, __VA_ARGS__)
#define LOG_SHADER_VIOLET(...) LOG_PTR()->log_variadic(UTILS::ConColor_RED | UTILS::ConColor_BLUE, UTILS::ConsoleLogType_Shader, __VA_ARGS__)
#define LOG_ERROR(...) LOG_ERROR_TEMPLATE(log_variadic, 0, __VA_ARGS__)
#define LOG_ERROR_NO_REPEAT(...) LOG_ERROR_TEMPLATE(log_variadic_no_repeat, 0, __VA_ARGS__)
#define LOG_ERROR_NO_FILE(...) LOG_PTR()->log_variadic(UTILS::ConColor_iRED, UTILS::ConsoleLogType_Error, __VA_ARGS__)
#define LOG_WARNING(...) LOG_WARNING_TEMPLATE(log_variadic, UTILS::ConsoleLogType_Warning, __VA_ARGS__)
#define LOG_WARNING_NO_REPEAT(...) LOG_WARNING_TEMPLATE(log_variadic_no_repeat, UTILS::ConsoleLogType_Warning, __VA_ARGS__)
#define SM_ASSERT(expression) (void)(                                                    \
        (!!(expression)) ||                                                              \
        (UTILS::Console::sm_assertHandler(#expression, __FILE__, (unsigned int)(__LINE__)), 0) \
    )