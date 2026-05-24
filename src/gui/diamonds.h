#pragma once

void startDiamonds() { startGenericWindow(DIAMONDS_CLASS_NAME, "Diamonds", L"IBKRGatewayClient.Diamonds", 700, 400); }

#define ID_DIAMONDS_RESULTS_LIST 7001

static HWND hDiamondsResults = NULL;

// ── Column definitions ────────────────────────────────────────────────────────

struct DiamondCol { const char* header; int width; int fmt; };
static const DiamondCol diamondCols[] = {
    { "Symbol",   100, LVCFMT_LEFT  },
    { "Shares",    80, LVCFMT_RIGHT },
    { "Avg Cost",  90, LVCFMT_RIGHT },
    { "Daily PnL", 90, LVCFMT_RIGHT },
    { "52W %",     75, LVCFMT_RIGHT },
    { "Mkt Value", 90, LVCFMT_RIGHT },
    { "Mkt Cap",  110, LVCFMT_RIGHT },
};
static const int DIAMOND_COL_COUNT = (int)(sizeof(diamondCols) / sizeof(diamondCols[0]));

// ── Repopulate ────────────────────────────────────────────────────────────────

static void Diamonds_Repopulate(HWND hWnd) {
    SendMessage(hDiamondsResults, WM_SETREDRAW, FALSE, 0);
    ListView_DeleteAllItems(hDiamondsResults);

    std::vector<TradingAPI::PositionInfo> rows;
    {
        std::lock_guard<std::mutex> lock(api.getPortfolioMutex());
        for (auto const& [sym, info] : api.getPortfolioMap())
            rows.push_back(info);
    }

    // Sort alphabetically by symbol
    std::sort(rows.begin(), rows.end(),
              [](const TradingAPI::PositionInfo& a, const TradingAPI::PositionInfo& b) {
                  return a.symbol < b.symbol;
              });

    char buf[64];
    for (int i = 0; i < (int)rows.size(); ++i) {
        const auto& info = rows[i];

        LVITEMA lvi = {};
        lvi.mask     = LVIF_TEXT | LVIF_PARAM;
        lvi.iItem    = i;
        lvi.iSubItem = 0;
        lvi.lParam   = info.conId;
        lvi.pszText  = (LPSTR)info.symbol.c_str();
        ListView_InsertItem(hDiamondsResults, &lvi);

        snprintf(buf, sizeof(buf), "%.2f", info.shares);
        ListView_SetItemText(hDiamondsResults, i, 1, buf);

        snprintf(buf, sizeof(buf), "%.2f", info.avgCost);
        ListView_SetItemText(hDiamondsResults, i, 2, buf);

        snprintf(buf, sizeof(buf), "%.2f", info.dailyPnL);
        ListView_SetItemText(hDiamondsResults, i, 3, buf);

        snprintf(buf, sizeof(buf), "%.2f%%", info.fiftyTwoWeekChange);
        ListView_SetItemText(hDiamondsResults, i, 4, buf);

        snprintf(buf, sizeof(buf), "%.2f", info.marketValue);
        ListView_SetItemText(hDiamondsResults, i, 5, buf);

        snprintf(buf, sizeof(buf), "%.2f", info.marketCap);
        ListView_SetItemText(hDiamondsResults, i, 6, buf);
    }

    SendMessage(hDiamondsResults, WM_SETREDRAW, TRUE, 0);
    RedrawWindow(hDiamondsResults, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
    
    SetWindowTextA(hWnd, ("Diamonds: " + std::to_string(rows.size()) + " Positions").c_str());
}

// ── Window procedure ──────────────────────────────────────────────────────────

LRESULT CALLBACK WndProcDiamonds(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {

    case WM_CREATE: {
        HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;

        hDiamondsResults = CreateWindowExA(
            WS_EX_CLIENTEDGE, "SysListView32", "",
            WS_CHILD | WS_VISIBLE | WS_BORDER |
            LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER,
            0, 0, 700, 400,
            hWnd, (HMENU)ID_DIAMONDS_RESULTS_LIST, hInst, NULL);

        DWORD exStyle = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER;
        if (Settings_DarkMode()) exStyle |= LVS_EX_GRIDLINES;
        ListView_SetExtendedListViewStyle(hDiamondsResults, exStyle);

        LVCOLUMNA lvc = {};
        lvc.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_FMT;
        for (int i = 0; i < DIAMOND_COL_COUNT; ++i) {
            lvc.cx      = diamondCols[i].width;
            lvc.pszText = (LPSTR)diamondCols[i].header;
            lvc.fmt     = diamondCols[i].fmt;
            ListView_InsertColumn(hDiamondsResults, i, &lvc);
        }

        api.setDiamondsWindow(hWnd);

        api.addApiUpdateWindow(hWnd);
        break;
    }

    case WM_SIZE: {
        if (!hDiamondsResults) return 0;
        RECT rc;
        GetClientRect(hWnd, &rc);
        MoveWindow(hDiamondsResults, 0, 0, rc.right, rc.bottom, TRUE);
        break;
    }

    case WM_DIAMONDS_UPDATE: {
        if (hDiamondsResults) Diamonds_Repopulate(hWnd);
        break;
    }

    // ── Dark mode ─────────────────────────────────────────────────────────────
    case WM_NOTIFY: {
        NMHDR* hdr = (NMHDR*)lParam;
        if (hdr->idFrom != ID_DIAMONDS_RESULTS_LIST) break;
        if (hdr->code == NM_DBLCLK) {
            LPNMITEMACTIVATE act = (LPNMITEMACTIVATE)lParam;
            int row = act->iItem;
            if (row != -1) {
                char text[256];
                ListView_GetItemText(hDiamondsResults, row, 0, text, sizeof(text));
                char msg[512];
                LVITEMA item{};
                item.mask = LVIF_PARAM;
                item.iItem = row;
                ListView_GetItem(hDiamondsResults, &item);
                int conId = (int)item.lParam;
                snprintf(msg, sizeof(msg), "conId: %d\nSymbol: %s", conId, text);
                MessageBoxA(hWnd, msg, "NM_DBLCLK", MB_OK);
            }
        }
        if (hdr->code == NM_RCLICK) {
            LPNMITEMACTIVATE act = (LPNMITEMACTIVATE)lParam;
            int row = act->iItem;
            if (row != -1) {
                char text[256];
                ListView_GetItemText(hDiamondsResults, row, 0, text, sizeof(text));
                char msg[512];
                LVITEMA item{};
                item.mask = LVIF_PARAM;
                item.iItem = row;
                ListView_GetItem(hDiamondsResults, &item);
                int conId = (int)item.lParam;
                snprintf(msg, sizeof(msg), "conId: %d\nSymbol: %s", conId, text);
                MessageBoxA(hWnd, msg, "NM_RCLICK", MB_OK);
            }
        }
        if (hdr->code == NM_CUSTOMDRAW) {
            NMLVCUSTOMDRAW* cd = (NMLVCUSTOMDRAW*)lParam;
            bool dark = Settings_DarkMode();

            switch (cd->nmcd.dwDrawStage) {
                case CDDS_PREPAINT:
                    return CDRF_NOTIFYITEMDRAW;

                case CDDS_ITEMPREPAINT:
                    if (dark) {
                        cd->clrTextBk = (cd->nmcd.dwItemSpec % 2 == 0) ? DM_BG : DM_BG2;
                        cd->clrText   = DM_TEXT;
                    }
                    return CDRF_NOTIFYSUBITEMDRAW;

                case CDDS_ITEMPREPAINT | CDDS_SUBITEM: {
                    if (cd->iSubItem == 3) { // Daily PnL — green/red
                        char buf[32] = {};
                        ListView_GetItemText(hDiamondsResults, (int)cd->nmcd.dwItemSpec, 3, buf, sizeof(buf));
                        double val = atof(buf);
                        if (val > 0) cd->clrText = RGB(80, 200, 120);
                        else if (val < 0) cd->clrText = RGB(220, 80, 80);
                        if (dark) cd->clrTextBk = (cd->nmcd.dwItemSpec % 2 == 0) ? DM_BG : DM_BG2;
                        return CDRF_NEWFONT;
                    }
                    return CDRF_DODEFAULT;
                }
            }
        }
        break;
    }

    case WM_API_UPDATE: {
        if (hDiamondsResults) {
            if (api.isMarketDataConnected() && api.isTradingConnected()) {
                Diamonds_Repopulate(hWnd);
            } else {
                ListView_DeleteAllItems(hDiamondsResults);
            }
        }
        break;
    }
    
    case WM_DESTROY:
        api.unsetDiamondsWindow();
        hDiamondsResults = NULL;
        api.removeApiUpdateWindow(hWnd);
        break;
    }

    return HandleCommonMessages(hWnd, message, wParam, lParam);
}