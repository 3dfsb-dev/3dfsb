prefix=/usr/lib/x86_64-linux-gnu/
localprefix=/usr/local/lib/

gtkstuff="-I /usr/include/gtk-3.0/ -I /usr/include/cairo -I /usr/include/pango-1.0 -I /usr/include/gdk-pixbuf-2.0 -I /usr/include/atk-1.0"

/usr/lib/gcc/x86_64-linux-gnu/4.8/cc1 -quiet -v -I /usr/include/SDL -I /usr/local/include -I /usr/include/ -I /usr/X11R6/include -I /usr/include/gstreamer-1.0 -I /usr/include/glib-2.0 -I /usr/lib/x86_64-linux-gnu/glib-2.0/include -I /usr/include/libxml2 $gtkstuff -imultiarch x86_64-linux-gnu -D_REENTRANT -D _GNU_SOURCE=1 -D _REENTRANT -D _THREAD_SAFE tdfsb.c -quiet -dumpbase tdfsb.c -mtune=generic -march=x86-64 -auxbase tdfsb -g -O2 -version -fstack-protector -Wformat -Wformat-security -o ccqdhkBS.s

as -v -I /usr/include/SDL -I /usr/local/include -I /usr/include/ -I /usr/X11R6/include -I /usr/include/gstreamer-1.0 -I /usr/include/glib-2.0 -I /usr/lib/x86_64-linux-gnu/glib-2.0/include -I /usr/include/libxml2 --64 -o ccUyg6CF.o ccqdhkBS.s

/usr/lib/gcc/x86_64-linux-gnu/4.8/collect2 --sysroot=/ --build-id --eh-frame-hdr -m elf_x86_64 --hash-style=gnu --as-needed -dynamic-linker /lib64/ld-linux-x86-64.so.2 -z relro -o tdfsb /usr/lib/gcc/x86_64-linux-gnu/4.8/../../../x86_64-linux-gnu/crt1.o /usr/lib/gcc/x86_64-linux-gnu/4.8/../../../x86_64-linux-gnu/crti.o /usr/lib/gcc/x86_64-linux-gnu/4.8/crtbegin.o -L/usr/lib/x86_64-linux-gnu -L/usr/lib/ -L/usr/local/lib/ -L/usr/X11R6/lib -L/usr/lib/gcc/x86_64-linux-gnu/4.8 -L/usr/lib/gcc/x86_64-linux-gnu/4.8/../../../x86_64-linux-gnu -L/usr/lib/gcc/x86_64-linux-gnu/4.8/../../../../lib -L/lib/x86_64-linux-gnu -L/lib/../lib -L/usr/lib/x86_64-linux-gnu -L/usr/lib/../lib -L/usr/lib/gcc/x86_64-linux-gnu/4.8/../../.. /usr/lib/x86_64-linux-gnu/libX11.so /usr/lib/x86_64-linux-gnu//libsmpeg-0.4.so.0.1.4 ccUyg6CF.o -lgcc --as-needed -lgcc_s --no-as-needed -lpthread -lc -lgcc --as-needed -lgcc_s --no-as-needed /usr/lib/gcc/x86_64-linux-gnu/4.8/crtend.o /usr/lib/gcc/x86_64-linux-gnu/4.8/../../../x86_64-linux-gnu/crtn.o /usr/lib/x86_64-linux-gnu/libX11.so /usr/lib/x86_64-linux-gnu/libSDL.so $prefix/libgstreamer-1.0.so $prefix/libgobject-2.0.so /usr/lib/x86_64-linux-gnu/libc.so $localprefix/gstreamer-1.0/libgstopengl.so $localprefix/libgstgl-1.0.so.0 $localprefix/libgstvideo-1.0.so.0 $prefix/libgdk_pixbuf-2.0.so -lm -lSDL -lSDL_image -lGL -lGLU -lglut -lXmu -lXi -lXext -lX11 -lsmpeg -lgstreamer-1.0 -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lxml2 -lglib-2.0 2>&1
