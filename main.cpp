#include "main.h"

// Main code
int main(int argument_count, char* argument_list[])
{
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"TextEditor", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"TextEditor", WS_OVERLAPPEDWINDOW, 200, 200, GetSystemMetrics(SM_CXSCREEN)-400, GetSystemMetrics(SM_CYSCREEN)-400, nullptr, nullptr, wc.hInstance, nullptr);
    //BOOL darkmode = TRUE;
    //::DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkmode, sizeof(darkmode));
    /*MARGINS margins{};
    margins = { 0, 0, 31, 0 };
    auto hr = DwmExtendFrameIntoClientArea(hwnd, &margins);*/

    HMENU hMenubar = CreateMenu();
    HMENU hFile = CreateMenu();
    HMENU hEdit = CreateMenu();
    HMENU hSetting = CreateMenu();
    AppendMenuW(hFile, MF_STRING, ID_SAVE, L"Save\tCtrl+S");
    AppendMenuW(hFile, MF_STRING, ID_QUIT, L"Quit\tAlt+F4");

    AppendMenuW(hEdit, MF_STRING | MF_UNCHECKED, ID_READ_ONLY, L"Read only");
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

    RegisterHotKey(hwnd, ID_SAVE, MOD_CONTROL | MOD_NOREPEAT, 'S');
    RegisterHotKey(hwnd, ID_UNDO, MOD_CONTROL | MOD_NOREPEAT, 'Z');
    RegisterHotKey(hwnd, ID_REDO, MOD_CONTROL | MOD_NOREPEAT, 'Y');
    RegisterHotKey(hwnd, ID_CUT, MOD_CONTROL | MOD_NOREPEAT, 'X');
    RegisterHotKey(hwnd, ID_COPY, MOD_CONTROL | MOD_NOREPEAT, 'C');
    RegisterHotKey(hwnd, ID_PASTE, MOD_CONTROL | MOD_NOREPEAT, 'V');
    RegisterHotKey(hwnd, ID_DELETE, MOD_NOREPEAT, VK_DELETE);
    RegisterHotKey(hwnd, ID_SELECT_ALL, MOD_CONTROL | MOD_NOREPEAT, 'A');

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

    //editor.SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());

    std::string filename;
    if (argument_count >= 2)
    {
        filename = argument_list[1];

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

    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\CascadiaMono.ttf", 14);
    //io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Verdana.ttf", 14);

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

            /*ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 4));
            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Save", "Ctrl-S"))
                    {
                        std::string textToSave = editor.GetText();
                        SaveText(textToSave);
                    }
                    if (ImGui::MenuItem("Quit", "Alt-F4"))
                        break;
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Edit"))
                {
                    bool ro = editor.IsReadOnly();
                    if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
                        editor.SetReadOnly(ro);
                    ImGui::Separator();

                    if (ImGui::MenuItem("Undo", "Ctrl-Z", nullptr, !ro && editor.CanUndo()))
                        editor.Undo();
                    if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && editor.CanRedo()))
                        editor.Redo();

                    ImGui::Separator();

                    if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor.HasSelection()))
                        editor.Copy();
                    if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && editor.HasSelection()))
                        editor.Cut();
                    if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && editor.HasSelection()))
                        editor.Delete();
                    if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
                        editor.Paste();

                    ImGui::Separator();

                    if (ImGui::MenuItem("Select all", nullptr, nullptr))
                        editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(editor.GetTotalLines(), 0));

                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Setting"))
                {
                    bool ws = editor.IsShowingWhitespaces();
                    if (ImGui::MenuItem("ShowWhiteSpaces", nullptr, &ws))
                        editor.SetShowWhitespaces(ws);
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }
            ImGui::PopStyleVar();*/

            if (ImGui::BeginTable("Layout", 2, ImGuiTableFlags_Resizable))
            {
                ImGui::TableSetupColumn("Explorer");
                ImGui::TableSetupColumn("Editor");
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::BeginChild("ExplorerChild", ImVec2(0,0), false);
                {
                    RecursivelyDisplayDirectoryNode(rootNode);
                }
                ImGui::EndChild();

                static unsigned short initial_column_spacing = 0;
                if (initial_column_spacing < 3)
                {
                    ImGui::TableSetColumnWidth(0, 200.0f);
                    initial_column_spacing++;
                }

                ImGui::TableSetColumnIndex(1);
                editor.Render("TextEditor");

                ImGui::EndTable();
            }

            /*ImGui::Checkbox("Bg", &bg);
            ImGui::ColorEdit4("clear color", (float*)&clear_color);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);*/
        }
        ImGui::End();

        ImGui::ShowDemoWindow();

        //clear_color = ImLerp(clear_color, ImVec4(31 / 255.f, 31 / 255.f, 31 / 255.f, bg ? 255 / 255.f : 155 / 255.f), io.DeltaTime * 10.0f);

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