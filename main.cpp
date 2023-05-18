#include "main.h"

/*void RenderBlur(HWND hwnd)
{
    struct ACCENTPOLICY
    {
        int na;
        int nf;
        int nc;
        int nA;
    };
    struct WINCOMPATTRDATA
    {
        int na;
        PVOID pd;
        ULONG ul;
    };

    const HINSTANCE hm = LoadLibrary("user32.dll");
    if (hm)
    {
        typedef BOOL(WINAPI* pSetWindowCompositionAttribute)(HWND, WINCOMPATTRDATA*);

        const pSetWindowCompositionAttribute SetWindowCompositionAttribute = (pSetWindowCompositionAttribute)GetProcAddress(hm, "SetWindowCompositionAttribute");
        if (SetWindowCompositionAttribute)
        {
            ACCENTPOLICY policy = { 4, 0, 155, 0 };
            WINCOMPATTRDATA data = { 19, &policy,sizeof(ACCENTPOLICY) };
            SetWindowCompositionAttribute(hwnd, &data);
        }
        FreeLibrary(hm);
    }
}*/

// Main code
int main(int argc, char* argv[])
{
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    /*HINSTANCE hInstance = GetModuleHandle(NULL);
    HWND loadingHwnd = CreateWindowEx(0, "STATIC", "Loading...", WS_POPUP | WS_VISIBLE, (GetSystemMetrics(SM_CXSCREEN) - 200) / 2, (GetSystemMetrics(SM_CYSCREEN) - 100) / 2, 200, 100, NULL, NULL, hInstance, NULL);
    RenderBlur(loadingHwnd);
    Sleep(3000);
    DestroyWindow(loadingHwnd);*/
    FreeConsole();
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"TextEditor", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"TextEditor", WS_OVERLAPPEDWINDOW, 200, 200, GetSystemMetrics(SM_CXSCREEN)-400, GetSystemMetrics(SM_CYSCREEN)-400, nullptr, nullptr, wc.hInstance, nullptr);
    //BOOL darkmode = TRUE;
    //::DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkmode, sizeof(darkmode));
    /*MARGINS margins{};f
    margins = { 0, 0, 31, 0 };
    auto hr = DwmExtendFrameIntoClientArea(hwnd, &margins);*/

    HMENU hMenubar = CreateMenu();
    HMENU hFile = CreateMenu();
    HMENU hEdit = CreateMenu();
    HMENU hSetting = CreateMenu();
    AppendMenuW(hFile, MF_STRING, ID_SAVE, L"Save\tCtrl+S");
    AppendMenuW(hFile, MF_STRING, ID_QUIT, L"Quit\tAlt+F4");

    AppendMenuW(hEdit, MF_STRING, ID_UNDO, L"Undo\tCtrl+Z");
    AppendMenuW(hEdit, MF_STRING, ID_REDO, L"Redo\tCtrl+Y");
    AppendMenuW(hEdit, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hEdit, MF_STRING, ID_CUT, L"Cut\tCtrl+X");
    AppendMenuW(hEdit, MF_STRING, ID_COPY, L"Copy\tCtrl+C");
    AppendMenuW(hEdit, MF_STRING, ID_PASTE, L"Paste\tCtrl+V");
    AppendMenuW(hEdit, MF_STRING, ID_DELETE, L"Delete\tDel");
    AppendMenuW(hEdit, MF_STRING, ID_SELECT_ALL, L"Select all\tCtrl+A");

    AppendMenuW(hSetting, MF_STRING | MF_UNCHECKED, ID_SHOW_SPACES, L"Show spaces");

    AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hFile, L"File");
    AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hEdit, L"Edit");
    AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hSetting, L"Setting");
    SetMenu(hwnd, hMenubar);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_NORMAL); //SW_MAXIMIZE
    ::UpdateWindow(hwnd);

    std::string filename;
    if (argc >= 2)
    {
        filename = argv[1];

        rootNode = CreateDirectryNodeTreeFromPath(filename);
        ToEdit(filename.c_str());
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.IniFilename = NULL;

    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
    static const ImWchar icons_ranges_brands[] = { ICON_MIN_FAB, ICON_MAX_16_FAB, 0 };
    ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;

    ImFont* main = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arlrdbd.ttf", 15);
    FontAwesome = io.Fonts->AddFontFromMemoryCompressedTTF(fa6_solid_compressed_data, fa6_solid_compressed_size, 15.f, &icons_config, icons_ranges);
    FontAwesomeBrands = io.Fonts->AddFontFromMemoryCompressedTTF(fa_brands_400_compressed_data, fa_brands_400_compressed_size, 15.f, &icons_config, icons_ranges_brands);
    ImFont* large = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arlrdbd.ttf", 21);
    ImFont* cascadia = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\CascadiaMono.ttf", 15);
    
    editor.SetLanguageDefinition(TextEditor::LanguageDefinition::TXT());

    // Our state
    ImVec4 clear_color = ImVec4(255 / 255.f, 255 / 255.f, 255 / 255.f, 255 / 255.f);
    DragAcceptFiles(hwnd, TRUE);
    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2 work_pos = viewport->WorkPos;
        ImVec2 work_size = viewport->WorkSize;

        ImGui::Begin("TextEditor", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
        {
            ImGui::SetWindowPos(ImVec2(0,0), ImGuiCond_Once);
            ImGui::SetWindowSize(ImVec2(float(work_pos.x + work_size.x), float(work_pos.y + work_size.y)), ImGuiCond_Always);

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            if (ImGui::BeginTable("Layout", 2, ImGuiTableFlags_Resizable))
            {
                ImGui::TableSetupColumn("Explorer", ImGuiTableColumnFlags_WidthFixed, 200.0f);
                ImGui::TableSetupColumn("Editor");
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::BeginChild("ExplorerChild", ImVec2(0, ImGui::GetWindowSize().y - 20), true);
                {
                    padding(6,3);
                    ImGui::PushFont(large);
                    ImGui::TextColored(ImGui::GetStyleColorVec4(ImGuiCol_Text), "Explorer");
                    ImGui::PopFont();
                    padding(0,3);
                    RecursivelyDisplayDirectoryNode(rootNode);
                }
                ImGui::EndChild();

                ImGui::TableSetColumnIndex(1);
                ImGui::PushFont(cascadia);
                editor.Render("TextEditor", ImVec2(0, ImGui::GetWindowSize().y - 20));
                ImGui::PopFont();

                ImGui::EndTable();
            }

            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(57 / 255.f, 57 / 255.f, 57 / 255.f, 255.f));
            ImGui::PushStyleColor(ImGuiCol_TableBorderLight, ImVec4(35 / 255.f, 35 / 255.f, 35 / 255.f, 255.f));
            ImGui::BeginChild("FooterWrapper", ImVec2(0, 20), true);
            {
                if (ImGui::BeginTable("Footer", 4, ImGuiTableFlags_Resizable, ImVec2(0,20)))
                {
                    auto cpos = editor.GetCursorPosition();
                    ImGui::TableSetupColumn("File");
                    ImGui::TableSetupColumn("CursorPos");
                    ImGui::TableSetupColumn("Lines");
                    ImGui::TableSetupColumn("Lang");
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); padding(5,1); ImGui::Text("File: %s", editor.fileToEdit);
                    ImGui::TableSetColumnIndex(1); padding(5,1); ImGui::Text("Ln: %d Col: %d", cpos.mLine + 1, cpos.mColumn + 1);
                    ImGui::TableSetColumnIndex(2); padding(5,1); ImGui::Text("Lines: %d", editor.GetTotalLines());
                    ImGui::TableSetColumnIndex(3); padding(5,1); ImGui::Text("Lang: %s", editor.GetLanguageDefinition().mName.c_str());

                    ImGui::EndTable();
                }
            }
            ImGui::EndChild();
            ImGui::PopStyleColor(2);
            ImGui::PopStyleVar();

            /*ImGui::Checkbox("Bg", &bg);
            ImGui::ColorEdit4("clear color", (float*)&clear_color);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);*/
        }
        ImGui::End();

        //ImGui::ShowDemoWindow();

        //clear_color = ImLerp(clear_color, ImVec4(31 / 255.f, 31 / 255.f, 31 / 255.f, bg ? 255 / 255.f : 155 / 255.f), io.DeltaTime * 10.0f);

        // Enable/disable menu bar buttons
        EnableMenuItem(GetMenu(hwnd), ID_SAVE, MF_BYCOMMAND | (editor.fileToEdit != "" ? MF_ENABLED : MF_DISABLED));
        EnableMenuItem(GetMenu(hwnd), ID_UNDO, MF_BYCOMMAND | (editor.CanUndo() ? MF_ENABLED : MF_DISABLED));
        EnableMenuItem(GetMenu(hwnd), ID_REDO, MF_BYCOMMAND | (editor.CanRedo() ? MF_ENABLED : MF_DISABLED));
        EnableMenuItem(GetMenu(hwnd), ID_COPY, MF_BYCOMMAND | (editor.HasSelection() ? MF_ENABLED : MF_DISABLED));
        EnableMenuItem(GetMenu(hwnd), ID_CUT, MF_BYCOMMAND | (editor.HasSelection() ? MF_ENABLED : MF_DISABLED));
        EnableMenuItem(GetMenu(hwnd), ID_DELETE, MF_BYCOMMAND | (editor.HasSelection() ? MF_ENABLED : MF_DISABLED));
        EnableMenuItem(GetMenu(hwnd), ID_PASTE, MF_BYCOMMAND | (ImGui::GetClipboardText() != nullptr ? MF_ENABLED : MF_DISABLED));

        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        //g_pSwapChain->Present(1, 0); // Present with vsync
        g_pSwapChain->Present(0, 0); // Present without vsync
    }
    DragAcceptFiles(hwnd, FALSE);
    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}



