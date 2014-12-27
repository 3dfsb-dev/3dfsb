prefix=/usr/lib/x86_64-linux-gnu/
















#gcc -v -I/usr/include/gstreamer-1.0/ -I/usr/include/glib-2.0 sdlshare.c -I/usr/./lib/x86_64-linux-gnu/glib-2.0/include/  -o sdlshare -lX11

/usr/lib/gcc/x86_64-linux-gnu/4.9/cc1 -quiet -v -I /usr/include/gstreamer-1.0/ -I /usr/include/glib-2.0 -I /usr/./lib/x86_64-linux-gnu/glib-2.0/include/ -imultiarch x86_64-linux-gnu sdlshare.c -quiet -dumpbase sdlshare.orig.c -mtune=generic -march=x86-64 -auxbase sdlshare.orig -version -fstack-protector-strong -Wformat -Wformat-security -o /tmp/ccaTviib.s

as -v -I /usr/include/gstreamer-1.0/ -I /usr/include/glib-2.0 -I /usr/./lib/x86_64-linux-gnu/glib-2.0/include/ --64 -o /tmp/cc9dJJeE.o /tmp/ccaTviib.s

/usr/lib/gcc/x86_64-linux-gnu/4.9/collect2 -plugin /usr/lib/gcc/x86_64-linux-gnu/4.9/liblto_plugin.so -plugin-opt=/usr/lib/gcc/x86_64-linux-gnu/4.9/lto-wrapper -plugin-opt=-fresolution=/tmp/ccpfN6b7.res -plugin-opt=-pass-through=-lgcc -plugin-opt=-pass-through=-lgcc_s -plugin-opt=-pass-through=-lc -plugin-opt=-pass-through=-lgcc -plugin-opt=-pass-through=-lgcc_s --sysroot=/ --build-id --eh-frame-hdr -m elf_x86_64 --hash-style=gnu --as-needed -dynamic-linker /lib64/ld-linux-x86-64.so.2 -z relro -o sdlshare /usr/lib/gcc/x86_64-linux-gnu/4.9/../../../x86_64-linux-gnu/crt1.o /usr/lib/gcc/x86_64-linux-gnu/4.9/../../../x86_64-linux-gnu/crti.o /usr/lib/gcc/x86_64-linux-gnu/4.9/crtbegin.o -L/usr/lib/gcc/x86_64-linux-gnu/4.9 -L/usr/lib/gcc/x86_64-linux-gnu/4.9/../../../x86_64-linux-gnu -L/usr/lib/gcc/x86_64-linux-gnu/4.9/../../../../lib -L/lib/x86_64-linux-gnu -L/lib/../lib -L/usr/lib/x86_64-linux-gnu -L/usr/lib/../lib -L/usr/lib/gcc/x86_64-linux-gnu/4.9/../../.. /tmp/cc9dJJeE.o -lX11 -lgcc --as-needed -lgcc_s --no-as-needed -lc -lgcc --as-needed -lgcc_s --no-as-needed /usr/lib/gcc/x86_64-linux-gnu/4.9/crtend.o /usr/lib/gcc/x86_64-linux-gnu/4.9/../../../x86_64-linux-gnu/crtn.o /usr/lib/x86_64-linux-gnu/libX11.so /usr/lib/x86_64-linux-gnu/libSDL.so $prefix/libgstreamer-1.0.so.0.403.0 $prefix/libgobject-2.0.so /usr/lib/x86_64-linux-gnu/libc.so $prefix/libgstvideo-1.0.so.0.403.0 $prefix/libgstgl-1.0.so.0.403.0 -lm -lSDL -lSDL_image -lGL -lGLU -lglut -lXmu -lXi -lXext -lX11 -lsmpeg -lgstreamer-1.0 -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lxml2 -lglib-2.0

