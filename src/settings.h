#pragma once

static const char* SETTINGS_CLASS_NAME = "TNTSettingsWindowClass";

void startSettings() { startGenericWindow(SETTINGS_CLASS_NAME, "Settings", L"IBKRGatewayClient.Settings", 300, 80); }

#define ID_SETTINGS_KILL_GATEWAY 4001

bool Settings_KillGatewayOnExit() {
    return Settings_Load("KillGatewayOnExit", 0) != 0;
}

LRESULT CALLBACK WndProcSettings(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {

    case WM_CREATE: {
        Session_AddWindow(hWnd);
        HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
        int margin = 8;

        HWND hChk = CreateWindowA("BUTTON", "Kill IBKR Gateway on exit",
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            margin, margin, 250, 24,
            hWnd, (HMENU)ID_SETTINGS_KILL_GATEWAY, hInst, NULL);

        // Load saved state
        if (Settings_KillGatewayOnExit())
            SendMessage(hChk, BM_SETCHECK, BST_CHECKED, 0);

        break;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_SETTINGS_KILL_GATEWAY) {
            HWND hChk = GetDlgItem(hWnd, ID_SETTINGS_KILL_GATEWAY);
            DWORD checked = (SendMessage(hChk, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0;
            Settings_Save("KillGatewayOnExit", checked);
        }
        break;

    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;

    case WM_MOVE:
        SaveWinPosition(hWnd, SETTINGS_CLASS_NAME);
        break;

    case WM_DESTROY:
        SaveWinPosition(hWnd, SETTINGS_CLASS_NAME);
        Session_RemoveWindow(hWnd);
        g_AppWindows[SETTINGS_CLASS_NAME] = NULL;
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
