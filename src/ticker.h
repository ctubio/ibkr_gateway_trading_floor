#pragma once

static const char* TICKER_CLASS_NAME = "TNTTickerWindowClass";

void startTicker() { startGenericWindow(TICKER_CLASS_NAME, "Ticker", L"IBKRGatewayClient.Ticker", 380, 240); }

LRESULT CALLBACK WndProcTicker(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
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

    case WM_ERASEBKGND: {
        HDC hdc = (HDC)wParam;
        RECT rc;
        GetClientRect(hWnd, &rc);
        FillRect(hdc, &rc, Settings_DarkMode() ? hDarkBrush : (HBRUSH)(COLOR_BTNFACE + 1));
        return 1;
    }

    // Edit boxes, listboxes
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX: {
        if (!Settings_DarkMode()) break;
        HDC hdc = (HDC)wParam;
        SetTextColor(hdc, DM_TEXT);
        SetBkColor(hdc, DM_BG2);
        return (LRESULT)hDarkBrush2;
    }

    // Static labels
    case WM_CTLCOLORSTATIC: {
        if (!Settings_DarkMode()) break;
        HDC hdc = (HDC)wParam;
        SetTextColor(hdc, DM_TEXT);
        SetBkColor(hdc, DM_BG);
        return (LRESULT)hDarkBrush;
    }

    // Buttons — BS_AUTOCHECKBOX and regular buttons
    case WM_CTLCOLORBTN: {
        if (!Settings_DarkMode()) break;
        HDC hdc = (HDC)wParam;
        SetTextColor(hdc, DM_TEXT);
        SetBkColor(hdc, DM_BG);
        return (LRESULT)hDarkBrush;
    }

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
