#pragma once

static const char* ORDERS_CLASS_NAME = "TNTOrdersWindowClass";

void startOrders() { startGenericWindow(ORDERS_CLASS_NAME, "Orders", L"IBKRGatewayClient.Orders", 380, 240); }

LRESULT CALLBACK WndProcOrders(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
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

