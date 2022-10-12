#include <chrono>
#include <GL/glew.h>
#include <GL/wglew.h>
#include <cxxopts.hpp>

#include <utils.hpp>
#include <window.hpp>
#include <camera.hpp>
#include <renderer.hpp>

BEGIN_VISUALIZER_NAMESPACE

static LRESULT CALLBACK WindowEvenHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    auto window = reinterpret_cast<visualizer::Window *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    if (!window)
    {
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    switch (uMsg)
    {
    case WM_CLOSE:
    {
        window->Close();
        break;
    }
    case WM_MOVE:
    {
        GetWindowRect(hWnd, &window->GetWindowRect());
        break;
    }
    case WM_SIZE:
    {
        WORD width = LOWORD(lParam), height = HIWORD(lParam);
        
        GetWindowRect(hWnd, &window->GetWindowRect());

        if (width > 0 && height > 0)
        {
            window->Resize(width, height);
        }
        else
        {
            const RECT& rect = window->GetWindowRect();
            SetWindowPos(hWnd, HWND_TOP, rect.left, rect.top, std::max(static_cast<uint16_t>(1), width), std::max(static_cast<uint16_t>(1), height), 0);
        }
        break;
    }
    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case VK_ESCAPE:
        {
            window->Close();
            break;
        }
        case VK_LEFT:
        {
            window->SetMustMoveCameraLeft(true);
            break;
        }
        case VK_UP:
        {
            window->SetMustMoveCameraForward(true);
            break;
        }
        case VK_RIGHT:
        {
            window->SetMustMoveCameraRight(true);
            break;
        }
        case VK_DOWN:
        {
            window->SetMustMoveCameraBackward(true);
            break;
        }
        }
        break;
    }
    case WM_KEYUP:
    {
        switch (wParam)
        {
        case VK_LEFT:
        {
            window->SetMustMoveCameraLeft(false);
            break;
        }
        case VK_UP:
        {
            window->SetMustMoveCameraForward(false);
            break;
        }
        case VK_RIGHT:
        {
            window->SetMustMoveCameraRight(false);
            break;
        }
        case VK_DOWN:
        {
            window->SetMustMoveCameraBackward(false);
            break;
        }
        }
        break;
    }
    case WM_LBUTTONDOWN:
    {
        ShowCursor(false);
        window->SetMouseButtonDown(true);
        const RECT& rect = window->GetWindowRect();
        const POINT centerPos{ static_cast<long>(rect.left + window->GetWidth() / 2), static_cast<long>(rect.top + window->GetHeight() / 2) };
        SetCursorPos(centerPos.x, centerPos.y);
        break;
    }
    case WM_LBUTTONUP:
    {
        ShowCursor(true);
        window->SetMouseButtonDown(false);
        break;
    }
    case WM_MOUSEMOVE:
    {
        if (window->GetMouseButtonDown())
        {
            const RECT& rect = window->GetWindowRect();
            const POINT centerPos{ static_cast<long>(rect.left + window->GetWidth() / 2), static_cast<long>(rect.top + window->GetHeight() / 2) };

            POINT mousePos;
            GetCursorPos(&mousePos);

            if (mousePos.x != centerPos.x || mousePos.y != centerPos.y)
            {
                window->SetCameraMovement(mousePos.x - centerPos.x, mousePos.y - centerPos.y);
                SetCursorPos(centerPos.x, centerPos.y);
            }
        }
        break;
    }
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool Window::InitWindow(const std::string_view& name, uint16_t width, uint16_t height, cxxopts::ParseResult& commandLineOptions)
{
    if (!m_IsInitialized)
    {
        m_CommandLineOptions = &commandLineOptions;

        m_Name = name;
        m_Width = width;
        m_Height = height;

        m_hInstance = GetModuleHandle(nullptr);

        if (!m_hInstance)
        {
            std::cerr << "Couldn't get module handle\n";
            DisplayLastWinAPIError();
            return false;
        }

        auto wr = RECT{ 0, 0, static_cast<LONG>(m_Width), static_cast<LONG>(m_Height) };
        auto style = WS_OVERLAPPEDWINDOW, exStyle = WS_EX_WINDOWEDGE;

        PIXELFORMATDESCRIPTOR pfd;
        memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
        pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cDepthBits = 24;
        pfd.cStencilBits = 0;
        pfd.iLayerType = PFD_MAIN_PLANE;

        {
            auto fakeWindowClass = WNDCLASSEX{};

            fakeWindowClass.cbSize = sizeof(WNDCLASSEX);
            fakeWindowClass.style = CS_HREDRAW | CS_VREDRAW;
            fakeWindowClass.lpfnWndProc = WindowEvenHandler;
            fakeWindowClass.hInstance = m_hInstance;
            fakeWindowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
            fakeWindowClass.lpszClassName = "fake";

            if (!RegisterClassEx(&fakeWindowClass))
            {
                std::cerr << "Couldn't register class: " << fakeWindowClass.lpszClassName << '\n';
                DisplayLastWinAPIError();
                return false;
            }

            AdjustWindowRectEx(&wr, style, FALSE, exStyle);

            HWND fakehWnd = CreateWindowEx(0, "fake", "fake", style, CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top, nullptr, nullptr, m_hInstance, nullptr);

            if (!fakehWnd)
            {
                std::cerr << "Couldn't create fake window\n";
                DisplayLastWinAPIError();
                return false;
            }

            HDC hDC = GetDC(fakehWnd);

            if (!hDC)
            {
                std::cerr << "Couldn't get fake display context\n";
                DisplayLastWinAPIError();
                return false;
            }

            int32_t pixelFormatID = ChoosePixelFormat(hDC, &pfd);
            if (pixelFormatID == 0)
            {
                std::cerr << "Couldn't choose fake pixel format\n";
                DisplayLastWinAPIError();
                return false;
            }

            if (!SetPixelFormat(hDC, pixelFormatID, &pfd))
            {
                std::cerr << "Couldn't set fake pixel format\n";
                DisplayLastWinAPIError();
                return false;
            }

            HGLRC fakeContext = wglCreateContext(hDC);

            if (!fakeContext)
            {
                std::cerr << "Couldn't create fake OpenGL context\n";
                DisplayLastWinAPIError();
                return false;
            }

            if (!wglMakeCurrent(hDC, fakeContext))
            {
                std::cerr << "Couldn't bind fake OpenGL context\n";
                DisplayLastWinAPIError();
                return false;
            }

            glewExperimental = GL_TRUE;

            const GLenum glewInitError = glewInit();

            if (glewInitError != GLEW_OK)
            {
                std::cerr << "Couldn't initialize glew library\n";
                return false;
            }

            if (!WGLEW_ARB_create_context)
            {
                std::cerr << "Missing extension WGL_ARB_create_context, did you install your device's driver?";
                return false;
            }

            if (!WGLEW_ARB_pixel_format)
            {
                std::cerr << "Missing extension WGL_ARB_pixel_format, did you install your device's driver?";
                return false;
            }

            {
                auto windowClass = WNDCLASSEX{};

                windowClass.cbSize = sizeof(WNDCLASSEX);
                windowClass.style = CS_HREDRAW | CS_VREDRAW;
                windowClass.lpfnWndProc = WindowEvenHandler;
                windowClass.hInstance = m_hInstance;
                windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
                windowClass.lpszClassName = m_Name.data();

                if (!RegisterClassEx(&windowClass))
                {
                    std::cerr << "Couldn't register class: " << windowClass.lpszClassName << '\n';
                    DisplayLastWinAPIError();
                    return false;
                }

                m_hWnd = CreateWindowEx(0, m_Name.data(), m_Name.data(), style, CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top, nullptr, nullptr, m_hInstance, nullptr);

                if (!m_hWnd)
                {
                    std::cerr << "Couldn't create window\n";
                    DisplayLastWinAPIError();
                    return false;
                }

                m_hDC = GetDC(m_hWnd);

                if (!m_hDC)
                {
                    std::cerr << "Couldn't get display context\n";
                    DisplayLastWinAPIError();
                    return false;
                }

                const int32_t pixelFormat[] = {
                    WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
                    WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
                    WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
                    WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
                    WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
                    WGL_COLOR_BITS_ARB, 32,
                    WGL_DEPTH_BITS_ARB, 24,
                    WGL_STENCIL_BITS_ARB, 0,
                    0
                };

                uint32_t numFormats;
                if (!wglChoosePixelFormatARB(m_hDC, pixelFormat, nullptr, 1, &pixelFormatID, &numFormats))
                {
                    std::cerr << "Couldn't choose pixel format\n";
                    DisplayLastWinAPIError();
                    return false;
                }

                if (!SetPixelFormat(m_hDC, pixelFormatID, &pfd))
                {
                    std::cerr << "Couldn't set pixel format\n";
                    DisplayLastWinAPIError();
                    return false;
                }

                {
                    const bool useDebugContext = (*m_CommandLineOptions)["debug"].as<bool>();

                    int32_t contextFlags = useDebugContext ? WGL_CONTEXT_DEBUG_BIT_ARB : GL_NONE;

                    struct Version
                    {
                        const uint8_t major;
                        const uint8_t minor;
                    };

                    // Looks for the latest supported OpenGL version
                    const std::array versions = { Version{4, 6}, Version{4, 5}, Version{4, 4}, Version{4, 3}, Version{4, 2}, Version{4, 1}, Version{4, 0}, Version{3, 3}, Version{3, 2}, Version{3, 1}, Version{3, 0} };

                    int32_t contextAttribs[] =
                    {
                        WGL_CONTEXT_MAJOR_VERSION_ARB, 0,
                        WGL_CONTEXT_MINOR_VERSION_ARB, 0,
                        // We don't want deprecated functionalities
                        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                        WGL_CONTEXT_FLAGS_ARB, contextFlags,
                        WGL_CONTEXT_OPENGL_NO_ERROR_ARB, useDebugContext ? GL_FALSE : GL_TRUE,
                        0
                    };

                    std::size_t index = 0;

                    while (!m_hrc && index < versions.size())
                    {
                        contextAttribs[1] = versions[index].major;
                        contextAttribs[3] = versions[index].minor;

                        m_hrc = wglCreateContextAttribsARB(m_hDC, 0, contextAttribs);
                        ++index;
                    }

                    if (!m_hrc)
                    {
                        std::cerr << "Couldn't create OpenGL context\n";
                        return false;
                    }
                }
            }

            if(!wglMakeCurrent(nullptr, nullptr))
            {
                std::cerr << "Couldn't unbind fake OpenGL context\n";
                DisplayLastWinAPIError();
                return false;
            }

            if(!wglDeleteContext(fakeContext))
            {
                std::cerr << "Couldn't delete fake OpenGL context\n";
                DisplayLastWinAPIError();
                return false;
            }

            if (!::DestroyWindow(fakehWnd))
            {
                std::cerr << "Couldn't destroy fake window\n";
                DisplayLastWinAPIError();
                return false;
            }

            if(!UnregisterClass("fake", m_hInstance))
            {
                std::cerr << "Couldn't unregister class: " << fakeWindowClass.lpszClassName << '\n';
                DisplayLastWinAPIError();
                return false;
            }
        }

        if(!wglMakeCurrent(m_hDC, m_hrc))
        {
            std::cerr << "Couldn't bind OpenGL context\n";
            DisplayLastWinAPIError();
            return false;
        }

        SetWindowLongPtr(m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

        GLint major;
        glGetIntegerv(GL_MAJOR_VERSION, &major);

        GLint minor;
        glGetIntegerv(GL_MINOR_VERSION, &minor);

        std::cout << "OpenGL version : " << major << '.' << minor << '\n';

        GLint contextProfileMask;
        glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &contextProfileMask);

        if (contextProfileMask & GL_CONTEXT_CORE_PROFILE_BIT)
        {
            std::cout << "Core profile context created\n";
        }

        if (contextProfileMask & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT)
        {
            std::cout << "Compatibility profile context created\n";
        }

        GLint contextFlags;
        glGetIntegerv(GL_CONTEXT_FLAGS, &contextFlags);

        if (contextFlags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT)
        {
            std::cout << "Forward compatible context created\n";
        }

        if (contextFlags & GL_CONTEXT_FLAG_DEBUG_BIT)
        {
            std::cout << "Debug context created\n";
        }

        if (contextFlags & GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT)
        {
            std::cout << "Robust access context created\n";
        }

        if (contextFlags & GL_CONTEXT_FLAG_NO_ERROR_BIT)
        {
            std::cout << "No error context created\n";
        }

        CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo;

        HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

        GetConsoleScreenBufferInfo(consoleHandle, &consoleScreenBufferInfo);

        if (GLEW_ARB_direct_state_access)
        {
            SetConsoleTextAttribute(consoleHandle, FOREGROUND_GREEN);
            std::cout << "Direct state access available\n";
            m_DirectStateAccessAvailable = true;
        }
        else
        {
            SetConsoleTextAttribute(consoleHandle, FOREGROUND_RED);
            std::cout << "Direct state access unavailable\n";
        }        

        if (GLEW_ARB_buffer_storage)
        {
            SetConsoleTextAttribute(consoleHandle, FOREGROUND_GREEN);
            std::cout << "Buffer storage available\n";
            m_BufferStorageAvailable = true;
        }
        else
        {
            SetConsoleTextAttribute(consoleHandle, FOREGROUND_RED);
            std::cout << "Buffer storage unavailable\n";
        }

        SetConsoleTextAttribute(consoleHandle, consoleScreenBufferInfo.wAttributes);

        // Put that to 0 to disable V-Sync
        wglSwapIntervalEXT(1);

        m_IsInitialized = true;
    }

    return true;
}

void Window::DestroyWindow()
{
    ::DestroyWindow(m_hWnd);
    UnregisterClass(m_Name.data(), m_hInstance);
}

Window::Window()
{}

Window::~Window()
{
    DestroyWindow();
}

bool Window::Update() const
{
    MSG msg;
    while (PeekMessage(&msg, m_hWnd, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return m_WindowShouldRun;
}

void Window::Run()
{
    if (!m_IsInitialized)
    {
        return;
    }

    ShowWindow(m_hWnd, SW_SHOW);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    m_Camera = std::make_shared<Camera>(m_Width, m_Height, glm::vec3(0., 0., 2.5f));

    m_Renderer = std::make_unique<Renderer>(m_Width, m_Height, m_Camera);

    if (!m_Renderer->Initialize())
    {
        std::cerr << "Renderer failed to initialize\n";
        return;
    }

    std::chrono::duration<float> dt;
    std::chrono::duration<float> totalElapsedTime;

    std::chrono::time_point<std::chrono::steady_clock> start, lastFrame;
    start = lastFrame = std::chrono::steady_clock::now();

    while (Update())
    {
        std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::steady_clock::now();
        dt = end - lastFrame;
        totalElapsedTime = end - start;
        lastFrame = end;
        HandleCameraMovement(dt.count());

        m_Renderer->Render();

        SwapBuffers(m_hDC);
    }

    m_Renderer->Cleanup();
}

void Window::Close()
{
    m_WindowShouldRun = false;
}

void Window::Resize(const WORD &width, const WORD &height)
{
    if (m_Width != width || m_Height != height)
    {
        m_Width = width;
        m_Height = height;

        m_Renderer->UpdateViewport(width, height);
    }
}

void Window::SetCameraMovement(long horizontalMovement, long verticalMovement)
{
    m_Camera->HorizontalMovement(horizontalMovement);
    m_Camera->VerticalMovement(verticalMovement);

    m_Renderer->UpdateCamera();
}

void Window::MoveCameraForward(float dt)
{
    m_Camera->MoveForward(dt);
}

void Window::MoveCameraBackward(float dt)
{
    m_Camera->MoveBackward(dt);
}

void Window::MoveCameraLeft(float dt)
{
    m_Camera->MoveLeft(dt);
}

void Window::MoveCameraRight(float dt)
{
    m_Camera->MoveRight(dt);
}

void Window::HandleCameraMovement(float dt)
{
    bool anyMovement = false;

    if (m_MustMoveCameraForward)
    {
        MoveCameraForward(dt);
        anyMovement = true;
    }

    if (m_MustMoveCameraBackward)
    {
        MoveCameraBackward(dt);
        anyMovement = true;
    }

    if (m_MustMoveCameraLeft)
    {
        MoveCameraLeft(dt);
        anyMovement = true;
    }

    if (m_MustMoveCameraRight)
    {
        MoveCameraRight(dt);
        anyMovement = true;
    }

    if (anyMovement)
    {
        m_Renderer->UpdateCamera();
    }
}

END_VISUALIZER_NAMESPACE
