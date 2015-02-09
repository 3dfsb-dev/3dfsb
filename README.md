3dfsb
=====

3D File System Browser - cleaned up and improved fork of the old tdfsb which runs on GNU/Linux and should also run on BeOS/Haiku and FreeBSD. Forked, cleaned up and improved by Tom Van Braeckel, originally written by Leander Seige.

Project homepage: https://github.com/tomvanbraeckel/3dfsb

Mailing list: 3dfsb-dev@googlegroups.com

Archive: https://groups.google.com/forum/#!forum/3dfsb-dev

News
----
- We're getting ready for [Google's Summer of Code 2015](/wiki/Google%20Summer%20of%20Code%202015.md)
- 3DFSB version 1.2 has been [released](https://github.com/tomvanbraeckel/3dfsb/releases/tag/v1.2)

Biggest improvements
--------------------
- Open any file, either in the 3D world or on your 2D desktop
- Extended audio and video support: more than 100 additional container formats and decoders are now supported through the latest GStreamer 1.4.3
- Better file identification: filetype is now determined by the contents of the file (with libmagic) with the extension of the file as a fallback
- High-resolution video previews: cranked up from the old 256x256 pixels to however high your graphics card supports (eg: 8192x8192)
- More fun: you can now zap away at your files with the lasergun tool! For your own protection, nothing is physically deleted from disk, unless y
ou explicitly configure the program to do so.
- Video input device (eg: webcam) file previews: Video4Linux (V4L2) capture devices are now visible in the 3D world and can be viewed just like your movies!
- No more waiting: fly into your directories instantly, with file previews being loaded asynchronously in the background

Performance
-----------
This version runs at 1920x1080 resolution while playing 720p H264 video (2048x2048 texture) on a single-core of the Intel Core i7 at 2.90Ghz.

Screenshots
===========
![Screenshot: all major audio and video formats are supported through GStreamer. Video texture size went up from 256x256 to however high your GPU can go. Think 8192x8192.](/screenshots/01_videos.png?raw=true "High-resolution video previews")
*Above: all major audio and video formats are supported through GStreamer. Video texture size went up from 256x256 to however high your GPU can go. Think 8192x8192.*

![Screenshot: hardware device files (such as webcams) are visible in the 3D world and can be accessed from it.](/screenshots/02_webcam.png?raw=true "Hardware device file video capture")
*Above: hardware device files (such as webcams) are visible in the 3D world and can be accessed from it.*

![Screenshot: filetype detection is much more robust and versatile, relying on libmagic to identify a filetype by its contents. The old method, which is based on filename extensions, is used as a fallback.](/screenshots/03_image.png?raw=true "Improved filetype detection")
*Above: filetype detection is much more robust and versatile, relying on libmagic to identify a filetype by its contents. The old method, which is based on filename extensions, is used as a fallback.*

![Screenshot: you can use different tools to operate on your files, for example: blast them with the laser to delete them! And don't worry: for safety reasons, the program doesn't actually delete them from your disk unless you explicitly configure it to do so.](/screenshots/04_laser_weapon_tool.png?raw=true "Laserweapon tool to delete your files")
*Above: you can use different tools to operate on your files, for example: blast them with the laser to delete them! And don't worry: for safety reasons, the program doesn't actually delete them from your disk unless you explicitly configure it to do so.*

![Screenshot: Open files and run programs in the 3D World](/screenshots/05_libreoffice_in_3D_world.png?raw=true "Open files and run programs in the 3D World")
*Above: an OpenDocument Text file is opened with its default handler (LibreOffice) in the 3D world.*


USAGE
=====

3dfsb

  or
  
3dfsb --version

  or
  
3dfsb -D /path/to/dir

  or
  
3dfsb --dir /path/to/dir

- The config file is $HOME/.3dfsb (e.g. /home/user/.3dfsb). If it does not
  exist tdsfb will generate one with all available options. this is strongly
  recommended.

- To bring up the help screen, press 'h' (will also be printed to the terminal)

- Simply walk into the spheres for cd'ing into another directory

- Select an object by looking at it or pointing at it with the crosshair

- Press F9 to cycle through the different tools that you can apply to the selected object

- Click or press Return to apply to apply the current tool to the selected object

- By default, SHIFT+TAB starts a new terminal emulator.

- You can set a custom SHIFT+TAB command in the configuration file.
  If you put one or more "%s" in the custom command, they are replaced by the currently selected object (= directory or selected file).
  For instance if you set the command to
    xmms "%s" &
  and press SHIFT+TAB from a directory, xmms will be started with the directory given as argument. 
  If you select an audio file and press SHIFT-TAB, xmms will play the file.

- If you open a video, audio or /dev/video* file in the 3D World, it will be played in the 3D World. If you open an other type of file in the 3D World, a new X session will be started in the 3D World and in that X session, xdg-open will be started to open your file. Use F12 to bind  your input (mouse, keyboard) to the in-world X server and F12 again to unbind it.

- If you open a file in the 2D desktop with the "open in 2D desktop" tool, xdg-open will be used to open your file.

	Info about xdg-open / xdg-mime:
	-------------------------------
	xdg-open uses .desktop files, which can be found in /usr/share/applications/
	Local .desktop files of the user can be found in ~/.local/share/applications/

	You can get a file's default open command with:
	xdg-mime query default $(file --mime-type -b filename)

	You can set it with:
	xdg-mime default file.desktop mime/type

	xdg-open saves these associations of the user in ~/.local/share/applications/mimeapps.list
	See also: /usr/share/applications/defaults.list

	More info: https://wiki.archlinux.org/index.php/xdg-open


Dependencies
============

Compilation libraries:
----------------------
You need imagemagick's "mogrify" tool to convert the built-in image files to pnm files and also the xxd tool.

Hint: sudo apt-get install imagemagick

Development libraries:
----------------------
You may need to install 'devel' packages of these
libraries in order to get the necessary .h files
and the sdl-config script.

- SDL: http://www.libsdl.org
- SDL_image: http://www.libsdl.org/projects/SDL_image/index.html
- OpenGL/GLU/glut: Linux/FreeBSB users can use their package manager, BeOS/Haiku users should have OpenGL (including GLU) included. For glut look at http://www.bebits.com/
- GSteamer 1.xx Core, Gstreamer 1.xx plugins: http://www.gstreamer.com/
- libmagic-dev: for mimetype identification
- libxdo-dev: for sending keystrokes
- libxtst-dev: for XTest extension (for sending keystrokes)

On Ubuntu, you can install all build-time dependencies with:

sudo apt-get install build-essential freeglut3-dev libsdl-image1.2-dev libsdl1.2-dev libxi-dev libxmu-dev libmagic-dev imagemagick

For GStreamer, you need:
sudo apt-get install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libgstreamer-plugins-bad1.0-dev

And if you want all GStreamer codecs and plugins, use:

sudo apt-get install gstreamer1.0-plugins-* gstreamer1.0-libav

(And for pulseaudio, but this is not being used currently but might be someday: sudo apt-get install gstreamer1.0-pulseaudio)

Compilation
===========

There is a small shell script which contains a standard compile string for every OS (Linux, BeOS and BSD).
So please try:

./compile.sh

Alternatively, CMake can be used to compile.
Run:

rm CMakeCache.txt	# To be sure the cache is clean, not necessary every time
cmake .
make

If the compilation was successful you can call the 3dfsb:

./3dfsb


ADDITIONAL NOTES
================

FreeBSD:

If you have problems with SDL_image try:
cd /usr/ports/graphics/sdl_image/
make install


BeOS/Haiku:

This package does not contain a BeOS binary, because I don't 
have BeOS/Haiku installed on my computer anymore.

Compile the code report any issues you may experience.

NVidia Cards under Linux (+FreeBSD?)
------------------------------------

NVidia users who have problems compiling the 3dfsb are often linking
against the wrong libraries, especially the libGLU.so.
In this case please read the instructions that came
with your drivers.
The following often helped but of course i can give no
guarantee, try it on your own risk!
    There are two libGLU.so's. Move the original libGLU.so
    to a safe place (libGLU.so.my.original or something).
    Make a link libGLU.so->libGLU.so.the.right.one.
    libGLU.so.the.right.one is often >500kByte while
    the wrong libGLU.so is much smaller.




CREDITS
=======
Maintained, improved and cleaned up by Tom Van Braeckel.

Thanks go out to:
- Leander Seige for building the original tdfsb program
- Marc Berhault for some additional code (see ChangeLog)
- Nico 'aMadMan' Toerl for extensive beta testing
- Kyle 'greenfly' Rankin for some additional code (see ChangeLog)
- Benjamin Burke for help and testing of the compile.sh
- Rafal Zawadzki for creating man pages and debian packages

History
=======
25/12/2014:
Tom Van Braeckel here; I've picked up where Leander left off.
I'm cleaning up the code and adding features, see CHANGELOG!

22/06/2007:
Leander Seige: "TDFSB is 6 years old now and I havent done anything on it for a long time.
Today I would code a lot of things very different - better i hope - than years
ago. But, so what, it still works and I got very less bug reports on 3dfsb.
Some days ago I installed Gentoo for the first time and I heared that someone
(thank you!) build a package for Gentoo so I tried emerge 3dfsb and it worked :)
Besides that it has problems with freeglut which I quickly fixed now.
So I hope some people, YOU, still like it. Have fun!"

