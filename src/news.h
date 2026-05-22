#pragma once

void startNews() { startGenericWindow(NEWS_CLASS_NAME, "News", L"IBKRGatewayClient.News", 420, 500); }

#define ID_NEWS_LIST_COMBO   6001
#define ID_NEWS_SYM_COMBO    6002
#define ID_NEWS_RESULTS_LIST 6003

static HWND hNewsListCombo = NULL;
static HWND hNewsSymCombo  = NULL;
static HWND hNewsResults   = NULL;
static std::vector<std::string> newsSymEntries;

static const int NEWS_COMBO_H   = 24;
static const int NEWS_COMBO_GAP = 8;
static const int NEWS_SELECTOR_H = 8 + NEWS_COMBO_H + 8;

// ── Helpers ───────────────────────────────────────────────────────────────────

static std::string News_GetSelectedList() {
    int sel = (int)SendMessage(hNewsListCombo, CB_GETCURSEL, 0, 0);
    if (sel == CB_ERR) return "";
    int len = (int)SendMessage(hNewsListCombo, CB_GETLBTEXTLEN, sel, 0);
    if (len <= 0) return "";
    std::string name(len + 1, '\0');
    SendMessageA(hNewsListCombo, CB_GETLBTEXT, sel, (LPARAM)name.data());
    name.resize(len);
    return name;
}

static bool News_LoadListCombo(const std::string& savedList = "") {
    SendMessage(hNewsListCombo, CB_RESETCONTENT, 0, 0);
    Book_LoadAllLists(hNewsListCombo);
    if ((int)SendMessage(hNewsListCombo, CB_GETCOUNT, 0, 0) == 0) return false;
    int idx = CB_ERR;
    if (!savedList.empty())
        idx = (int)SendMessageA(hNewsListCombo, CB_FINDSTRINGEXACT, -1, (LPARAM)savedList.c_str());
    SendMessage(hNewsListCombo, CB_SETCURSEL, idx == CB_ERR ? 0 : idx, 0);
    return true;
}

static bool News_LoadSymbolCombo(const std::string& savedEntry = "") {
    SendMessage(hNewsSymCombo, CB_RESETCONTENT, 0, 0);
    newsSymEntries.clear();
    std::string listName = News_GetSelectedList();
    if (listName.empty()) return false;
    for (const auto& full : Book_ReadListEntries(listName.c_str())) {
        newsSymEntries.push_back(full);
        SendMessageA(hNewsSymCombo, CB_ADDSTRING, 0, (LPARAM)Book_DisplayLabel(full).c_str());
    }
    if (newsSymEntries.empty()) return false;
    int idx = CB_ERR;
    for (int i = 0; i < (int)newsSymEntries.size(); ++i)
        if (newsSymEntries[i] == savedEntry) { idx = i; break; }
    SendMessage(hNewsSymCombo, CB_SETCURSEL, idx == CB_ERR ? 0 : idx, 0);
    return true;
}

// ── Layout ────────────────────────────────────────────────────────────────────

static void News_Layout(HWND hWnd) {
    if (!hNewsResults) return;
    RECT rc; GetClientRect(hWnd, &rc);
    const int m = 8;

    bool combosVisible = hNewsListCombo && IsWindowVisible(hNewsListCombo);

    if (combosVisible) {
        int availW = rc.right - m * 2 - NEWS_COMBO_GAP;
        int eachW  = availW / 2;
        SetWindowPos(hNewsListCombo, NULL, m,                           m, eachW, 200,
                     SWP_NOZORDER | SWP_NOACTIVATE);
        SetWindowPos(hNewsSymCombo,  NULL, m + eachW + NEWS_COMBO_GAP, m, eachW, 200,
                     SWP_NOZORDER | SWP_NOACTIVATE);
        MoveWindow(hNewsResults, 0, NEWS_SELECTOR_H, rc.right, rc.bottom - NEWS_SELECTOR_H, TRUE);
    } else {
        MoveWindow(hNewsResults, 0, 0, rc.right, rc.bottom, TRUE);
    }
}

// ── News request ──────────────────────────────────────────────────────────────

void News_RequestForSymbol(const std::string& fullEntry) {
    auto firstDot = fullEntry.find('.');
    if (firstDot == std::string::npos) return;
    std::string conIdStr = fullEntry.substr(0, firstDot);
    std::string rest     = fullEntry.substr(firstDot + 1);
    auto secondDot       = rest.find('.');
    std::string symbol   = (secondDot != std::string::npos) ? rest.substr(0, secondDot) : rest;

    Settings_SaveString("LastNewsEntry", fullEntry);
    LogDebug("News request for: " + symbol + " conId: " + conIdStr);

    HWND newsWnd = g_AppWindows[NEWS_CLASS_NAME];
    if (newsWnd) SetWindowTextA(newsWnd, ("News: " + symbol).c_str());

    if (hNewsResults) ListView_DeleteAllItems(hNewsResults);
    api.reqNewsForSymbol(std::stoi(conIdStr), symbol);
}

// ── Window procedure ──────────────────────────────────────────────────────────

LRESULT CALLBACK WndProcNews(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {

    case WM_CREATE: {
        HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
        const int m = 8;

        // Combos side by side — hidden until focused
        hNewsListCombo = CreateWindowA("COMBOBOX", NULL,
            WS_CHILD | WS_VSCROLL | CBS_DROPDOWNLIST,
            m, m, 180, 200,
            hWnd, (HMENU)ID_NEWS_LIST_COMBO, hInst, NULL);

        hNewsSymCombo = CreateWindowA("COMBOBOX", NULL,
            WS_CHILD | WS_VSCROLL | CBS_DROPDOWNLIST,
            m + 180 + NEWS_COMBO_GAP, m, 180, 200,
            hWnd, (HMENU)ID_NEWS_SYM_COMBO, hInst, NULL);

        // Results ListView — single "Headline" column, full width
        hNewsResults = CreateWindowExA(
            WS_EX_CLIENTEDGE, "SysListView32", "",
            WS_CHILD | WS_VISIBLE | WS_BORDER |
            LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER,
            0, 0, 400, 500,
            hWnd, (HMENU)ID_NEWS_RESULTS_LIST, hInst, NULL);

        DWORD exStyle = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER;
        if (Settings_DarkMode()) exStyle |= LVS_EX_GRIDLINES;
        ListView_SetExtendedListViewStyle(hNewsResults, exStyle);

        LVCOLUMNA lvc = {};
        lvc.mask    = LVCF_WIDTH | LVCF_TEXT | LVCF_FMT;
        lvc.cx      = 400;
        lvc.pszText = (LPSTR)"Headline";
        lvc.fmt     = LVCFMT_LEFT;
        ListView_InsertColumn(hNewsResults, 0, &lvc);

        api.setNewsWindow(hWnd);
        SetWindowTextA(hWnd, "News");

        std::string lastList  = Settings_LoadString("LastNewsList");
        std::string lastEntry = Settings_LoadString("LastNewsEntry");
        News_LoadListCombo(lastList);
        if (News_LoadSymbolCombo(lastEntry)) {
            int sel = (int)SendMessage(hNewsSymCombo, CB_GETCURSEL, 0, 0);
            if (sel != CB_ERR && sel < (int)newsSymEntries.size())
                News_RequestForSymbol(newsSymEntries[sel]);
        }
        break;
    }

    case WM_SIZE:
        News_Layout(hWnd);
        break;

    case WM_ACTIVATE:
        if (LOWORD(wParam) != WA_INACTIVE) {
            ShowWindow(hNewsListCombo, SW_SHOW);
            ShowWindow(hNewsSymCombo,  SW_SHOW);
        } else {
            ShowWindow(hNewsListCombo, SW_HIDE);
            ShowWindow(hNewsSymCombo,  SW_HIDE);
        }
        News_Layout(hWnd);
        return 0;

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_NEWS_LIST_COMBO && HIWORD(wParam) == CBN_SELCHANGE) {
            Settings_SaveString("LastNewsList", News_GetSelectedList());
            News_LoadSymbolCombo();
            if (!newsSymEntries.empty()) News_RequestForSymbol(newsSymEntries[0]);
        }
        if (LOWORD(wParam) == ID_NEWS_SYM_COMBO && HIWORD(wParam) == CBN_SELCHANGE) {
            int sel = (int)SendMessage(hNewsSymCombo, CB_GETCURSEL, 0, 0);
            if (sel != CB_ERR && sel < (int)newsSymEntries.size())
                News_RequestForSymbol(newsSymEntries[sel]);
        }
        break;

    case WM_NEWS_RESULTS: {
        if (!hNewsResults) break;
        auto news = api.getNewsResults();
        ListView_DeleteAllItems(hNewsResults);
        for (int i = 0; i < (int)news.size(); ++i) {
            LVITEMA lvi = {};
            lvi.mask    = LVIF_TEXT;
            lvi.iItem   = i;
            lvi.pszText = (LPSTR)news[i].c_str();
            ListView_InsertItem(hNewsResults, &lvi);
        }
        // Stretch the single column to fill the list width
        RECT rc; GetClientRect(hNewsResults, &rc);
        ListView_SetColumnWidth(hNewsResults, 0, rc.right);
        break;
    }

    // ── Dark mode ─────────────────────────────────────────────────────────────
    case WM_NOTIFY: {
        NMHDR* hdr = (NMHDR*)lParam;
        if (hdr->idFrom != ID_NEWS_RESULTS_LIST || hdr->code != NM_CUSTOMDRAW) break;
        NMLVCUSTOMDRAW* cd = (NMLVCUSTOMDRAW*)lParam;
        if (!Settings_DarkMode()) break;
        switch (cd->nmcd.dwDrawStage) {
            case CDDS_PREPAINT:     return CDRF_NOTIFYITEMDRAW;
            case CDDS_ITEMPREPAINT:
                cd->clrTextBk = (cd->nmcd.dwItemSpec % 2 == 0) ? DM_BG : DM_BG2;
                cd->clrText   = DM_TEXT;
                return CDRF_DODEFAULT;
        }
        break;
    }

    case WM_DESTROY:
        api.setNewsWindow(NULL);
        hNewsListCombo = NULL;
        hNewsSymCombo  = NULL;
        hNewsResults   = NULL;
        newsSymEntries.clear();
        break;
    }

    return HandleCommonMessages(hWnd, message, wParam, lParam);
}