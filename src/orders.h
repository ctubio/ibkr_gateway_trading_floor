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
        SaveWinPosition(hWnd, ORDERS_CLASS_NAME);
        break;

    case WM_DESTROY:
        SaveWinPosition(hWnd, ORDERS_CLASS_NAME);
        Session_RemoveWindow(hWnd);
        g_AppWindows[ORDERS_CLASS_NAME] = NULL;
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
