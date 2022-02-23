#!/bin/sh
strip Clicorn.exe
cp ./dll/x64/webview.dll ./dll/x64/webview.dll.save
cp ./dll/x64/WebView2Loader.dll ./dll/x64/WebView2Loader.dll.save
bin2header Clicorn.exe
bin2header ./dll/x64/webview.dll
bin2header ./dll/x64/WebView2Loader.dll
mv ./dll/x64/webview.dll.save ./dll/x64/webview.dll
mv ./dll/x64/WebView2Loader.dll.save ./dll/x64/WebView2Loader.dll
x86_64-w64-mingw32-gcc -mwindows -O2 ./src/winboot.c -I./dll/x64 -I./ -o AlicornPE.exe
strip AlicornPE.exe
upx -9 AlicornPE.exe
