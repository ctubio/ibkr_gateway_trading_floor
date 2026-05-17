# ibkr_gateway_trading_floor

A Windows tray application for trades using Interactive Brokers Gateway.

## Features
- Tray icon with connection status
- Auto-connects to IB Gateway on startup
- Watchdog reconnection
- Symbol book — create lists of symbols
- Window position persistence via registry

## Requirements
- IBKR Pro account
- IBKR Gateway for Windows installed from https://www.interactivebrokers.com/en/trading/ibgateway-latest.php

## Build
Cross-compiled on Linux (Raspberry Pi) targeting Windows x86_64.
```sh
    make lib   # build dependencies (once)
    make       # build TNT.exe
```

## Build-in Dependencies
- IBKR TWS C++ API
- Protobuf + Abseil (via CMake)
- Intel Decimal Floating Point Library

## Usage
- Enjoy!
- To exit `TNT.exe`, look for the Exit menu option in your system tray icon

## Uninstall
- Delete `TNT.exe`
- Open the Registry Editor and delete the folder: `Computer\HKEY_CURRENT_USER\Software\ibkr_gateway_trading_floor`
