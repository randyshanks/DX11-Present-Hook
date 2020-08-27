/*
* DXHook.cpp defines the implementation for the members of the the DXHook class 
*/

/* Includes for this file */
#include "DXHook.hpp"

/*
* Implementation for the Window Call back
*/
LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	/* If True ((Menu open)) and the ImGui window call back succeded return */
	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) return true;

	return CallWindowProc(dx_hook->o_wndproc, hWnd, uMsg, wParam, lParam);
}

/*
* Implementation for the hooked present function
*/
HRESULT __stdcall HKPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	/* Check if it has been initialized */
	if (!dx_hook->initialized)
	{
		/* Attepmt to get the swap chain device */
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&dx_hook->p_device)))
		{
			/* Get the device context */
			dx_hook->p_device->GetImmediateContext(&dx_hook->p_context);

			/* Initialize the swap chain description struct */
			DXGI_SWAP_CHAIN_DESC sd;

			/* Get this swapchains description */
			pSwapChain->GetDesc(&sd);

			/* Update the class varible with the swapchain window */
			dx_hook->window = sd.OutputWindow;

			/* Create a texture back buffer */
			ID3D11Texture2D* back_buffer;

			/* Get the swap chains back buffer */
			pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&back_buffer);

			/* Initialzie the render target view */
			dx_hook->p_device->CreateRenderTargetView(back_buffer, NULL, &dx_hook->main_render_target_view);

			/* Release the back buffer */
			back_buffer->Release();

			dx_hook->o_wndproc = (WNDPROC)SetWindowLongPtr(dx_hook->window, GWLP_WNDPROC, (LONG_PTR)WndProc);

			/* Initialzie imgui */
			dx_hook->InitImGui();

			/* Update the initialized var */
			dx_hook->initialized = true;
		}
		else
		{
			/* Call the orginal present function */
			return dx_hook->o_present(pSwapChain, SyncInterval, Flags);
		}
	}

	/* Walk the tasks list and execute each function */
	for (auto& task : dx_hook->tasks)
	{
		/* Call the function */
		task();
	}

	/* Call the orginal present function */
	return dx_hook->o_present(pSwapChain, SyncInterval, Flags);
}

/*
* Implementation for the actual initialization function 
* The tasks vector is for the functions that you want to be called inside the Present hook
* Eg UI, Cheat-Visuals or other shit 
*/
bool DXHook::Init(std::vector<std::function<void()>> tasks)
{
	/* Log the initiation */
	printf("[i] Attempting to hook present\n");

	/* Initialize D3D11 using kiero */
	if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
	{
		/* Log the success */
		printf("[i] Kiero initiation was successful\n");

		/* Bind the HKPresent with kiero */
		kiero::bind(8, (void**)&this->o_present, HKPresent);

		/* Store the task list */
		this->tasks = tasks;

		return true;
	}
	else
	{
		/* Log the failure */
		printf("[!] Something went wrong\n");
	}

	return false;
}

/*
* Implementation for DXHook->ImGuiInit()
* Function initiates ImGui with the Device and context of the present hook
*/
void DXHook::InitImGui()
{
	/* Create the ImGui Context */
	ImGui::CreateContext();

	/* Edit the ImGui Config Flags to prevent the mouse cursor changing */
	ImGui::GetIO().ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;

	/* Setup interface and renderer bindings */
	ImGui_ImplWin32_Init(this->window);
	ImGui_ImplDX11_Init(this->p_device, this->p_context);
}


DXHook* dx_hook = new DXHook;