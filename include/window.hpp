#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <Windows.h>

namespace cxxopts
{
    class ParseResult;
}

BEGIN_VISUALIZER_NAMESPACE

class Camera;
class Renderer;

class Window
{
public:
    ~Window();

    Window(const Window &) = delete;
    Window(Window &&) = delete;

    inline static Window &GetInstance()
    {
        static Window instance;
        return instance;
    }

    bool Update() const;
    void Close();

    bool InitWindow(const std::string_view&, uint16_t width, uint16_t height, cxxopts::ParseResult& commandLineOptions);
    
    void Run();

    void DestroyWindow();

    void Resize(const WORD &width, const WORD &height);

    void SetCameraMovement(long horizontalMovement, long verticalMovement);

    inline void SetMouseButtonDown(bool mouseButtonDown) { m_MouseButtonDown = mouseButtonDown; }
    inline bool GetMouseButtonDown() const { return m_MouseButtonDown; }

    inline void SetMustMoveCameraForward(bool mustMoveCameraForward) { m_MustMoveCameraForward = mustMoveCameraForward; }
    inline void SetMustMoveCameraBackward(bool mustMoveCameraBackward) { m_MustMoveCameraBackward = mustMoveCameraBackward; }
    inline void SetMustMoveCameraLeft(bool mustMoveCameraLeft) { m_MustMoveCameraLeft = mustMoveCameraLeft; }
    inline void SetMustMoveCameraRight(bool mustMoveCameraRight) { m_MustMoveCameraRight = mustMoveCameraRight; }

    inline RECT& GetWindowRect() { return m_WindowRect; }

    inline uint32_t GetWidth() const { return m_Width; }
    inline uint32_t GetHeight() const { return m_Height; }

    inline HWND GetHWND() const { return m_hWnd; }

private:
    Window();

    void MoveCameraForward(float dt);
    void MoveCameraBackward(float dt);
    void MoveCameraLeft(float dt);
    void MoveCameraRight(float dt);

    void HandleCameraMovement(float dt);

    uint16_t m_Width, m_Height;
    bool m_WindowShouldRun = true;
    bool m_IsInitialized = false;
    std::string_view m_Name;

    HINSTANCE m_hInstance = nullptr;
    HWND m_hWnd = nullptr;
    HGLRC m_hrc = nullptr;
    HDC m_hDC = nullptr;
    RECT m_WindowRect;

    std::shared_ptr<Camera> m_Camera;
    std::unique_ptr<Renderer> m_Renderer;

    cxxopts::ParseResult* m_CommandLineOptions = nullptr;

    enum
    {
        Scene1 = 0,
        Scene2,
        Scene3,
        Scene4,
        Scene5,
        Scene6,
        SceneCount
    } m_SceneID = SceneCount;

    bool m_MouseButtonDown = false;
    bool m_MustMoveCameraForward = false;
    bool m_MustMoveCameraBackward = false;
    bool m_MustMoveCameraLeft = false;
    bool m_MustMoveCameraRight = false;

    bool m_DirectStateAccessAvailable = false;
    bool m_BufferStorageAvailable = false;
};

END_VISUALIZER_NAMESPACE

#endif // !WINDOW_HPP
