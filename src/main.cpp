#include "api/gateway.h"
#include "api/debug.h"
#include "api/registry.h"
#include "api/sound.h"
#include "api/shared.h"

#include "gui/settings.h"
#include "gui/book.h"
#include "gui/timesales.h"
#include "gui/diamonds.h"
#include "gui/coins.h"
#include "gui/news.h"
#include "gui/levels.h"
#include "gui/orders.h"
#include "gui/ticker.h"
#include "gui/dashboard.h"

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow) {
    MutexInstance();

    InitDarkBrushes();
    
    registerWindowClass(hInst, WndProcBookNewList,   BOOK_NEW_LIST_CLASS_NAME,  2);
    registerWindowClass(hInst, WndProcBook,          BOOK_CLASS_NAME,           2);
    registerWindowClass(hInst, WndProcCoins,         COINS_CLASS_NAME,          3);
    registerWindowClass(hInst, WndProcDiamonds,      DIAMONDS_CLASS_NAME,       4);
    registerWindowClass(hInst, WndProcSettings,      SETTINGS_CLASS_NAME,       5);
    registerWindowClass(hInst, WndProcDebugLog,      DEBUGLOG_CLASS_NAME,      11);
    registerWindowClass(hInst, WndProcNews,          NEWS_CLASS_NAME,           6);
    registerWindowClass(hInst, WndProcNewsArticle,   NEWS_ARTICLE_CLASS_NAME,   6);
    registerWindowClass(hInst, WndProcTicker,        TICKER_CLASS_NAME,         9);
    registerWindowClass(hInst, WndProcTimesales,     TIMESALES_CLASS_NAME,      7);
    registerWindowClass(hInst, WndProcLevels,        LEVELS_CLASS_NAME,         8);
    registerWindowClass(hInst, WndProcOrders,        ORDERS_CLASS_NAME,        10);
    registerWindowClass(hInst, WndProcDashboard,     DASHBOARD_CLASS_NAME,      1);
	startDashboard(hInst);

    // Initialize common controls (ListView, Tab, etc.)
    INITCOMMONCONTROLSEX icex = { sizeof(icex), ICC_WIN95_CLASSES | ICC_LISTVIEW_CLASSES | ICC_TAB_CLASSES };
    InitCommonControlsEx(&icex);

    Session_RestoreWindows(
        startBook,    startCoins,    startDiamonds,
        startNews,    startSettings, startTimesales,
        startLevels,  startTicker,   startOrders,
        startDebugLog
    );

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
	
    return (int)msg.wParam;
}