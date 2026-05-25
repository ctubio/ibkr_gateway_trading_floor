#pragma once

void StartDiamonds() { StartGenericWindow(DIAMONDS_CLASS_NAME, "Diamonds", L"IBKRGatewayClient.Diamonds", 1100, 420); }

#define ID_DIAMONDS_RESULTS_LIST 7001

// ── Column indices (keep in sync with diamondCols[]) ─────────────────────────
enum DiamondColIdx {
    DCOL_SYMBOL = 0,
    DCOL_POSITION,
    DCOL_AVGPRICE,
    DCOL_ASKSIZE,
    DCOL_ASK,
    DCOL_LAST,
    DCOL_BID,
    DCOL_BIDSIZE,
    DCOL_DAILYPNL,
    DCOL_CHGPCT,
    DCOL_UNREALIZED_PL,
    DCOL_UNREALIZED_PL_PCT,
    DCOL_MKTVAL,
    DCOL_PCT_NETLIQ,
    DCOL_DIV_YIELD,
    DCOL_DIV_DATE,
    DCOL_DIV_AMT,
    DCOL_ANNUAL_DIV,
    DCOL_COUNT
};

// ── Column definitions ────────────────────────────────────────────────────────

struct DiamondCol { const char* header; int width; int fmt; };
static const DiamondCol diamondCols[] = {
    { "Instrument",        90, LVCFMT_LEFT  },
    { "Position",          70, LVCFMT_RIGHT },
    { "Avg Price",         80, LVCFMT_RIGHT },
    { "Ask Size",          70, LVCFMT_RIGHT },
    { "Ask",               70, LVCFMT_RIGHT },
    { "Last",              70, LVCFMT_RIGHT },
    { "Bid",               70, LVCFMT_RIGHT },
    { "Bid Size",          70, LVCFMT_RIGHT },
    { "Daily P&L",         85, LVCFMT_RIGHT },
    { "Change %",          70, LVCFMT_RIGHT },
    { "Unrealized P&L",    95, LVCFMT_RIGHT },
    { "Unrealized P&L %",  95, LVCFMT_RIGHT },
    { "Market Value",      85, LVCFMT_RIGHT },
    { "% of Net Liq",      85, LVCFMT_RIGHT },
    { "Div Yield %",       80, LVCFMT_RIGHT },
    { "Div Date",          85, LVCFMT_RIGHT },
    { "Div Amount",        80, LVCFMT_RIGHT },
    { "Annual Div",        80, LVCFMT_RIGHT },
};
static_assert((int)(sizeof(diamondCols) / sizeof(diamondCols[0])) == DCOL_COUNT,
              "diamondCols count must match DiamondColIdx::DCOL_COUNT");

// ── Helpers ───────────────────────────────────────────────────────────────────

// Find a row in the Diamonds list by conId (stored in lParam).
static int Diamonds_FindRow(HWND hList, int conId) {
    int count = ListView_GetItemCount(hList);
    for (int i = 0; i < count; ++i) {
        LVITEMA lvi = {};
        lvi.mask  = LVIF_PARAM;
        lvi.iItem = i;
        ListView_GetItem(hList, &lvi);
        if ((int)lvi.lParam == conId) return i;
    }
    return -1;
}

// Update the market-data columns of one row from a TickerInfo snapshot.
static void Diamonds_UpdateMarketCols(HWND hList, int row, const TradingAPI::TickerInfo& t) {
    char buf[64];

    auto setNum = [&](int col, double val, const char* fmt) {
        if (val != 0.0) snprintf(buf, sizeof(buf), fmt, val);
        else            buf[0] = '\0';
        ListView_SetItemText(hList, row, col, buf);
    };
    auto setStr = [&](int col, const std::string& val) {
        ListView_SetItemText(hList, row, col, (LPSTR)val.c_str());
    };

    char symBuf[64];
    ListView_GetItemText(hList, row, DCOL_SYMBOL, symBuf, sizeof(symBuf));
    std::string symbol = symBuf;

    double shares = 0.0, avgCost = 0.0;
    {
        std::lock_guard<std::mutex> lock(api.getPortfolioMutex());
        auto& pmap = api.getPortfolioMap();
        if (pmap.count(symbol)) {
            shares = pmap[symbol].shares;
            avgCost = pmap[symbol].avgCost;
        }
    }

    double netLiq = 0.0;
    auto summary = api.getAccountSummary();
    if (summary.count("NetLiquidation")) netLiq = std::stod(summary["NetLiquidation"]);

    double mktVal     = shares * t.last;
    double unrlPnL    = shares * (t.last - avgCost);
    double unrlPnLPct = (avgCost != 0.0 && t.last != 0.0) ? ((t.last - avgCost) / avgCost * 100.0) : 0.0;
    double dailyPnL   = shares * t.change();
    double pctNetLiq  = (netLiq != 0.0 && mktVal != 0.0) ? (mktVal / netLiq * 100.0) : 0.0;

    setNum(DCOL_ASKSIZE,           t.askSize,          "%.0f");
    setNum(DCOL_ASK,               t.ask,              "%.2f");
    setNum(DCOL_LAST,              t.last,             "%.2f");
    setNum(DCOL_BID,               t.bid,              "%.2f");
    setNum(DCOL_BIDSIZE,           t.bidSize,          "%.0f");
    setNum(DCOL_DAILYPNL,          dailyPnL,           "%+.2f");
    setNum(DCOL_CHGPCT,            t.changePct(),      "%+.2f%%");
    setNum(DCOL_UNREALIZED_PL,     unrlPnL,            "%+.2f");
    setNum(DCOL_UNREALIZED_PL_PCT, unrlPnLPct,         "%+.2f%%");
    setNum(DCOL_MKTVAL,            mktVal,             "%.2f");
    setNum(DCOL_PCT_NETLIQ,        pctNetLiq,          "%.2f%%");
    setNum(DCOL_DIV_YIELD,         t.dividendYield(),  "%.2f%%");
    setStr(DCOL_DIV_DATE,          t.dividendDate);
    setNum(DCOL_DIV_AMT,           t.dividendAmount,   "%.3f");
    setNum(DCOL_ANNUAL_DIV,        t.annualDividends,  "%.3f");
}

// ── Repopulate ────────────────────────────────────────────────────────────────

static void Diamonds_Repopulate(HWND hWnd) {
    HWND hList = GetDlgItem(hWnd, ID_DIAMONDS_RESULTS_LIST);
    if (!hList) return;

    SendMessage(hList, WM_SETREDRAW, FALSE, 0);
    ListView_DeleteAllItems(hList);

    std::vector<TradingAPI::PositionInfo> rows;
    {
        std::lock_guard<std::mutex> lock(api.getPortfolioMutex());
        for (auto const& [sym, info] : api.getPortfolioMap())
            rows.push_back(info);
    }

    std::sort(rows.begin(), rows.end(),
              [](const TradingAPI::PositionInfo& a, const TradingAPI::PositionInfo& b) {
                  return a.symbol < b.symbol;
              });

    char buf[64];
    for (int i = 0; i < (int)rows.size(); ++i) {
        const auto& pos = rows[i];

        // ── Symbol (col 0) with conId in lParam ──────────────────────────────
        LVITEMA lvi = {};
        lvi.mask     = LVIF_TEXT | LVIF_PARAM;
        lvi.iItem    = i;
        lvi.iSubItem = 0;
        lvi.lParam   = pos.conId;
        lvi.pszText  = (LPSTR)pos.symbol.c_str();
        ListView_InsertItem(hList, &lvi);

        // ── Position columns ─────────────────────────────────────────────────
        snprintf(buf, sizeof(buf), "%.4g", pos.shares);
        ListView_SetItemText(hList, i, DCOL_POSITION, buf);

        snprintf(buf, sizeof(buf), "%.2f", pos.avgCost);
        ListView_SetItemText(hList, i, DCOL_AVGPRICE, buf);

        // ── Market data columns — pre-fill from cache if already available ───
        TradingAPI::TickerInfo tickInfo;
        if (api.getTickerData(pos.conId, pos.symbol, tickInfo))
            Diamonds_UpdateMarketCols(hList, i, tickInfo);
    }

    SendMessage(hList, WM_SETREDRAW, TRUE, 0);
    RedrawWindow(hList, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

    SetWindowTextA(hWnd, ("Diamonds: " + std::to_string(rows.size()) + " Positions").c_str());
}

// ── Window procedure ──────────────────────────────────────────────────────────

LRESULT CALLBACK WndProcDiamonds(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {

    case WM_CREATE: {
        HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;

        HWND hList = CreateWindowExA(
            WS_EX_CLIENTEDGE, "SysListView32", "",
            WS_CHILD | WS_VISIBLE | WS_BORDER |
            LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER,
            0, 0, 1100, 420,
            hWnd, (HMENU)ID_DIAMONDS_RESULTS_LIST, hInst, NULL);

        DWORD exStyle = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER;
        if (Settings_DarkMode()) exStyle |= LVS_EX_GRIDLINES;
        ListView_SetExtendedListViewStyle(hList, exStyle);

        LVCOLUMNA lvc = {};
        lvc.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_FMT;
        for (int i = 0; i < DCOL_COUNT; ++i) {
            lvc.cx      = diamondCols[i].width;
            lvc.pszText = (LPSTR)diamondCols[i].header;
            lvc.fmt     = diamondCols[i].fmt;
            ListView_InsertColumn(hList, i, &lvc);
        }

        api.setDiamondsWindow(hWnd);
        api.addApiUpdateWindow(hWnd);
        break;
    }

    case WM_SIZE: {
        HWND hList = GetDlgItem(hWnd, ID_DIAMONDS_RESULTS_LIST);
        if (!hList) return 0;
        RECT rc; GetClientRect(hWnd, &rc);
        MoveWindow(hList, 0, 0, rc.right, rc.bottom, TRUE);
        break;
    }

    case WM_DIAMONDS_UPDATE: {
        Diamonds_Repopulate(hWnd);
        break;
    }

    // ── Live market data update for one symbol ────────────────────────────────
    // Posted by Impl::tickPrice / tickSize / tickString / tickGeneric
    // lParam = new std::string("conId.symbol") — we own it and must delete.
    case WM_TICKER_UPDATE: {
        auto* key = reinterpret_cast<std::string*>(lParam);
        if (!key) break;

        auto dot = key->find('.');
        if (dot != std::string::npos) {
            int conId          = std::stoi(key->substr(0, dot));
            std::string symbol = key->substr(dot + 1);

            TradingAPI::TickerInfo info;
            if (api.getTickerData(conId, symbol, info)) {
                HWND hList = GetDlgItem(hWnd, ID_DIAMONDS_RESULTS_LIST);
                int row = Diamonds_FindRow(hList, conId);
                if (row >= 0)
                    Diamonds_UpdateMarketCols(hList, row, info);
            }
        }
        delete key;
        break;
    }

    // ── Connection state changed ──────────────────────────────────────────────
    case WM_API_UPDATE: {
        HWND hList = GetDlgItem(hWnd, ID_DIAMONDS_RESULTS_LIST);
        if (!hList) break;
        if (api.isMarketDataConnected() && api.isTradingConnected()) {
            // Re-request positions (market data re-subscribed in positionEnd()).
            api.setDiamondsWindow(hWnd);
        } else {
            // Clear everything — positions and prices are stale.
            ListView_DeleteAllItems(hList);
        }
        break;
    }

    // ── Notification handling ─────────────────────────────────────────────────
    case WM_NOTIFY: {
        NMHDR* hdr = (NMHDR*)lParam;
        if (hdr->idFrom != ID_DIAMONDS_RESULTS_LIST) break;

        if (hdr->code == NM_DBLCLK || hdr->code == NM_RCLICK) {
            LPNMITEMACTIVATE act = (LPNMITEMACTIVATE)lParam;
            int row = act->iItem;
            if (row >= 0) {
                HWND hList = GetDlgItem(hWnd, ID_DIAMONDS_RESULTS_LIST);
                char sym[256] = {};
                ListView_GetItemText(hList, row, 0, sym, sizeof(sym));
                LVITEMA item = {};
                item.mask  = LVIF_PARAM;
                item.iItem = row;
                ListView_GetItem(hList, &item);
                StartTimesales(sym, (int)item.lParam);
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
                    if (cd->iSubItem == DCOL_CHGPCT || cd->iSubItem == DCOL_DAILYPNL ||
                        cd->iSubItem == DCOL_UNREALIZED_PL || cd->iSubItem == DCOL_UNREALIZED_PL_PCT) {
                        HWND hList = GetDlgItem(hWnd, ID_DIAMONDS_RESULTS_LIST);
                        char buf[32] = {};
                        ListView_GetItemText(hList, (int)cd->nmcd.dwItemSpec, cd->iSubItem, buf, sizeof(buf));
                        double val = atof(buf);
                        if      (val > 0) cd->clrText = RGB(80, 200, 120);
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

    case WM_DESTROY:
        api.unsetDiamondsWindow();   // cancels market data subscriptions + positions
        api.removeApiUpdateWindow(hWnd);
        break;
    }

    return HandleCommonMessages(hWnd, message, wParam, lParam);
}