// Standalone Windows drawing/timer exercise. No network, files, hooks or payloads.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <algorithm>
#include <string>

namespace {
constexpr UINT_PTR TIMER_ID = 1;
constexpr int CLOSE_BUTTON = 100;
constexpr ULONGLONG DURATION_MS = 30000;

struct State {
    ULONGLONG started = 0;
    HFONT heading = nullptr;
    HFONT body = nullptr;
    HFONT face = nullptr;
    HWND closeButton = nullptr;
    bool testClose = false;
    bool testEscape = false;
    bool expectedExit = false;
};

void text(HDC dc, HFONT font, const std::wstring& value, RECT area) {
    HGDIOBJ previous = SelectObject(dc, font);
    DrawTextW(dc, value.c_str(), -1, &area, DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);
    SelectObject(dc, previous);
}

LRESULT CALLBACK windowProcedure(HWND window, UINT message, WPARAM key, LPARAM details) {
    auto* state = reinterpret_cast<State*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    switch (message) {
        case WM_CREATE: {
            auto* creation = reinterpret_cast<CREATESTRUCTW*>(details);
            state = static_cast<State*>(creation->lpCreateParams);
            SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
            HDC dc = GetDC(window);
            int dpi = GetDeviceCaps(dc, LOGPIXELSY);
            ReleaseDC(window, dc);
            auto font = [dpi](int points, int weight) {
                return CreateFontW(-MulDiv(points, dpi, 72), 0, 0, 0, weight, FALSE, FALSE, FALSE,
                                   DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                   CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
            };
            state->heading = font(27, FW_SEMIBOLD);
            state->body = font(15, FW_NORMAL);
            state->face = font(56, FW_LIGHT);
            state->closeButton = CreateWindowW(L"BUTTON", L"Close simulation", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                40, 390, 210, 40, window, reinterpret_cast<HMENU>(CLOSE_BUTTON), creation->hInstance, nullptr);
            SendMessageW(state->closeButton, WM_SETFONT, reinterpret_cast<WPARAM>(state->body), TRUE);
            state->started = GetTickCount64();
            SetTimer(window, TIMER_ID, 100, nullptr);
            return 0;
        }
        case WM_GETMINMAXINFO: {
            auto* limits = reinterpret_cast<MINMAXINFO*>(details);
            limits->ptMinTrackSize = {700, 580};
            return 0;
        }
        case WM_SIZE:
            if (state && state->closeButton) MoveWindow(state->closeButton, 40, HIWORD(details) - 76, 220, 42, TRUE);
            InvalidateRect(window, nullptr, TRUE);
            return 0;
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT: {
            PAINTSTRUCT painting{};
            HDC dc = BeginPaint(window, &painting);
            RECT area{};
            GetClientRect(window, &area);
            HBRUSH blue = CreateSolidBrush(RGB(0, 103, 184));
            FillRect(dc, &area, blue);
            DeleteObject(blue);
            if (state) {
                SetBkMode(dc, TRANSPARENT);
                SetTextColor(dc, RGB(255, 255, 255));
                int right = area.right - 40;
                text(dc, state->body, L"C++ COURSEWORK / SIMULATION", {40, 25, right, 65});
                text(dc, state->face, L":(", {40, 72, right, 180});
                text(dc, state->heading, L"Blue-screen simulation", {40, 190, right, 245});
                text(dc, state->body, L"This is a drawing demonstration. Windows has not crashed, and no restart is taking place.", {40, 255, right, 330});
                auto elapsed = GetTickCount64() - state->started;
                auto progress = static_cast<unsigned>(std::min<ULONGLONG>(100, elapsed * 100 / DURATION_MS));
                text(dc, state->body, std::to_wstring(progress) + L"% demonstration complete", {40, 340, right, 380});
                text(dc, state->body, L"Press Esc, Enter, Alt+F4 or the close button. The demo also closes automatically after 30 seconds.",
                     {285, area.bottom - 82, right, area.bottom - 10});
            }
            EndPaint(window, &painting);
            return 0;
        }
        case WM_TIMER:
            if (!state) return 0;
            if (state->testClose) { PostMessageW(window, WM_CLOSE, 0, 0); return 0; }
            if (state->testEscape) { PostMessageW(window, WM_KEYDOWN, VK_ESCAPE, 0); return 0; }
            if (GetTickCount64() - state->started >= DURATION_MS) { DestroyWindow(window); return 0; }
            InvalidateRect(window, nullptr, FALSE);
            return 0;
        case WM_KEYDOWN:
            if (key == VK_ESCAPE || key == VK_RETURN) {
                if (state) state->expectedExit = true;
                DestroyWindow(window);
                return 0;
            }
            break;
        case WM_COMMAND:
            if (LOWORD(key) == CLOSE_BUTTON) { DestroyWindow(window); return 0; }
            break;
        case WM_CLOSE:
            if (state) state->expectedExit = true;
            DestroyWindow(window);
            return 0;
        case WM_DESTROY:
            KillTimer(window, TIMER_ID);
            if (state) {
                DeleteObject(state->heading);
                DeleteObject(state->body);
                DeleteObject(state->face);
            }
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(window, message, key, details);
}
} // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR commandLine, int show) {
    State state;
    state.testClose = std::wstring(commandLine) == L"--self-test-close";
    state.testEscape = std::wstring(commandLine) == L"--self-test-escape";
    SetProcessDPIAware();
    WNDCLASSW type{};
    type.lpfnWndProc = windowProcedure;
    type.hInstance = instance;
    type.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    type.lpszClassName = L"CourseworkBlueScreenDemo";
    if (!RegisterClassW(&type)) return 1;
    HWND window = CreateWindowW(type.lpszClassName, L"Blue-screen simulation | C++ coursework",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 960, 660, nullptr, nullptr, instance, &state);
    if (!window) return 1;
    bool testing = state.testClose || state.testEscape;
    ShowWindow(window, testing ? SW_HIDE : show);
    MSG event{};
    while (GetMessageW(&event, nullptr, 0, 0) > 0) {
        // Esc/Enter also work when the ordinary close button has keyboard focus.
        if (event.message == WM_KEYDOWN && (event.wParam == VK_ESCAPE || event.wParam == VK_RETURN)) {
            SendMessageW(window, event.message, event.wParam, event.lParam);
            continue;
        }
        TranslateMessage(&event);
        DispatchMessageW(&event);
    }
    UnregisterClassW(type.lpszClassName, instance);
    return testing && !state.expectedExit ? 1 : 0;
}
