#!/bin/sh
# Compile 3dfsb using sdl-config and pkg-config to find the necessary CFLAGS paths and linker flags

usage() {
	echo "Usage: $0 [i386]"
	echo "Add the i386 option to compile for 32-bit (i386) architectures on a 64-bit machine"
	#echo "Starting build in 2 seconds..."; sleep 2
}
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

#########################
# EXECUTION STARTS HERE #
#########################
i386="$1"

usage

# On 64 bit machines, pkg-config will search for pkg-configs in /usr/lib/x86_64-linux-gnu/pkgconfig/ by default
if [ "$i386" = "i386" ]; then
	export PKG_CONFIG_PATH=/usr/lib/i386-linux-gnu/pkgconfig/
	i386option="-m32"
fi

# SDL stuff
SDL_CONFIG=$(execfind sdl-config sdl11-config sdl10-config sdl12-config \/boot\/develop\/tools/gnupro\/bin\/sdl-config);
if ! "$SDL_CONFIG" --version > /dev/null; then
    echo "Cannot find the sdl-config script.";
    echo "Please check your SDL installation.";
    exit 1;
fi
echo "Using $SDL_CONFIG.";
SDL_CFLAGS=$($SDL_CONFIG --cflags);	# Example: -I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT
SDL_LIBS=$($SDL_CONFIG --libs);		# Example: -L/usr/lib/x86_64-linux-gnu -lSDL

GSTREAMER_CFLAGS=$(pkg-config --cflags gstreamer-1.0)	# Example: -pthread -I/usr/local/include/gstreamer-1.0 -I/usr/local/lib/gstreamer-1.0/include -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include
GSTREAMER_LIBS=$(pkg-config --libs gstreamer-1.0)	# Example: -L/usr/local/lib -lgstreamer-1.0 -lgobject-2.0 -lglib-2.0

# GTK is useful for debugging purposes (dumping an image to a file)
#GTK_CFLAGS=$(pkg-config --cflags gtk+-2.0)
#GTK_LIBS=$(pkg-config --libs gtk+-2.0)

OTHER_LIBS=$(pkg-config --libs glu)	# Example: -lGLU -lGL

NOPKGCONFIG_LIBS="-lglut -lmagic -lm -lxdo -lX11 -lXtst"

echo "Converting built-in images to XPM format with imagemagick's 'mogrify' tool..."
mogrify -format xpm images/*.png
if [ $? -ne 0 ]; then
	echo "ERROR converting built-in images to XPM format"
	exit 1
fi

echo "Converting other built-in files to .h file format with the xxd tool..."
xxd -i resources/xorg.conf > resources/xorg.conf.h
if [ $? -ne 0 ]; then
	echo "ERROR creating resources/xorg.conf.h"
	exit 1
fi

if uname -s | grep -i -c "LINUX" > /dev/null; then 
    echo "GNU/Linux detected.";
    echo -n "Compiling...";
    # On Linux, pkg-config is easier to use than sdl-config
    SDL_CFLAGS=$(pkg-config --cflags SDL_image);	# Example: -D_GNU_SOURCE=1 -D_REENTRANT -I/usr/include/SDL 
    SDL_LIBS=$(pkg-config --libs SDL_image);		# Example: -lSDL_image -lSDL 

    # -Wconversion fails for a non-fixable reason, IIRC...
    # -Werror=format-nonliteral fails when we read the command to execute from the config file and put it in an snprintf() to substitute the %s... but can't we just do a simple find and replace of the %s?
    warnings="-pedantic -pedantic-errors -std=c99 -Waggregate-return -Wall -Wcast-align -Wcast-qual -Wchar-subscripts  -Wcomment -Wdisabled-optimization -Werror -Wextra -Wfloat-equal  -Wformat -Wformat-security -Wformat-y2k -Wformat=2 -Wmissing-prototypes -Wpointer-arith -Wshadow -Wstrict-prototypes -Wunused"

    # This fails
    #gccopt="-static -static-libgcc"

    gccopt="-g"		# debugging info by default

    gcc $i386option $gccopt $warnings $SDL_CFLAGS $GSTREAMER_CFLAGS $GTK_CFLAGS *.c -o 3dfsb $GSTREAMER_LIBS $SDL_LIBS $OTHER_LIBS $GTK_LIBS $NOPKGCONFIG_LIBS
    echo "done."
    echo "Now run the 3D File System Browser with ./3dfsb"
elif uname -s | grep -i -c "BEOS" > /dev/null; then 
    echo "BeOS detected.";
    echo "compiling...";
    gcc $SDL_LIBS $SDL_CFLAGS -I/boot/develop/tools/gnupro/include/ -I/boot/develop/headers/be/opengl/ -L/boot/home/config/lib -L/boot/develop/tools/gnupro/lib/ -lSDL_image -lGL -lglut -lmagic -D_THREAD_SAFE -O2 -x c -o 3dfsb *.c
elif uname -s | grep -i -c "BSD" > /dev/null; then 
    echo "BSD detected.";
    echo "compiling...";
    gcc $SDL_LIBS $SDL_CFLAGS -I/usr/local/include -I/usr/include/ -I/usr/X11R6/include -L/usr/lib/ -L/usr/local/lib/ -L/usr/X11R6/lib -lSDL_image -lGL -lGLU -lglut -lXmu -lXi -lXext -lX11 -lm -lmagic -D_THREAD_SAFE -O2 -x c -o 3dfsb *.c
else
    echo "Unknown OS. If you are running Linux, BeOS or";
    echo "FreeBSD please send me the output of 'uname -s'." 
    exit 1;
fi;
