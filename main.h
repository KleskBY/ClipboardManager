#pragma once
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"
#include <d3d9.h>
#include <dinput.h>
#include <tchar.h>
#include <chrono>
#include <thread>
#include <string>
#include <vector>

static const char* AppClass = "APP CLASS";
static const char* AppName =  "Clipboard Manager";
static HWND hwnd = NULL;
static LPDIRECT3D9              g_pD3D = NULL;
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};
static ImFont* DefaultFont = nullptr;
static int WindowWidth = 300;
static int WindowHeight = 334;
static bool ShowMenu = true;

IDirect3DTexture9* tTrash = nullptr;
IDirect3DTexture9* tCopy = nullptr;
IDirect3DTexture9* tLock = nullptr;

struct ClipboardStruct
{
    char Buffer[8192];
    std::string path;
    int number;
    bool Encrypted;
};
std::vector<ClipboardStruct>ClipboardBuffers;

bool LoadBuffer(char* buffer, const char* path)
{
    FILE* f = fopen(path, "rb");
    if (f)
    {
        fseek(f, 0, SEEK_END);
        long fsize = ftell(f);
        fseek(f, 0, SEEK_SET);
        char* string = (char*)malloc(fsize + 1);
        fread(string, fsize, 1, f);
        fclose(f);
        string[fsize] = 0;
        for (int i = 0; i < fsize; i++) buffer[i] = string[i];
        buffer[fsize] = '\0';
        return true;
    }
    return false;
}

static void SaveBuffers()
{
    if (ClipboardBuffers.size() > 0)
    {
        for (int i = 0; i < ClipboardBuffers.size(); i++)
        {
            FILE* file = fopen(ClipboardBuffers[i].path.c_str(), "wb");
            if (file)
            {
                fwrite(ClipboardBuffers[i].Buffer, 1, strlen(ClipboardBuffers[i].Buffer), file);
                fclose(file);
            }
        }
    }
}

HRESULT CreateDeviceD3D(HWND hWnd)
{
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL) return E_FAIL;

    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    //g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0) return E_FAIL;

    return S_OK;
}

void CleanupDeviceD3D()
{
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = NULL; }
}

void ResetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL) IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            g_d3dpp.BackBufferWidth = LOWORD(lParam);
            g_d3dpp.BackBufferHeight = HIWORD(lParam);
            ResetDevice();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) return 0;// Disable ALT application menu
        break;
    case WM_NCHITTEST:
    {
        ImVec2 Shit = ImGui::GetMousePos();
        if (Shit.y < 25 && Shit.x < WindowWidth - 25)
        {
            LRESULT hit = DefWindowProc(hWnd, msg, wParam, lParam);
            if (hit == HTCLIENT) hit = HTCAPTION;
            return hit;
        }
        else break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}


void GetMyStyle()
{
    ImGuiStyle& style = ImGui::GetStyle();
    {
        style.Alpha = 1.f;
        style.WindowPadding = ImVec2(5, 5);
        style.WindowRounding = 0.f;//10.0f;
        style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
        style.ChildRounding = 3.0f;
        style.FrameBorderSize = 1;
        style.FramePadding = ImVec2(4, 3);
        style.FrameRounding = 5;
        style.ItemSpacing = ImVec2(5, 5);
        style.ItemInnerSpacing = ImVec2(4, 4);
        style.TouchExtraPadding = ImVec2(4, 4);
        style.IndentSpacing = 21.0f;
        style.ColumnsMinSpacing = 6.0f;
        style.ScrollbarSize = 10.0f;
        style.ScrollbarRounding = 3.0f;
        style.GrabMinSize = 10.0f;
        style.GrabRounding = 3.0f;
        style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
        style.DisplayWindowPadding = ImVec2(22, 22);
        style.DisplaySafeAreaPadding = ImVec2(4, 4);
        style.AntiAliasedLines = true;

        // Setup style
        ImVec4* colors = ImGui::GetStyle().Colors;
        colors[ImGuiCol_Text] = ImColor(255, 255, 255, 255);						//Цвет текста
        colors[ImGuiCol_TextDisabled] = ImColor(53, 53, 53, 255);
        colors[ImGuiCol_WindowBg] = ImColor(25, 25, 25, 255);						// Задний фон окна
        colors[ImGuiCol_ChildBg] = ImColor(15, 15, 15, 255);						// Задний фон "ребенка" (Child)
        colors[ImGuiCol_PopupBg] = ImColor(25, 25, 25, 255);
        colors[ImGuiCol_Border] = ImColor(36, 35, 35, 255);							//Обводка прямоуголника приложения и айтемом
        colors[ImGuiCol_BorderShadow] = ImColor(0, 0, 0, 200);						//Тень от обводки
        colors[ImGuiCol_FrameBg] = ImColor(20, 20, 20, 200);						// Задний фон слайдеров, кнопок и прочей херни
        colors[ImGuiCol_FrameBgHovered] = ImColor(140, 0, 0, 255);					// Когда наведен курсор
        colors[ImGuiCol_FrameBgActive] = ImColor(255, 0, 0, 175);					//Когда нажато
        colors[ImGuiCol_TitleBg] = ImColor(15, 15, 15, 255);						//Верхняя полоска с название программы когда окно неактивно
        colors[ImGuiCol_TitleBgActive] = ImColor(20, 20, 20, 255);					//Когда активно
        colors[ImGuiCol_TitleBgCollapsed] = ImColor(0, 0, 0, 255);
        colors[ImGuiCol_MenuBarBg] = ImColor(25, 25, 25, 255);
        colors[ImGuiCol_ScrollbarBg] = ImColor(20, 20, 20, 255);					//Ползунок(ScrollBar) задний фон
        colors[ImGuiCol_ScrollbarGrab] = ImColor(242, 0, 0, 232);					//Ползунок(ScrollBar) при нажатии??
        colors[ImGuiCol_ScrollbarGrabHovered] = ImColor(241, 0, 0, 174);			//Ползунок(ScrollBar) при наведении	
        colors[ImGuiCol_ScrollbarGrabActive] = ImColor(255, 0, 0, 133);				//Ползунок(ScrollBar) при перетягивании	
        colors[ImGuiCol_CheckMark] = ImColor(255, 0, 0, 253);						//Галочка (CheckBox)	//ImVec4(0.8f, 0.15f, 0.15f, 1.00f);  //CheckBox 
        colors[ImGuiCol_SliderGrab] = ImColor(246, 0, 0, 255);						// Слайдер когда нажимаем?
        colors[ImGuiCol_SliderGrabActive] = ImColor(255, 0, 0, 175);				// Слайдер когда перетягиваем
        colors[ImGuiCol_Button] = ImColor(20, 20, 20, 249);							// Кнопка 
        colors[ImGuiCol_ButtonHovered] = ImColor(140, 0, 0, 223);					// Кнопка при наведении
        colors[ImGuiCol_ButtonActive] = ImColor(0, 0, 0, 168);				        // Кнопка при нажамтии
        colors[ImGuiCol_Header] = ImColor(255, 0, 0, 175);							// Выбранный айтем из комбобокса
        colors[ImGuiCol_HeaderHovered] = ImColor(244, 0, 0, 223);
        colors[ImGuiCol_HeaderActive] = ImColor(255, 0, 0, 255);
        colors[ImGuiCol_Separator] = ImColor(0, 0, 0, 78);
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.81f, 0.81f, 0.81f, 1.00f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.75f, 0.10f, 0.10f, 1.00f);
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.98f, 0.26f, 0.26f, 1.00f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.98f, 0.26f, 0.26f, 1.00f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.87f, 0.87f, 0.87f, 1.00f);
        colors[ImGuiCol_Tab] = ImVec4(0.01f, 0.01f, 0.01f, 1.00f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
        colors[ImGuiCol_TabActive] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
        colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.68f, 0.68f, 0.68f, 1.00f);
        colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.77f, 0.33f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.87f, 0.55f, 0.08f, 1.00f);
        colors[ImGuiCol_TextSelectedBg] = ImVec4(0.47f, 0.60f, 0.76f, 1.00f);
        colors[ImGuiCol_DragDropTarget] = ImVec4(0.58f, 0.58f, 0.58f, 1.00f);
        colors[ImGuiCol_NavHighlight] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
    }
}
