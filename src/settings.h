#pragma once

HWND hSettingsWnd = NULL;
static const char* SETTINGS_CLASS_NAME = "TNTSettingsWindowClass";


LRESULT CALLBACK WndProcSettings(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {

    case WM_CREATE: {
        break;
    }

    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;

    case WM_MOVE:
        SaveWinPosition(hWnd, SETTINGS_CLASS_NAME);
        break;

    case WM_DESTROY:
        SaveWinPosition(hWnd, SETTINGS_CLASS_NAME);
        hSettingsWnd = NULL;
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void startSettings() {
    if (hSettingsWnd && IsWindow(hSettingsWnd)) {
        ShowWindow(hSettingsWnd, SW_SHOW);
        SetForegroundWindow(hSettingsWnd);
    } else {
        int x = CW_USEDEFAULT, y = CW_USEDEFAULT, w = 380, h = 240;
        LoadWinPosition(SETTINGS_CLASS_NAME, x, y, w, h);

        hSettingsWnd = CreateWindowExA(
            WS_EX_APPWINDOW,
            SETTINGS_CLASS_NAME,
            "Settings",
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
            x, y, w, h,
            NULL, NULL, GetModuleHandle(NULL), NULL
        );

        SetWindowTaskbarId(hSettingsWnd, L"IBKRTunnel.Settings");
    }
}
