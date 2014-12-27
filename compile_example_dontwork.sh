#!/bin/bash -x
# Already added: -L/usr/lib//x86_64-linux-gnu/
#gcc -lX11 $(pkg-config --libs --cflags gstreamer-0.10) -lgstinterfaces-0.10 $(sdl-config --libs --cflags) sdl-gstreamer.c -o sdl-gstreamer
# = 
#gcc -lX11 -pthread -I/usr/include/gstreamer-0.10 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/libxml2 -pthread -lgstreamer-0.10 -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lxml2 -lglib-2.0 -lgstinterfaces-0.10 -L/usr/lib/x86_64-linux-gnu -lSDL -I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT sdl-gstreamer.c -o sdl-gstreamer
gcc -v -I/usr/include/gstreamer-0.10 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/libxml2 -I/usr/include/SDL -pthread -lX11 -lgstreamer-0.10 -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lxml2 -lglib-2.0 -lgstinterfaces-0.10 -lSDL -L/usr/lib/x86_64-linux-gnu -D_GNU_SOURCE=1 -D_REENTRANT sdl-gstreamer.c -o sdl-gstreamer

#gcc -v -lX11 -pthread -I/usr/include/gstreamer-0.10 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/libxml2 -pthread -lgstreamer-0.10 -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lxml2 -lglib-2.0 -lgstinterfaces-0.10 -L/usr/lib/x86_64-linux-gnu -lSDL -I/usr/include/SDL -D_GNU_SOURCE=1 -gstreamer.c -o sdl-gstreamer

