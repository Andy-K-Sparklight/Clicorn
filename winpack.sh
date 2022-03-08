#!/bin/sh
strip Clicorn.exe
bin2header Clicorn.exe
bin2header ./dll/x64/webview.dll
bin2header ./dll/x64/WebView2Loader.dll
bin2header ./dll/x64/libcurl.dll
x86_64-w64-mingw32-gcc -mwindows -O2 ./src/winboot.c -I./dll/x64 -I./ -o AlicornPE.exe
strip AlicornPE.exe
upx -9 AlicornPE.exe
