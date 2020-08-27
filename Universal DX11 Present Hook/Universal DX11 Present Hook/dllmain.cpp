/*
* dllmain.cpp Defines the entry point for this binary 
*/

/* Includes for this file */
#include <Windows.h>
#include <iostream>

#include "DirectX 11 Hook/DXHook.hpp"

/*
* Test function to add to the tasks list 
*/
void Menu()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("ImGui Window");
	ImGui::Text("Hello World");
	ImGui::End();

	ImGui::Render();

	dx_hook->p_context->OMSetRenderTargets(1, &dx_hook->main_render_target_view, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

/* 
* Main thread for this binary 
*/
inline void Main(HMODULE hMod)
{
	/* Allocate console */
	{
		/* Allocate a console window to the target process */
		AllocConsole();

		/* Set output to our allocated console window */
		freopen("CONOUT$", "w", stdout);

		/* Set input to our allocated console window */
		freopen("CONIN$", "r", stdin);
	}

	/* Initialize the hook */
	dx_hook->Init({Menu});

	/* Busy wait until this binary has exited */
	while (true)
	{
		/* Exit once trigger point is pressed */
		if (GetAsyncKeyState(VK_DELETE) & 0x8000) break;

		/* Prevent busy waiting */
		SleepEx(1, false);
	}

	FreeLibraryAndExitThread(hMod, 0);
	FreeConsole();

	return;
}

/* 
* Entry point for this binary, will bootstrap the main thread 
*/
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)Main, hModule, 0, nullptr));
	}
	else
	{
		/* DLL_PROCESS_DETACH handler here */
	}

	return TRUE;
}
