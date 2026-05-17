#pragma once

static const char* COINS_CLASS_NAME = "TNTCoinsWindowClass";

void startCoins() { startGenericWindow(COINS_CLASS_NAME, "Coins", L"IBKRGatewayClient.Coins", 380, 240); }

LRESULT CALLBACK WndProcCoins(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {

    case WM_CREATE: {
        Session_AddWindow(hWnd);
        break;
    }

    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;

    case WM_MOVE:
        SaveWinPosition(hWnd, COINS_CLASS_NAME);
        break;

    case WM_DESTROY:
        SaveWinPosition(hWnd, COINS_CLASS_NAME);
        Session_RemoveWindow(hWnd);
        g_AppWindows[COINS_CLASS_NAME] = NULL;
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
