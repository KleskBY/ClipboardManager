#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#pragma comment(lib, "d3dx9.lib")
#define DIRECTINPUT_VERSION 0x0800
#include "main.h"
#include "Toolkit.h"
#include "c://Program Files (x86)/Microsoft DirectX SDK (June 2010)/Include/d3dx9tex.h"
#include "img/trash.h"
#include "img/copy.h"
#include "img/lock.h"

#include <iostream>
#include <fstream>
#include "resource.h"

#pragma region SettignsLoader

static std::vector<std::string> SettingsList;
bool IsConfigSelected[50];
int SelectedConfig = 0;

void getFilesList(std::string filePath, std::string extension, std::vector<std::string>& returnFileName)
{
    WIN32_FIND_DATA fileInfo;
    HANDLE hFind;
    std::string  fullPath = filePath + extension;
    hFind = FindFirstFile(fullPath.c_str(), &fileInfo);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        returnFileName.push_back(filePath + fileInfo.cFileName);
        while (FindNextFile(hFind, &fileInfo) != 0) returnFileName.push_back(filePath + fileInfo.cFileName);
    }
}

void RefreshSettings()
{
    //memset(&SettingsList, 0, sizeof(SettingsList));
    SettingsList.clear();
    std::string configDir = std::string(getenv("appdata")) + std::string("\\ClipboardManager\\");

    std::vector<std::string> filesPaths;
    getFilesList(configDir, "*.bin*", filesPaths);
    std::vector<std::string>::const_iterator it = filesPaths.begin();
    while (it != filesPaths.end())
    {
        std::string outpath = it->c_str();
        SettingsList.push_back(outpath);
        //remove(it->c_str());
        it++;
    }
}

void UpdateClipboars()
{
    ClipboardBuffers.clear();
    RefreshSettings();
    if (SettingsList.size() > 0)
    {
        for (int i = 0; i < SettingsList.size(); i++)
        {
            ClipboardStruct newClipboard;
            LoadBuffer(newClipboard.Buffer, SettingsList[i].c_str());
            newClipboard.path = SettingsList[i];

            std::string configname = SettingsList[i].substr(0, SettingsList[i].size() - 4);
            std::string pathtocfg = configname.substr(0, configname.find_last_of("\\/"));
            configname = configname.erase(0, pathtocfg.size() + 1);
            newClipboard.number = atoi(configname.c_str());
            if (SettingsList[i].find("enc") != std::string::npos)
            {
                configname.erase(0, 3);
                newClipboard.number = atoi(configname.c_str());
                newClipboard.Encrypted = true;
            }
            else newClipboard.Encrypted = false;
            ClipboardBuffers.push_back(newClipboard);
        }
    }
}
#pragma endregion
int main(int, char* argv[])
{
    if (FindWindow(AppClass, NULL)) return EXIT_SUCCESS;

    std::string appdata = getenv("appdata");
    std::string mainfolder = appdata + "\\ClipboardManager";
    CreateDirectory(mainfolder.c_str(), 0);

    UpdateClipboars();

    char username[64];
    DWORD username_len = 64;
    GetUserName(username, &username_len);
    char EncryptKey = username[0];

    RECT desktop;
    GetWindowRect(GetDesktopWindow(), &desktop);
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, AppClass, NULL };
    RegisterClassEx(&wc);
    hwnd = CreateWindowEx(WS_EX_TOPMOST/* | WS_EX_LAYERED*/ | WS_EX_TOOLWINDOW, AppClass, AppName, WS_POPUP, (desktop.right / 2) - (WindowWidth / 2), (desktop.bottom / 2) - (WindowHeight / 2), WindowWidth, WindowHeight, 0, 0, wc.hInstance, 0);
    NOTIFYICONDATA nid;

    HICON hi = NULL;
    hi = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
    nid.hWnd = hwnd;
    nid.uID = 100;
    nid.uVersion = NOTIFYICON_VERSION;
    nid.uCallbackMessage = WM_MYMESSAGE;
    nid.hIcon = hi;
    strcpy(nid.szTip, "Clipboard manager");
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    Shell_NotifyIcon(NIM_ADD, &nid);
    //SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
    //SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, ULW_COLORKEY);

    if (CreateDeviceD3D(hwnd) < 0)
    {
        CleanupDeviceD3D();
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    RegisterHotKey(NULL, 1, MOD_CONTROL, VK_NUMPAD1);
    RegisterHotKey(NULL, 2, MOD_CONTROL, VK_NUMPAD2);
    RegisterHotKey(NULL, 3, MOD_CONTROL, VK_NUMPAD3);
    RegisterHotKey(NULL, 4, MOD_CONTROL, VK_NUMPAD4);
    RegisterHotKey(NULL, 5, MOD_CONTROL, VK_NUMPAD5);
    RegisterHotKey(NULL, 6, MOD_CONTROL, VK_NUMPAD6);
    RegisterHotKey(NULL, 7, MOD_CONTROL, VK_NUMPAD7);
    RegisterHotKey(NULL, 8, MOD_CONTROL, VK_NUMPAD8);
    RegisterHotKey(NULL, 9, MOD_CONTROL, VK_NUMPAD9);
    RegisterHotKey(NULL, 10, NULL, VK_HOME);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    DefaultFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 20.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
    GetMyStyle(); //ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    if (tTrash == nullptr) D3DXCreateTextureFromFileInMemoryEx(g_pd3dDevice, &trash, sizeof(trash), 32, 32, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &tTrash);
    if (tCopy == nullptr) D3DXCreateTextureFromFileInMemoryEx(g_pd3dDevice, &copyicon, sizeof(copyicon), 32, 32, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &tCopy);
    if (tLock == nullptr) D3DXCreateTextureFromFileInMemoryEx(g_pd3dDevice, &lock, sizeof(lock), 32, 32, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &tLock);


    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    static bool open = true;
    DWORD dwFlag = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    while (msg.message != WM_QUIT)
    {
        if (msg.message == WM_HOTKEY )
        {
            if (msg.wParam == 10)
            {
                ShowMenu = !ShowMenu;
                if (ShowMenu) ShowWindow(hwnd, SW_SHOWDEFAULT);
                else ShowWindow(hwnd, SW_HIDE);
            }
            else
            {
                if (ClipboardBuffers[msg.wParam - 1].Encrypted) for (int j = 0; j < strlen(ClipboardBuffers[msg.wParam - 1].Buffer); j++) ClipboardBuffers[msg.wParam - 1].Buffer[j] = ClipboardBuffers[msg.wParam - 1].Buffer[j] ^ EncryptKey;
                Clipboard::PasteText(std::string(ClipboardBuffers[msg.wParam - 1].Buffer));
                if (ClipboardBuffers[msg.wParam - 1].Encrypted) for (int j = 0; j < strlen(ClipboardBuffers[msg.wParam - 1].Buffer); j++) ClipboardBuffers[msg.wParam - 1].Buffer[j] = ClipboardBuffers[msg.wParam - 1].Buffer[j] ^ EncryptKey;
            }
            msg.message = NULL;
        }

        
        static float CPUvalues[60] = {};
        static DWORD CPUrefresh_time = 0.0;
        static float CPUphase = 0.0f;
        if (CPUrefresh_time == 0) CPUrefresh_time = GetTickCount();
        while (CPUrefresh_time < GetTickCount())
        {
            for (int i = 0; i < 60; i++)
            {
                if (i + 1 > 59)  CPUvalues[i] = CPUphase;
                else CPUvalues[i] = CPUvalues[i + 1];
            }
            CPUphase = Monitor::GetCPULoad() * 100.f;
            CPUrefresh_time = CPUrefresh_time + 1000;
        }

        static float values[60] = {};
        static int values_offset = 0;
        static DWORD refresh_time = 0;
        static float phase = 0.0f;
        if (refresh_time == 0.0) refresh_time = GetTickCount();
        while (refresh_time < GetTickCount())
        {
            for (int i = 0; i < 60; i++)
            {
                if (i + 1 > 59) values[i] = phase;
                else values[i] = values[i + 1];
            }
            phase = (float)(Monitor::GetRAM());
            refresh_time = refresh_time + 1000;
        }
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        if (!open) ExitProcess(EXIT_SUCCESS);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        if (ShowMenu)
        {
            ImGui_ImplDX9_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            ImGui::SetNextWindowSize(ImVec2(WindowWidth, WindowHeight), ImGuiCond_Once);
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
            //ImGui::ShowDemoWindow();
            ImGui::Begin(AppName, &open, dwFlag);
            {
                ImGui::BeginChild("##config", ImVec2(290,200));
                if (ClipboardBuffers.size() > 0)
                {
                    for (int i = 0; i < ClipboardBuffers.size(); i++)
                    {
                        if (ClipboardBuffers[i].Encrypted)
                        {
                            ImGui::PushItemWidth(186);
                            ImGui::InputText(("##" + std::to_string(ClipboardBuffers[i].number)).c_str(), ClipboardBuffers[i].Buffer, 8192, ImGuiInputTextFlags_Password);
                            ImGui::PopItemWidth();
                        }
                        else ImGui::InputTextMultiline(("##"+ std::to_string(ClipboardBuffers[i].number)).c_str(), ClipboardBuffers[i].Buffer, 8192, ImVec2(186,28));
                        
                        ImGui::SameLine(); 
                        ImGui::PushID(tTrash + i);
                        if (ImGui::ImageButton(tTrash, ImVec2(24, 24), ImVec2(0, 0), ImVec2(1, 1), 1))
                        {
                            std::remove(ClipboardBuffers[i].path.c_str()); 
                            UpdateClipboars();
                        }
                        ImGui::PopID();

                        ImGui::PushID(tLock + i);
                        ImGui::SameLine(); 
                        if (ImGui::ImageButton(tLock, ImVec2(24, 24), ImVec2(0, 0), ImVec2(1, 1), 1))
                        {
                            if (!ClipboardBuffers[i].Encrypted)
                            {
                                std::remove(ClipboardBuffers[i].path.c_str());
                                for (int j = 0; j < strlen(ClipboardBuffers[i].Buffer); j++) ClipboardBuffers[i].Buffer[j] = ClipboardBuffers[i].Buffer[j] ^ EncryptKey;
                                std::string::size_type pos = ClipboardBuffers[i].path.find_last_of(".");
                                ClipboardBuffers[i].path.insert(pos, "enc");
                                SaveBuffers();
                                UpdateClipboars();
                            }
                            else
                            {
                                std::remove(ClipboardBuffers[i].path.c_str());
                                for (int j = 0; j < strlen(ClipboardBuffers[i].Buffer); j++) ClipboardBuffers[i].Buffer[j] = ClipboardBuffers[i].Buffer[j] ^ EncryptKey;
                                ClipboardBuffers[i].path = mainfolder + "\\" + std::to_string(ClipboardBuffers[i].number) + ".bin";
                                SaveBuffers();
                                UpdateClipboars();
                            }
                        }
                        ImGui::PopID();

                        ImGui::PushID(tCopy + i);
                        ImGui::SameLine();
                        if (ImGui::ImageButton(tCopy, ImVec2(24, 24), ImVec2(0, 0), ImVec2(1, 1), 1))
                        {
                            if (ClipboardBuffers[i].Encrypted) for (int j = 0; j < strlen(ClipboardBuffers[i].Buffer); j++) ClipboardBuffers[i].Buffer[j] = ClipboardBuffers[i].Buffer[j] ^ EncryptKey;
                            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                            std::wstring wide = converter.from_bytes(std::string(ClipboardBuffers[i].Buffer));         //std::string narrow = converter.to_bytes(wide_utf16_source_string);
                            Clipboard::SetClipboardUnciodeText(wide.c_str());
                            if (ClipboardBuffers[i].Encrypted) for (int j = 0; j < strlen(ClipboardBuffers[i].Buffer); j++) ClipboardBuffers[i].Buffer[j] = ClipboardBuffers[i].Buffer[j] ^ EncryptKey;
                        }
                        ImGui::PopID();
                    }
                }
                ImGui::EndChild();
                if (ImGui::Button("ADD"))
                {
                    SaveBuffers();

                    int Number = 1;
                    if (ClipboardBuffers.size() > 0)
                    {
                        for (int i = 0; i < ClipboardBuffers.size(); i++)
                        {
                            if (ClipboardBuffers[i].number > Number) Number = ClipboardBuffers[i].number;
                        }
                        Number = Number + 1;
                    }
                    
                    ClipboardStruct newClipboard;
                    newClipboard.path = mainfolder +"\\"+std::to_string(Number)+".bin";
                    newClipboard.number = Number;
                    newClipboard.Buffer[0] = '\0';

                    FILE* file = fopen(newClipboard.path.c_str(), "wb");
                    if (file)
                    {
                        fwrite(newClipboard.Buffer, 1, 3, file);
                        fclose(file);
                    }
                    Sleep(100);
                    UpdateClipboars();
                    //ClipboardBuffers.push_back(newClipboard);
                }
                ImGui::SameLine();
                if (ImGui::Button("SAVE")) SaveBuffers();
                ImGui::SameLine();
                if (ImGui::Button("UPDATE")) UpdateClipboars();

                ImGui::SameLine();
                char CurrentPathBuffer[MAX_PATH];
                GetModuleFileName(NULL, CurrentPathBuffer, MAX_PATH);
                std::string ExePath = std::string(CurrentPathBuffer);
                std::string::size_type pos = ExePath.find_last_of("\\/") + 1;
                std::string ExeName = std::string(CurrentPathBuffer).substr(pos, strlen(CurrentPathBuffer));
                if (Regedit::RegRead(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", ExeName.c_str(), REG_SZ) != "ERROR")
                {
                    if (ImGui::Button("Disable autostart")) DisableAutostart();
                }
                else if (ImGui::Button("Enable autostart")) EnableAutostart();

                ImGui::PlotLines(("CPU\n" + std::to_string((int)CPUphase) + "%").c_str(), CPUvalues, IM_ARRAYSIZE(CPUvalues), 0, NULL, 0.f, 100.f, ImVec2(108, 60));
                ImGui::SameLine();
                ImGui::PlotLines(("RAM\n" + std::to_string((int)phase) + "%").c_str(), values, IM_ARRAYSIZE(values), 0, NULL, 0.f, 100.f, ImVec2(108, 60));
            }
            ImGui::End();

            ImGui::EndFrame();

            g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
            if (g_pd3dDevice->BeginScene() >= 0)
            {
                ImGui::Render();
                ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
                g_pd3dDevice->EndScene();
            }
            HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
            if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) ResetDevice();
        }
        else std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    NOTIFYICONDATA nid2;
    nid2.cbSize = sizeof(NOTIFYICONDATA);
    nid2.hWnd = hwnd;
    nid2.uID = 100;
    Shell_NotifyIcon(NIM_DELETE, &nid2);
    DestroyWindow(hwnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}
