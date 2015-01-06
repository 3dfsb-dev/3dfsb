#!/bin/sh

# Note:
# - you need to apt-get install gtk-doc-tools 
# - you need to download gstreamer, gst-plugins-base, gst-plugins-bad and build them from source in that order (./autogen.sh && make)
# - libgtk-3-dev # for gtk/gtk.h, not really needed, I guess...
# - you need to download gst-plugins-good and build it from source to have avidemux (for video/msvideo) 
# - you need to download gst-libav and build it from source to have more audio and video decoders


prefix=/usr/lib/x86_64-linux-gnu/


execfind ()
{
    for cmd in $*; 
    do
        if "$cmd" --version > /dev/null; then
            echo "$cmd";
            break;
        fi;
    done;
}
                                            
SDL_CONFIG=$(execfind sdl-config sdl11-config sdl10-config sdl12-config \/boot\/develop\/tools/gnupro\/bin\/sdl-config);

if ! "$SDL_CONFIG" --version > /dev/null; then
    echo "Cannot find the sdl-config script.";
    echo "Please check your SDL installation.";
    exit 1;
fi

echo "Using $SDL_CONFIG.";
ARGS1=$($SDL_CONFIG --libs);
ARGS2=$($SDL_CONFIG --cflags);

if uname -s | grep -i -c "LINUX" > /dev/null; then 
    echo "GNU/Linux detected.";
    echo "compiling...";    
	gtkstuff="-I /usr/include/gtk-3.0/ -I /usr/include/cairo -I /usr/include/pango-1.0 -I /usr/include/gdk-pixbuf-2.0 -I /usr/include/atk-1.0"
	includes="-I/usr/local/include/gstreamer-1.0 $gtkstuff"
	#warnings="-std=c99 -pedantic -Wall -Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes"
	warnings="-Wall"

	# Dynamic:
	link=$(pkg-config --libs --cflags gstreamer-1.0)" "$(pkg-config --libs --cflags gstreamer-gl-1.0)" "$(pkg-config --libs --cflags gstreamer-video-1.0)" "$(pkg-config --libs --cflags gdk-pixbuf-2.0)"-lGL -lGLU"

	# Static:
	# This works fine in the first steps, but then fails in a way very similar to how it fails on pluto
	#link=$(pkg-config --cflags gstreamer-1.0)" "$(pkg-config --cflags gstreamer-gl-1.0)" "$(pkg-config --cflags gstreamer-video-1.0)" "$(pkg-config --cflags gdk-pixbuf-2.0)
	#gccopt="-static -static-libgcc"

    gcc $warnings $gccopt -v $ARGS1 $ARGS2 -I/usr/local/include -I/usr/include/ -I/usr/X11R6/include $includes -L/usr/lib/ -L/usr/local/lib/ -L/usr/X11R6/lib -D_THREAD_SAFE -g -x c $link -lSDL_image -lglut -lXmu -lXi -lXext -lX11 -lm -lsmpeg -o 3dfsb 3dfsb.c

elif uname -s | grep -i -c "BEOS" > /dev/null; then 
    echo "BeOS detected.";
    echo "compiling...";
    gcc $ARGS1 $ARGS2 -I/boot/develop/tools/gnupro/include/ -I/boot/develop/headers/be/opengl/ -L/boot/home/config/lib -L/boot/develop/tools/gnupro/lib/ -lSDL_image -lGL -lglut -lsmpeg -D_THREAD_SAFE -O2 -x c -o 3dfsb 3dfsb.c;
elif uname -s | grep -i -c "BSD" > /dev/null; then 
    echo "BSD detected.";
    echo "compiling...";
    gcc $ARGS1 $ARGS2 -I/usr/local/include -I/usr/include/ -I/usr/X11R6/include -L/usr/lib/ -L/usr/local/lib/ -L/usr/X11R6/lib -lSDL_image -lGL -lGLU -lglut -lXmu -lXi -lXext -lX11 -lm -lsmpeg -D_THREAD_SAFE -O2 -x c -o 3dfsb 3dfsb.c;
else
    echo "Unknown OS. If you are running Linux, BeOS or";
    echo "FreeBSD please send me the output of 'uname -s'." 
    exit 1;
fi;

exit 0;
