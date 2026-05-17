#pragma once

static const char* TIMESALES_CLASS_NAME = "TNTTimesalesWindowClass";

void startTimesales() { startGenericWindow(TIMESALES_CLASS_NAME, "Timesales", L"IBKRGatewayClient.Timesales", 380, 240); }

LRESULT CALLBACK WndProcTimesales(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {

    case WM_CREATE: {
        Session_AddWindow(hWnd);
        break;
    }

    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;

    case WM_MOVE:
        SaveWinPosition(hWnd, TIMESALES_CLASS_NAME);
        break;

    case WM_DESTROY:
        SaveWinPosition(hWnd, TIMESALES_CLASS_NAME);
        Session_RemoveWindow(hWnd);
        g_AppWindows[TIMESALES_CLASS_NAME] = NULL;
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
