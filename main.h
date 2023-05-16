#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#pragma comment(lib, "d3d11.lib")
#include <d3d11.h>
#include <tchar.h>
#include "imgui/imgui_internal.h"

#include <windowsx.h>
//#include <dwmapi.h>
//#include <shellapi.h>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "TextEditor/TextEditor.h"
#include "FontAwesome/IconsFontAwesome6.h"
#include "FontAwesome/IconsFontAwesome6Brands.h"

#define ID_SAVE 1
#define ID_QUIT 2
#define ID_UNDO 3
#define ID_REDO 4
#define ID_CUT 5
#define ID_COPY 6
#define ID_PASTE 7
#define ID_DELETE 8
#define ID_SELECT_ALL 9
#define ID_SHOW_SPACES 10

ImFont* FontAwesome;
ImFont* FontAwesomeBrands;

TextEditor editor;
const char* fileToEdit = "";

std::map <std::string, const char*> icons = {
    {"html", ICON_FA_HTML5},
    {"py", ICON_FA_PYTHON},
    {"js", ICON_FA_SQUARE_JS},
    {"css", ICON_FA_CSS3_ALT},
    {"php", ICON_FA_PHP},
    {"gitattributes", ICON_FA_GITHUB},
    {"gitignore", ICON_FA_GITHUB},
    {"cpp", ICON_FA_FILE_CODE},
    {"c", ICON_FA_FILE_CODE},
    {"hpp", ICON_FA_FILE_CODE},
    {"h", ICON_FA_FILE_CODE},
    {"png", ICON_FA_FILE_IMAGE},
    {"jpg", ICON_FA_FILE_IMAGE}
};

void ToEdit(const char* file)
{
    fileToEdit = file;
    std::ifstream t(file);
    if (t.good())
    {
        std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
        editor.SetText(str);
    }
}

void SaveText(std::string text)
{
    std::ofstream fw(fileToEdit, std::ofstream::out);

    if (fw.is_open())
    {
        fw << text;
        fw.close();
    }
}

struct DirectoryNode
{
    std::string FullPath;
    std::string FileName;
    std::vector<DirectoryNode> Children;
    bool IsDirectory;
    bool IsOpen = false;
};

void RecursivelyAddDirectoryNodes(DirectoryNode& parentNode, std::filesystem::directory_iterator directoryIterator)
{
    for (const std::filesystem::directory_entry& entry : directoryIterator)
    {
        DirectoryNode& childNode = parentNode.Children.emplace_back();
        childNode.FullPath = entry.path().u8string();
        childNode.FileName = entry.path().filename().u8string();
        if (childNode.IsDirectory = entry.is_directory(); childNode.IsDirectory)
            RecursivelyAddDirectoryNodes(childNode, std::filesystem::directory_iterator(entry));
    }

    auto moveDirectoriesToFront = [](const DirectoryNode& a, const DirectoryNode& b) { return (a.IsDirectory > b.IsDirectory); };
    std::sort(parentNode.Children.begin(), parentNode.Children.end(), moveDirectoriesToFront);
}

DirectoryNode CreateDirectryNodeTreeFromPath(const std::filesystem::path& rootPath)
{
    DirectoryNode rootNode;
    rootNode.FullPath = rootPath.u8string();
    rootNode.FileName = rootPath.filename().u8string();
    if (rootNode.IsDirectory = std::filesystem::is_directory(rootPath); rootNode.IsDirectory)
        RecursivelyAddDirectoryNodes(rootNode, std::filesystem::directory_iterator(rootPath));

    return rootNode;
}

std::string get_ext(const std::string& st) {
    size_t pos = st.rfind('.');
    if (pos <= 0) return "";
    return st.substr(pos + 1, std::string::npos);
}

void RecursivelyDisplayDirectoryNode(const DirectoryNode& parentNode)
{
    ImGui::PushID(&parentNode);
    if (parentNode.IsDirectory)
    {
        if (ImGui::TreeNodeEx((parentNode.FileName).c_str(), ((parentNode.IsOpen ? ICON_FA_FOLDER_OPEN : ICON_FA_FOLDER) + parentNode.FileName).c_str(), ImGuiTreeNodeFlags_SpanFullWidth))
        {
            const_cast<DirectoryNode&>(parentNode).IsOpen = true;

            for (const DirectoryNode& childNode : parentNode.Children)
                RecursivelyDisplayDirectoryNode(childNode);

            ImGui::TreePop();
        }
        else
            const_cast<DirectoryNode&>(parentNode).IsOpen = false;
    }
    else
    {
        std::string type = get_ext(parentNode.FileName);

        if (ImGui::TreeNodeEx((parentNode.FileName).c_str(), ((icons.find(type) != icons.end() ? icons[type] : ICON_FA_FILE) + parentNode.FileName + (parentNode.FullPath.c_str() == fileToEdit && editor.CanUndo() ? "*" : "")).c_str(), ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth))
        {
            if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered())
            {
                ToEdit(parentNode.FullPath.c_str());
            }
        }
    }
    ImGui::PopID();
}

DirectoryNode rootNode = CreateDirectryNodeTreeFromPath(L"C:/Users/admin/Desktop/TextEditor");

static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_COMMAND:
    case WM_HOTKEY: {
        UINT uState = GetMenuState(GetMenu(hWnd), wParam, MF_BYCOMMAND);
        switch (wParam) {
        case ID_SAVE: {
            std::string textToSave = editor.GetText();
            SaveText(textToSave);
            break;
        }
        case ID_QUIT: {
            ::PostQuitMessage(0);
            break;
        }
        case ID_UNDO: {
            editor.Undo();
            break;
        }
        case ID_REDO: {
            editor.Redo();
            break;
        }
        case ID_COPY: {
            editor.Copy();
            break;
        }
        case ID_CUT: {
            editor.Cut();
            break;
        }
        case ID_DELETE: {
            editor.Delete();
            break;
        }
        case ID_PASTE: {
            editor.Paste();
            break;
        }
        case ID_SELECT_ALL: {
            editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(editor.GetTotalLines(), 0));
            break;
        }
        case ID_SHOW_SPACES: {
            bool isChecked = (uState & MF_CHECKED) != 0;
            CheckMenuItem(GetMenu(hWnd), wParam, isChecked ? MF_UNCHECKED : MF_CHECKED);
            editor.SetShowWhitespaces(!isChecked);
            break;
        }
        }
        break;
    }
    case WM_GETMINMAXINFO: {
        LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
        lpMMI->ptMinTrackSize.x = 400;
        lpMMI->ptMinTrackSize.y = 300;
        return 0;
    }
    case WM_SIZE: {
        if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    }
    case WM_DROPFILES: {
        TCHAR szName[MAX_PATH];
        HDROP hDrop = (HDROP)wParam;
        int count = DragQueryFile(hDrop, 0xFFFFFFFF, szName, MAX_PATH);

        for (int i = 0; i < count; i++)
        {
            DragQueryFile(hDrop, i, szName, MAX_PATH);
            rootNode = CreateDirectryNodeTreeFromPath(szName);
        }

        DragFinish(hDrop);
        return 0;
    } break;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
