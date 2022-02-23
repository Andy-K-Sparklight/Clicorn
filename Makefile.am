bin_PROGRAMS=Clicorn

Clicorn_SOURCES=src/main.c src/cJSON/cJSON.c src/binds/bind.c src/binds/fs.c  src/binds/app.c src/binds/z.c src/binds/os.c

if WINDOWS
else
    Clicorn_SOURCES+=src/webview/webview.cc
endif


if WINDOWS
    Clicorn_CXXFLAGS=-I$(top_srcdir)/include -mwindows -L./dll/x64
else
    Clicorn_CXXFLAGS=-I$(top_srcdir)/include ${libs_CFLAGS}
endif

if WINDOWS
    Clicorn_CPPFLAGS=-I$(top_srcdir)/include
else
    Clicorn_CPPFLAGS=-I$(top_srcdir)/include ${libs_CFLAGS}
endif

if WINDOWS
    Clicorn_LDADD=-lwebview -lWebView2Loader
else
    Clicorn_LDADD=${libs_LIBS} -lstdc++
endif

