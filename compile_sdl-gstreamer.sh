gcc -lX11 $(pkg-config --libs --cflags gstreamer-0.10) -lgstinterfaces-0.10 $(sdl-config --libs --cflags) sdl-gstreamer.c -o sdl-gstreamer
