#pragma once

static const char* LEVELS_CLASS_NAME = "TNTLevelsWindowClass";

void startLevels() { startGenericWindow(LEVELS_CLASS_NAME, "Levels", L"IBKRGatewayClient.Levels", 380, 240); }

LRESULT CALLBACK WndProcLevels(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {

    case WM_CREATE: {
        Session_AddWindow(hWnd);
        break;
    }

    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;

    case WM_MOVE:
        SaveWinPosition(hWnd, LEVELS_CLASS_NAME);
        break;

    case WM_DESTROY:
        SaveWinPosition(hWnd, LEVELS_CLASS_NAME);
        Session_RemoveWindow(hWnd);
        g_AppWindows[LEVELS_CLASS_NAME] = NULL;
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
