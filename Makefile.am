bin_PROGRAMS=Clicorn

Clicorn_SOURCES=src/main.c src/cJSON/cJSON.c src/miniz/miniz.c src/base64/encode.c src/base64/decode.c src/base64/buffer.c src/binds/bind.c src/binds/fs.c  src/binds/app.c src/binds/z.c src/binds/os.c src/binds/net.c src/binds/sha1.c

if WINDOWS
else
    Clicorn_SOURCES+=src/webview/webview.cc
endif


if WINDOWS
    Clicorn_CXXFLAGS=-I$(top_srcdir)/include -L$(top_srcdir)/dll/x64 -I$(top_srcdir)/lib/curl/include -pthread
else
    Clicorn_CXXFLAGS=-I$(top_srcdir)/include ${libs_CFLAGS} -pthread
endif

if WINDOWS
    Clicorn_CPPFLAGS=-I$(top_srcdir)/include -I$(top_srcdir)/lib/curl/include -L$(top_srcdir)/dll/x64 -pthread
else
    Clicorn_CPPFLAGS=-I$(top_srcdir)/include ${libs_CFLAGS} -pthread
endif

if WINDOWS
    Clicorn_LDADD=-L$(top_srcdir)/lib/curl -lwebview -lWebView2Loader -lcurl
else
    Clicorn_LDADD=${libs_LIBS} -lstdc++
endif

