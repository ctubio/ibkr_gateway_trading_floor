#pragma once

static const char* DIAMONDS_CLASS_NAME = "TNTDiamondsWindowClass";

void startDiamonds() { startGenericWindow(DIAMONDS_CLASS_NAME, "Diamonds", L"IBKRGatewayClient.Diamonds", 380, 240); }

LRESULT CALLBACK WndProcDiamonds(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {

    case WM_CREATE: {
        Session_AddWindow(hWnd);
        break;
    }

    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;

    case WM_MOVE:
        SaveWinPosition(hWnd, DIAMONDS_CLASS_NAME);
        break;

    case WM_DESTROY:
        SaveWinPosition(hWnd, DIAMONDS_CLASS_NAME);
        Session_RemoveWindow(hWnd);
        g_AppWindows[DIAMONDS_CLASS_NAME] = NULL;
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
