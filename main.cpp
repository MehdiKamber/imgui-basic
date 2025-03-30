// Dear ImGui: standalone example application for DirectX 11

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include <iostream>
#include <string>

// Data
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static bool                     g_SwapChainOccluded = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Main code
int main(int, char**)
{
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Example", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);

    // Our state
    ImGuiStyle* style = &ImGui::GetStyle();

    style->WindowPadding = ImVec2(15, 15);
    style->WindowRounding = 5.0f;
    style->FramePadding = ImVec2(5, 5);
    style->FrameRounding = 4.0f;
    style->ItemSpacing = ImVec2(12, 8);
    style->ItemInnerSpacing = ImVec2(8, 6);
    style->IndentSpacing = 25.0f;
    style->ScrollbarSize = 15.0f;
    style->ScrollbarRounding = 9.0f;
    style->GrabMinSize = 5.0f;
    style->GrabRounding = 3.0f;

    style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
    style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    style->Colors[ImGuiCol_Border] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
    style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);

    bool show_demo_window = true;
    bool show_another_window = false;
    bool show_login_window = true;
    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 0.00f);
    bool testbool = false;
    int inttest = 10;
    float floattest = 5.5f;

    char inputlicense[128] = "";
    char testInput[128] = "";
    std::string licencekey = "1234";

    // Main loop
    bool done = false;
    while (!done)
    {

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

        if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            ::Sleep(10);
            continue;
        }
        g_SwapChainOccluded = false;

        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }


        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (show_login_window) // Step 2
        {
            ImGui::SetNextWindowSize(ImVec2(400, 200));
            if (ImGui::Begin("@mehdikamb", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
            {

                ImVec2 windowSize = ImGui::GetWindowSize();

                float inputWidth = 260.0f;
                float inputHeight = 75.0f;
                float buttonWidth = 100.0f;
                float buttonHeight = 25.0f;

                float totalHeight = inputHeight + buttonHeight + 10.0f;
                float inputStartX = (windowSize.x - inputWidth) * 0.5f;
                float buttonStartX = (windowSize.x - buttonWidth) * 0.5f;

                float startY = (windowSize.y - totalHeight) * 0.5f;

                ImGui::SetCursorPos(ImVec2(inputStartX, startY + buttonHeight + 10.0f));
                ImGui::InputText("##passwordinput", inputlicense, CHAR_MAX, ImGuiInputTextFlags_Password);

                ImGui::SetCursorPos(ImVec2(buttonStartX, startY + inputHeight + 0.0f));
                if (ImGui::Button("Login", ImVec2(buttonWidth, buttonHeight)))
                {
                    if (inputlicense == licencekey)
                    {
                        show_another_window = true;
                        show_login_window = false;
                    }
                }
            }
            ImGui::End();
        }




        if (show_another_window)
        {
            ImGui::SetNextWindowSize(ImVec2(500, 300));
            if (ImGui::Begin("@mehdikamb", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar))
            {
                // TABS
                ImGui::PushStyleColor(ImGuiCol_Tab, ImVec4(18 / 255.0f, 18 / 255.0f, 23 / 255.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_TabHovered, ImVec4(24 / 255.0f, 24 / 255.0f, 28 / 255.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_TabActive, ImVec4(24 / 255.0f, 24 / 255.0f, 28 / 255.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1)); 

                if (ImGui::BeginTabBar("MyTabs"))
                {
                    if (ImGui::BeginTabItem("Aimbot"))
                    {

                        //GROUPBOX 1
                        ImVec2 groupSize1 = ImVec2(235, 222);
                        ImVec2 groupPos1 = ImVec2(10, 70);
                        ImGui::SetCursorPos(groupPos1);
                        ImGui::BeginGroup();
                        ImVec2 cursorScreenPos1 = ImGui::GetCursorScreenPos();
                        float rounding1 = 10.0f;
                        ImGui::GetWindowDrawList()->AddRectFilled(cursorScreenPos1, ImVec2(cursorScreenPos1.x + groupSize1.x, cursorScreenPos1.y + groupSize1.y), IM_COL32(18, 18, 23, 255), rounding1);
                        ImGui::GetWindowDrawList()->AddRect(cursorScreenPos1, ImVec2(cursorScreenPos1.x + groupSize1.x, cursorScreenPos1.y + groupSize1.y), IM_COL32(18, 18, 23, 255), rounding1);
                        ImGui::Dummy(groupSize1);
                        ImGui::EndGroup();

                        // GROUPBOX 2
                        ImVec2 groupSize2 = ImVec2(235, 222);
                        ImVec2 groupPos2 = ImVec2(255, 70);
                        ImGui::SetCursorPos(groupPos2);
                        ImGui::BeginGroup();
                        ImVec2 cursorScreenPos2 = ImGui::GetCursorScreenPos();
                        float rounding2 = 10.0f;
                        ImGui::GetWindowDrawList()->AddRectFilled(cursorScreenPos2, ImVec2(cursorScreenPos2.x + groupSize2.x, cursorScreenPos2.y + groupSize2.y), IM_COL32(18, 18, 23, 255), rounding2);
                        ImGui::GetWindowDrawList()->AddRect(cursorScreenPos2, ImVec2(cursorScreenPos2.x + groupSize2.x, cursorScreenPos2.y + groupSize2.y), IM_COL32(18, 18, 23, 255), rounding2);
                        ImGui::Dummy(groupSize2);
                        ImGui::EndGroup();

                        // BUTTON
                        ImVec2 ButtonPos = ImVec2(20, 80);
                        ImGui::SetCursorPos(ButtonPos);
                        ImGui::Button("Button");

                        //TOGGLE BUTTON
                        ImVec2 checkboxPos = ImVec2(20, 110);
                        ImGui::SetCursorPos(checkboxPos);
                        static bool aimbotEnabled = false;
                        ImGui::Checkbox("CheckBox Button", &aimbotEnabled);


                        // RADIO BUTTON
                        static bool selected = false;
                        ImVec2 radiobuttonPos = ImVec2(20, 140);
                        ImGui::SetCursorPos(radiobuttonPos);

                        if (ImGui::RadioButton("Radio Button", selected)) {
                            selected = !selected;
                        }

                        // INPUT TEXT
                        float textBoxWidth = 75.0f;
                        ImGui::PushItemWidth(textBoxWidth);
                        ImVec2 InputText1 = ImVec2(20, 170);
                        ImGui::SetCursorPos(InputText1);
                        ImGui::InputText("Input Text", testInput, IM_ARRAYSIZE(testInput));

                        // COMBO BOX
                        ImVec2 ComboBox1 = ImVec2(20, 200);
                        ImGui::SetCursorPos(ComboBox1);
                        static const char* items[]{ "Test 1","Test 2","Test 3","Test 4","Test 5" };
                        static int Selecteditem = 0;
                        bool check = ImGui::Combo("Combo Box", &Selecteditem, items, IM_ARRAYSIZE(items));
                        if (check)
                        {
                        }

                        // COLOR PICKER
                        ImVec2 ColorPickerPos = ImVec2(20, 230);
                        ImGui::SetCursorPos(ColorPickerPos);
                        static ImVec4 color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
                        ImGui::ColorEdit4("Color Picker", (float*)&color, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoOptions);

                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Visual"))
                    {

                        // GROUPBOX 1
                        ImVec2 groupSize1 = ImVec2(235, 222);
                        ImVec2 groupPos1 = ImVec2(10, 70);
                        ImGui::SetCursorPos(groupPos1);
                        ImGui::BeginGroup();
                        ImVec2 cursorScreenPos1 = ImGui::GetCursorScreenPos();
                        float rounding1 = 10.0f;
                        ImGui::GetWindowDrawList()->AddRectFilled(cursorScreenPos1, ImVec2(cursorScreenPos1.x + groupSize1.x, cursorScreenPos1.y + groupSize1.y), IM_COL32(18, 18, 23, 255), rounding1);
                        ImGui::GetWindowDrawList()->AddRect(cursorScreenPos1, ImVec2(cursorScreenPos1.x + groupSize1.x, cursorScreenPos1.y + groupSize1.y), IM_COL32(18, 18, 23, 255), rounding1);
                        ImGui::Dummy(groupSize1);
                        ImGui::EndGroup();

                        // GROUPBOX 2
                        ImVec2 groupSize2 = ImVec2(235, 222);
                        ImVec2 groupPos2 = ImVec2(255, 70);
                        ImGui::SetCursorPos(groupPos2);
                        ImGui::BeginGroup();
                        ImVec2 cursorScreenPos2 = ImGui::GetCursorScreenPos();
                        float rounding2 = 10.0f;
                        ImGui::GetWindowDrawList()->AddRectFilled(cursorScreenPos2, ImVec2(cursorScreenPos2.x + groupSize2.x, cursorScreenPos2.y + groupSize2.y), IM_COL32(18, 18, 23, 255), rounding2);
                        ImGui::GetWindowDrawList()->AddRect(cursorScreenPos2, ImVec2(cursorScreenPos2.x + groupSize2.x, cursorScreenPos2.y + groupSize2.y), IM_COL32(18, 18, 23, 255), rounding2);
                        ImGui::Dummy(groupSize2);
                        ImGui::EndGroup();

                        ImVec2 TextPos1 = ImVec2(20, 80);
                        ImGui::SetCursorPos(TextPos1);
                        ImGui::Text("Visual");

                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Player"))
                    {
                        
                        ImVec2 groupSize1 = ImVec2(235, 222);
                        ImVec2 groupPos1 = ImVec2(10, 70);
                        ImGui::SetCursorPos(groupPos1);
                        ImGui::BeginGroup();
                        ImVec2 cursorScreenPos1 = ImGui::GetCursorScreenPos();
                        float rounding1 = 10.0f;
                        ImGui::GetWindowDrawList()->AddRectFilled(cursorScreenPos1, ImVec2(cursorScreenPos1.x + groupSize1.x, cursorScreenPos1.y + groupSize1.y), IM_COL32(18, 18, 23, 255), rounding1);
                        ImGui::GetWindowDrawList()->AddRect(cursorScreenPos1, ImVec2(cursorScreenPos1.x + groupSize1.x, cursorScreenPos1.y + groupSize1.y), IM_COL32(18, 18, 23, 255), rounding1);
                        ImGui::Dummy(groupSize1);
                        ImGui::EndGroup();

                        // GROUPBOX 2
                        ImVec2 groupSize2 = ImVec2(235, 222);
                        ImVec2 groupPos2 = ImVec2(255, 70);
                        ImGui::SetCursorPos(groupPos2);
                        ImGui::BeginGroup();
                        ImVec2 cursorScreenPos2 = ImGui::GetCursorScreenPos();
                        float rounding2 = 10.0f;
                        ImGui::GetWindowDrawList()->AddRectFilled(cursorScreenPos2, ImVec2(cursorScreenPos2.x + groupSize2.x, cursorScreenPos2.y + groupSize2.y), IM_COL32(18, 18, 23, 255), rounding2);
                        ImGui::GetWindowDrawList()->AddRect(cursorScreenPos2, ImVec2(cursorScreenPos2.x + groupSize2.x, cursorScreenPos2.y + groupSize2.y), IM_COL32(18, 18, 23, 255), rounding2);
                        ImGui::Dummy(groupSize2);
                        ImGui::EndGroup();

                        ImVec2 TextPos1 = ImVec2(20, 80);
                        ImGui::SetCursorPos(TextPos1);
                        ImGui::Text("Player");
                        

                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Config"))
                    {
                        
                        ImVec2 groupSize1 = ImVec2(235, 222);
                        ImVec2 groupPos1 = ImVec2(10, 70);
                        ImGui::SetCursorPos(groupPos1);
                        ImGui::BeginGroup();
                        ImVec2 cursorScreenPos1 = ImGui::GetCursorScreenPos();
                        float rounding1 = 10.0f;
                        ImGui::GetWindowDrawList()->AddRectFilled(cursorScreenPos1, ImVec2(cursorScreenPos1.x + groupSize1.x, cursorScreenPos1.y + groupSize1.y), IM_COL32(18, 18, 23, 255), rounding1);
                        ImGui::GetWindowDrawList()->AddRect(cursorScreenPos1, ImVec2(cursorScreenPos1.x + groupSize1.x, cursorScreenPos1.y + groupSize1.y), IM_COL32(18, 18, 23, 255), rounding1);
                        ImGui::Dummy(groupSize1);
                        ImGui::EndGroup();

                        // GROUPBOX 2
                        ImVec2 groupSize2 = ImVec2(235, 222);
                        ImVec2 groupPos2 = ImVec2(255, 70);
                        ImGui::SetCursorPos(groupPos2);
                        ImGui::BeginGroup();
                        ImVec2 cursorScreenPos2 = ImGui::GetCursorScreenPos();
                        float rounding2 = 10.0f;
                        ImGui::GetWindowDrawList()->AddRectFilled(cursorScreenPos2, ImVec2(cursorScreenPos2.x + groupSize2.x, cursorScreenPos2.y + groupSize2.y), IM_COL32(18, 18, 23, 255), rounding2);
                        ImGui::GetWindowDrawList()->AddRect(cursorScreenPos2, ImVec2(cursorScreenPos2.x + groupSize2.x, cursorScreenPos2.y + groupSize2.y), IM_COL32(18, 18, 23, 255), rounding2);
                        ImGui::Dummy(groupSize2);
                        ImGui::EndGroup();

                        ImVec2 TextPos1 = ImVec2(20, 80);
                        ImGui::SetCursorPos(TextPos1);
                        ImGui::Text("Config");
                        

                        ImGui::EndTabItem();
                    }

                    ImGui::EndTabBar();
                }

                ImGui::PopStyleColor(4);

            }
            ImGui::End();
        }




        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Present
        HRESULT hr = g_pSwapChain->Present(1, 0);   // Present with vsync
        //HRESULT hr = g_pSwapChain->Present(0, 0); // Present without vsync
        g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions

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

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
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
