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
            SaveWinPosition(hWnd);
            break;

        case WM_DESTROY:
            SaveWinPosition(hWnd);
            Session_RemoveWindow(hWnd);
            break;

        default: {
            LRESULT res = HandleDarkModeMessages(hWnd, message, wParam, lParam);
            if (res) return res;
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    return 0;
}
