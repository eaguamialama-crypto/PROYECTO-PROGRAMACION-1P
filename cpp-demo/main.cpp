#include "ArticulatedExerciseControl.h"

#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")

namespace {

constexpr wchar_t kMainWindowClass[] = L"FitFlexFigureDemoWindow";
constexpr int kExerciseControlId = 1001;

fitflex::ArticulatedExerciseControl g_exerciseControl;
fitflex::ExerciseKind g_currentExercise = fitflex::ExerciseKind::Squat;
HINSTANCE g_instance = nullptr;

LRESULT CALLBACK MainWindowProcedure(
    HWND window,
    UINT message,
    WPARAM wParam,
    LPARAM lParam) {
    switch (message) {
    case WM_CREATE: {
        RECT client{};
        GetClientRect(window, &client);
        RECT bounds{ 12, 12, client.right - 12, client.bottom - 12 };
        if (g_exerciseControl.Create(
                window, g_instance, kExerciseControlId, bounds) == nullptr) {
            return -1;
        }
        g_exerciseControl.SetExercise(g_currentExercise);
        return 0;
    }
    case WM_SIZE: {
        const int width = LOWORD(lParam);
        const int height = HIWORD(lParam);
        HWND child = GetDlgItem(window, kExerciseControlId);
        if (child != nullptr) {
            MoveWindow(child, 12, 12, width - 24, height - 24, TRUE);
        }
        return 0;
    }
    case WM_KEYDOWN:
        if (wParam == VK_SPACE) {
            g_currentExercise = g_currentExercise == fitflex::ExerciseKind::Squat
                ? fitflex::ExerciseKind::WallPushUp
                : fitflex::ExerciseKind::Squat;
            g_exerciseControl.SetExercise(g_currentExercise);
            return 0;
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        break;
    }

    return DefWindowProcW(window, message, wParam, lParam);
}

bool RegisterMainWindowClass(HINSTANCE instance) {
    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = MainWindowProcedure;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hbrBackground =
        reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    windowClass.lpszClassName = kMainWindowClass;
    return RegisterClassExW(&windowClass) != 0;
}

} // namespace

int WINAPI wWinMain(
    HINSTANCE instance,
    HINSTANCE,
    PWSTR,
    int showCommand) {
    Gdiplus::GdiplusStartupInput startupInput;
    ULONG_PTR gdiplusToken = 0;
    if (Gdiplus::GdiplusStartup(&gdiplusToken, &startupInput, nullptr)
        != Gdiplus::Ok) {
        return 1;
    }

    g_instance = instance;
    const bool registered =
        RegisterMainWindowClass(instance) &&
        fitflex::ArticulatedExerciseControl::RegisterWindowClass(instance);
    if (!registered) {
        Gdiplus::GdiplusShutdown(gdiplusToken);
        return 1;
    }

    HWND window = CreateWindowExW(
        0,
        kMainWindowClass,
        L"FitFlex - Figuras articuladas (Espacio: cambiar ejercicio)",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        920,
        850,
        nullptr,
        nullptr,
        instance,
        nullptr);
    if (window == nullptr) {
        Gdiplus::GdiplusShutdown(gdiplusToken);
        return 1;
    }

    ShowWindow(window, showCommand);
    UpdateWindow(window);

    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    Gdiplus::GdiplusShutdown(gdiplusToken);
    return static_cast<int>(message.wParam);
}
