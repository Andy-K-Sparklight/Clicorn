bin_PROGRAMS=Clicorn
Clicorn_SOURCES=src/main.c src/cJSON/cJSON.c src/webview/webview.cc src/binds/bind.c src/binds/err.c src/binds/fs.c  src/binds/app.c
Clicorn_CXXFLAGS=-I$(top_srcdir)/include ${libs_CFLAGS}
Clicorn_CPPFLAGS=-I$(top_srcdir)/include
Clicorn_LDADD=${libs_LIBS} -lstdc++
