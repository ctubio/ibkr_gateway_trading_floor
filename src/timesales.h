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
        SaveWinPosition(hWnd);
        break;

    case WM_DESTROY:
        SaveWinPosition(hWnd);
        Session_RemoveWindow(hWnd);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
