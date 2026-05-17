#pragma once

static const char* SETTINGS_CLASS_NAME = "TNTSettingsWindowClass";

void startSettings() { startGenericWindow(SETTINGS_CLASS_NAME, "Settings", L"IBKRGatewayClient.Settings", 300, 280); }

#define ID_SETTINGS_KILL_GATEWAY 4001
#define ID_SETTINGS_DARK_MODE    4002
#define ID_SETTINGS_PLAY_SOUNDS  4003
#define ID_SETTINGS_AUTO_GATEWAY 4004
#define ID_SETTINGS_DEBUG_LOG    4005
// ─── Debug Log ────────────────────────────────────────────────────────────────
void FlushDebugBuffer() {
    if (!hDebugEdit || !IsWindow(hDebugEdit)) return;
    SendMessageA(hDebugEdit, WM_SETTEXT, 0, (LPARAM)""); // clear first
    for (const auto& msg : debugBuffer) {
        int len = GetWindowTextLength(hDebugEdit);
        SendMessage(hDebugEdit, EM_SETSEL, len, len);
        SendMessageA(hDebugEdit, EM_REPLACESEL, FALSE, (LPARAM)msg.c_str());
    }
    // Scroll to bottom
    SendMessage(hDebugEdit, EM_SETSEL, -1, -1);
    SendMessage(hDebugEdit, EM_SCROLLCARET, 0, 0);
}

LRESULT CALLBACK WndProcDebugLog(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE: {
            hDebugEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL |
                ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY,
                0, 0, 0, 0, hWnd, NULL, GetModuleHandle(NULL), NULL);
            RECT rc;
            GetClientRect(hWnd, &rc);
            SetWindowPos(hDebugEdit, NULL, 0, 0, rc.right, rc.bottom, SWP_NOZORDER);
            FlushDebugBuffer(); // ← show all buffered messages
            break;
        }
        case WM_SIZE: {
            if (hDebugEdit) {
                RECT rc;
                GetClientRect(hWnd, &rc);
                SetWindowPos(hDebugEdit, NULL, 0, 0, rc.right, rc.bottom, SWP_NOZORDER);
            }
            break;
        }
        case WM_CLOSE:
            DestroyWindow(hWnd);
            break;
        case WM_DESTROY:
            hDebugEdit = NULL;
            hDebugLogWnd = NULL;
            break;
        
        default: {
            LRESULT res = HandleDarkModeMessages(hWnd, message, wParam, lParam);
            if (res) return res;
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    return 0;
}

bool Settings_KillGatewayOnExit() {
    return Settings_Load("KillGatewayOnExit", 0) != 0;
}

bool Settings_AutoGateway() {
    return Settings_Load("AutoGateway", 0) != 0;
}

// Call this on every window after creating it
void ApplyDarkModeToAllWindows() {
    // Enumerate all top-level windows owned by this process
    InitDarkBrushes();
    EnumWindows([](HWND hWnd, LPARAM) -> BOOL {
        DWORD pid;
        GetWindowThreadProcessId(hWnd, &pid);
        if (pid == GetCurrentProcessId()) {
            ApplyDarkMode(hWnd);
            InvalidateRect(hWnd, NULL, TRUE); // ← force repaint
            RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
        }
        return TRUE;
    }, 0);
}

LRESULT CALLBACK WndProcSettings(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE: {
            Session_AddWindow(hWnd);
            HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
            int margin = 8;

            HWND hChkAutoGtw = CreateWindowA("BUTTON", "Auto-start IBKR Gateway",
                WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                margin, margin, 250, 24,
                hWnd, (HMENU)ID_SETTINGS_AUTO_GATEWAY, hInst, NULL);
            if (Settings_AutoGateway())
                SendMessage(hChkAutoGtw, BM_SETCHECK, BST_CHECKED, 0);

            HWND hChkKill = CreateWindowA("BUTTON", "Kill IBKR Gateway on exit",
                WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                margin, margin + 32, 250, 24,
                hWnd, (HMENU)ID_SETTINGS_KILL_GATEWAY, hInst, NULL);
            if (Settings_KillGatewayOnExit())
                SendMessage(hChkKill, BM_SETCHECK, BST_CHECKED, 0);

            HWND hChkDark = CreateWindowA("BUTTON", "Dark mode",
                WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                margin, margin + 64, 250, 24,
                hWnd, (HMENU)ID_SETTINGS_DARK_MODE, hInst, NULL);
            if (Settings_DarkMode())
                SendMessage(hChkDark, BM_SETCHECK, BST_CHECKED, 0);

            HWND hChkSounds = CreateWindowA("BUTTON", "Play sounds",
                WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                margin, margin + 96, 250, 24,
                hWnd, (HMENU)ID_SETTINGS_PLAY_SOUNDS, hInst, NULL);
            if (Settings_Load("PlaySounds", 0))
                SendMessage(hChkSounds, BM_SETCHECK, BST_CHECKED, 0);

            HWND hBtnDebug = CreateWindowA("BUTTON", "Debug Messages",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
                margin, margin + 128, 250, 24,
                hWnd, (HMENU)ID_SETTINGS_DEBUG_LOG, hInst, NULL);

            ApplyDarkMode(hWnd);
            break;
        }

        case WM_COMMAND:
            if (LOWORD(wParam) == ID_SETTINGS_PLAY_SOUNDS) {
                HWND hChk = GetDlgItem(hWnd, ID_SETTINGS_PLAY_SOUNDS);
                DWORD checked = (SendMessage(hChk, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0;
                Settings_Save("PlaySounds", checked);
            }
            if (LOWORD(wParam) == ID_SETTINGS_AUTO_GATEWAY) {
                HWND hChk = GetDlgItem(hWnd, ID_SETTINGS_AUTO_GATEWAY);
                DWORD checked = (SendMessage(hChk, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0;
                Settings_Save("AutoGateway", checked);
            }
            if (LOWORD(wParam) == ID_SETTINGS_DEBUG_LOG) {
                if (!hDebugLogWnd || !IsWindow(hDebugLogWnd)) {
                    static bool classRegistered = false;
                    if (!classRegistered) {
                        WNDCLASSA wc = {};
                        wc.lpfnWndProc   = WndProcDebugLog;
                        wc.hInstance     = GetModuleHandle(NULL);
                        wc.lpszClassName = "DebugMessagesDialog";
                        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
                        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
                        RegisterClassA(&wc);
                        classRegistered = true;
                    }

                    int dlgW = 600, dlgH = 400;
                    int dlgX = (GetSystemMetrics(SM_CXSCREEN) - dlgW) / 2;
                    int dlgY = (GetSystemMetrics(SM_CYSCREEN) - dlgH) / 2;

                    hDebugLogWnd = CreateWindowExA(
                        WS_EX_TOPMOST,
                        "DebugMessagesDialog", "Debug Messages",
                        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                        dlgX, dlgY, dlgW, dlgH,
                        NULL, NULL, GetModuleHandle(NULL), NULL);
                } else {
                    ShowWindow(hDebugLogWnd, SW_SHOW);
                    SetForegroundWindow(hDebugLogWnd);
                    FlushDebugBuffer(); // ← refresh in case new messages arrived while hidden
                }
            }
            if (LOWORD(wParam) == ID_SETTINGS_KILL_GATEWAY) {
                HWND hChk = GetDlgItem(hWnd, ID_SETTINGS_KILL_GATEWAY);
                DWORD checked = (SendMessage(hChk, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0;
                Settings_Save("KillGatewayOnExit", checked);
            }
            if (LOWORD(wParam) == ID_SETTINGS_DARK_MODE) {
                HWND hChk = GetDlgItem(hWnd, ID_SETTINGS_DARK_MODE);
                DWORD checked = (SendMessage(hChk, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0;
                Settings_Save("DarkMode", checked);
                InitDarkBrushes();
                ApplyDarkModeToAllWindows();
                // Force full repaint of this window and all children
                InvalidateRect(hWnd, NULL, TRUE);
                RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW);
            }
            break;

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
