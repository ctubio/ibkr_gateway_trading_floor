#pragma once

static const char* NEWS_CLASS_NAME = "TNTNewsWindowClass";

void startNews() { startGenericWindow(NEWS_CLASS_NAME, "News", L"IBKRGatewayClient.News", 380, 240); }

LRESULT CALLBACK WndProcNews(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {

    case WM_CREATE: {
        Session_AddWindow(hWnd);
        break;
    }

    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;

    case WM_MOVE:
        SaveWinPosition(hWnd, NEWS_CLASS_NAME);
        break;

    case WM_DESTROY:
        SaveWinPosition(hWnd, NEWS_CLASS_NAME);
        Session_RemoveWindow(hWnd);
        g_AppWindows[NEWS_CLASS_NAME] = NULL;
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
