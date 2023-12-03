#include "Platforms/Windows/PlatformProgressWindows.h"
#ifdef IS_PLATFORM_WINDOWS

#include <Windows.h>
#include <CommCtrl.h>

namespace Insight
{
    namespace Platforms::Windows
    {
#define ID_DEFAULTPROGRESSCTRL	401
#define ID_SMOOTHPROGRESSCTRL	402
#define ID_VERTICALPROGRESSCTRL	403

        LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
            switch (uMsg)
            {
            case WM_DESTROY:
                ::PostQuitMessage(0);
                break;

            default:
                return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
            }

            return 0;
        }

        PlatformProgressWindows::PlatformProgressWindows()
        {
        }

        PlatformProgressWindows::~PlatformProgressWindows()
        {
            Close();
        }

        void PlatformProgressWindows::Show(std::string title)
        {
            if (!m_windowHandle)
            {
                // Setup window class attributes.
                WNDCLASSEX wcex;
                ZeroMemory(&wcex, sizeof(wcex));

                wcex.cbSize = sizeof(wcex);
                wcex.style = CS_HREDRAW | CS_VREDRAW;
                wcex.lpszClassName = m_windowClassName;
                wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW);
                wcex.hCursor = LoadCursor(GetModuleHandle(nullptr), IDC_ARROW);
                wcex.lpfnWndProc = WndProc;
                wcex.hInstance = GetModuleHandle(nullptr);

                RegisterClassEx(&wcex);

                HWND monitor = GetDesktopWindow();
                RECT monitorRect;
                GetClientRect(monitor, &monitorRect);

                const std::wstring windowTitle = std::wstring(title.begin(), title.end());
                const int windowWidth = 480;
                const int windowHeight = 480;

                const int progressBarPadding = 20;
                const int progressBarWith = windowWidth - (progressBarPadding * 3);
                const int progressBarHeight = 40;

                // Create the window.
                m_windowHandle = CreateWindowEx(
                    WS_EX_TOPMOST,
                    m_windowClassName,
                    windowTitle.c_str(),
                    WS_OVERLAPPEDWINDOW,
                    (monitorRect.right * 0.5f) - (windowWidth * 0.5f),
                    (monitorRect.bottom * 0.5f) - (windowHeight * 0.5f),
                    windowWidth,
                    windowHeight,
                    NULL,
                    NULL,
                    GetModuleHandle(nullptr),
                    NULL);

                // Create default progress bar.
                m_progressBarHandle = CreateWindowExA(
                    0,
                    PROGRESS_CLASSA,
                    "",
                    WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
                    progressBarPadding,
                    progressBarPadding,
                    progressBarWith,
                    progressBarHeight,
                    (HWND)m_windowHandle,
                    (HMENU)ID_DEFAULTPROGRESSCTRL,
                    GetModuleHandle(nullptr),
                    NULL);

                // Display the window.
                ::ShowWindow((HWND)m_windowHandle, SW_SHOWDEFAULT);
                ::UpdateWindow((HWND)m_windowHandle);

                UpdateProgress(0);
            }
        }

        void PlatformProgressWindows::Close()
        {
            if (m_windowHandle)
            {
                DestroyWindow((HWND)m_progressBarHandle);
                DestroyWindow((HWND)m_windowHandle);
                m_progressBarHandle = nullptr;
                m_windowHandle = nullptr;

                // Unregister window class, freeing the memory that was
                // previously allocated for this window.
                ::UnregisterClass(m_windowClassName, GetModuleHandle(nullptr));
            }
        }

        void PlatformProgressWindows::UpdateProgress(const float progress, std::string message)
        {
            SendMessage((HWND)m_progressBarHandle, PBM_SETPOS, progress, 0);
        }
    }
}
#endif