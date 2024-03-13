
// dllmain.cpp : Defines the entry point for the DLL application.
#include <cstdio>
#include <atomic>
#include <Windows.h>
#include "ConsoleWindow.hpp"

#include "Console.hpp"
#include "LuaHook.hpp"

std::atomic<bool> exitThreads(false);

int main(HMODULE hModule)
{

    //TODO: Only attach console when contraption->allocated_console
    UTILS::Console* consoleptr = UTILS::Console::getInstance();

    if (consoleptr != nullptr && !consoleptr->m_bConsoleAllocated) {
        AllocConsole();
        HANDLE v_output_handle = GetStdHandle(STD_OUTPUT_HANDLE);

        consoleptr->m_bConsoleAllocated = true;
        consoleptr->m_hConsole = v_output_handle;

        if (!SetConsoleOutputCP(CP_UTF8))
        {
            MessageBoxW(NULL, L"Unable to setup debug console", L"Error", MB_ICONERROR);
            return 1;
        }

        consoleptr->m_bCodePageSet = true;
    }

    FILE* fstdin = stdin;
    FILE* fstdout = stdout;
    FILE* fstderr = stderr;

    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD cursorPosition = { 0, 0 }; // set the cursor position to (0, 0)
    DWORD numCellsWritten;
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    GetConsoleScreenBufferInfo(console, &consoleInfo);

    FILE* fstdin_new, * fstdout_new, * fstderr_new;
    freopen_s(&fstdin_new, "CONIN$", "r", stdin);
    freopen_s(&fstdout_new, "CONOUT$", "w", stdout);
    freopen_s(&fstderr_new, "CONOUT$", "w", stderr);

    SetConsoleTitleW(L"Debug Console [SMCL]");
    printf("[SMCL] Loaded!\n");

    MH_Initialize();
    LuaHook::Hook();
    MH_EnableHook(MH_ALL_HOOKS);

    HANDLE ConsoleWindow = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ConsoleWindow::consoleWindow, NULL, 0, NULL);

    LOG_DEFAULT_WHITE("Hello world from SMCL!");

    // Force game to be rescaleable and not fullscreen >:)
    // GetModuleHandle(NULL);

    while (!exitThreads)
    {
        if (GetKeyState(VK_F5) & 0x8000) break;
    }

    exitThreads = true;

    // Wait for console window to close
    WaitForSingleObject(ConsoleWindow, INFINITE);

    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();

    printf("[SMCL] Unloaded!\n");
    SetConsoleTitleW(L"Debug Console");

    fclose(fstdin);
    fclose(fstdout);
    fclose(fstderr);

    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )  
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)main, hModule, 0, NULL);

        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

