3dfsb
=====

3D File System Browser - cleaned up and improved fork of the old tdfsb

This runs on GNU/Linux, and should also run on BeOS/Haiku and FreeBSD.

Homepage: https://github.com/tomvanbraeckel/3dfsb

Forked, cleaned up and improved by Tom Van Braeckel, originally written by Leander Seige.

Version 1.0 (07-01-2015)
************************

Major
-----
- More A/V formats: More than 100 audio and video formats now supported (through GStreamer 1.4.3)
- Better file identification: filetype is now determined by the contents of the file (libmagic) instead of the file extension
- High-resolution videos: up from the old 256x256 pixels to the maximum your graphics card can handle (eg:8192x8192)
- You can now zap away at your files with the lasergun tool! Nothing is deleted from disk, for your own protection.
- Video input device (eg: webcam) file previews: these are now visible in the 3D world, just like your movies!

Minor
-----
- Faster navigation by default
- Visible on-screen info by default
- Uncapped framerate by default
- Uncapped texture size by default
- Higher rendering resolution by default
- Lots and lots of code cleanups and comments added
- No more dependency on libsmpeg
- New maintainer (Tom Van Braeckel)

Performance:
------------
This version runs at 1920x1080 resolution while playing 720p H264 video (2048x2048 texture) on a single-core of the Intel Core i7 at 2.90Ghz.




USAGE
=====

3dfsb

  or
  
3dfsb --version

  or
  
3dfsb -D /path/to/dir

  or
  
3dfsb --dir /path/to/dir

- the config file is $HOME/.3dfsb (e.g. /home/user/.3dfsb). if it does not
  exist tdsfb will generate one with all available options. this is strongly
  recommended.

- if you start the 3dfsb the first time, press 'h' for the help menu
  (will also be printed to the terminal)

- simply walk into the spheres for cd'ing into another directory

- select an object by pointing at it with the crosshair and press the left
  mouse button. or hold the left mouse button and press any key to select
  the first object that begins with that character (case sensitive).
    -   while an object is selected press the right mouse button simultaneously to
        automatic approach the object
        [unfortunately doesn't work on BeOS/Haiku as well as resizing the window, afaik
        these are issues of the SDL implementation, use the right CTRL for now].
    -   if an mp3 or mpeg1 video file is selected press the RETURN key to start
        the playback

- some of the displays are not visible while the help display is active

- it is possible to execute a custom command from within 3dfsb. configure
  your command in the config file! the command can be called by pressing
  the tabulator key. put a "%s" in the command line, this will be replaced
  by the current directory or selected file.
  for instance if you set the command to
    xmms "%s" &
  and press tab from a directory, xmms will be started with the directory
  given as argument. 
  if you select an audio file and press the tab key xmms will play the file.
  the default command is
    cd "%s"; xterm&
  it will open a xterm in the current directory.
  this command is always present by pressing shift+tab!
  so you have two commands: 
  - one by configuring your custom command for the tab key
  - one by pressing shift+tab, it will execute the built in xterm call  
  but dont forget the quotation marks!
  you are free to not add "%s" to your custom command if you just
  want to launch any program. but if you do so, you can customize
  3dfsb for your needs, choose a certain kind of files and than launch
  emacs for editing text files, mplayer for playing avis or whatever...

Here is a print out of the default keyboard settings.
You may change these by editing ~/.3dfsb

    Esc           quit   F1/F2    speed +/-
    Mouse move    look   F3/F4      rot +/-
    UP         forward   F5/F6  ball detail
    DOWN      backward   HOME     start pos
    L/R     step aside   LMB  select object
    END    ground zero   +RMB|CTRL appr.obj
    F7/F8  max fps +/-   +ENTER ply mpg/mp3
    F9     change tool (selector, weapon)
    "t"      filenames   "g"   ground cross
    "c"      crosshair   "d"        display
    "."      dot files   "p"      print FPS
    "r" rel./get mouse   "f"     fullscreen
    "l"     reload dir   "b"   image bricks
    "u"           cd..   "a"      alphasort
    "m"        shading   " "         flying
    "h"      show help   "i"  print GL info
    "0"      jump home   "o"    classic nav
    "s"    save config   "#"   fps throttle
     
    "1|3|q|e"            Up|Down|Left|Right
    "2|w"                  Forward|Backward
    PgUp/Down or MMB+Mouse move up/downward



Dependencies
============

Note: this section needs a cleanup!

Needed Libraries:

You may need to install 'devel' packages of these
libraries in order to get the necessary .h files
and the sdl-config script.

- SDL: http://www.libsdl.org
- SDL_image: http://www.libsdl.org/projects/SDL_image/index.html
- OpenGL/GLU/glut: Linux/FreeBSB users can use their package manager, BeOS/Haiku users should have OpenGL (including GLU) included. For glut look at http://www.bebits.com/
- GSteamer 1.xx Core, Gstreamer 1.xx plugins: http://www.gstreamer.com/
- libmagic-dev: for mimetype identification

On Ubuntu, you can install all build-time dependencies with:

sudo apt-get install build-essential freeglut3-dev libsdl-image1.2-dev libsdl1.2-dev libsmpeg-dev libxi-dev libxmu-dev

This list of dependencies was made using: apt-rdepends --build-depends --follow=DEPENDS 3dfsb

+ for GStreamer, you need:
sudo apt-get install libgstreamer0.10-dev libgstreamer-plugins-base0.10-dev libgstreamer-plugins-bad0.10-dev

and also perhaps:

sudo apt-get install gstreamer0.10-plugins-*
sudo apt-get install gstinterfaces

and for pulseaudio:

sudo apt-get install gstreamer1.0-pulseaudio

Compilation
===========

There is a small shell script which contains a standard compile string for every OS (Linux, BeOS and BSD). So please try:

./compile.sh

If something goes wrong error messages will appear. If
you don't know how to solve them don't hesitate and
report the error messages to me.

If ./compile.sh was successful you can call the 3dfsb:

./3dfsb

or copy the 3dfsb binary to your path, e.g. /usr/bin/

Once you have a working 3dfsb binary you may try to
edit the compile.sh for a more optimized version.
You could change the -O2 argument (opimization) to a
higher value (e.g. -O6) or add machine specific
optimizations (e.g.  -march=i686 -mcpu=i686).
Read 'man gcc' for additional arguments.


ADDITIONAL NOTES
================


FreeBSD:

If you have problems with SDL_image try:
cd /usr/ports/graphics/sdl_image/
make install


BeOS/Haiku:

This package does not contain a BeOS binary, because I don't 
have BeOS/Haiku installed on my computer anymore.

The source should compile anyway.

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
Maintained, improved and cleaned up by Tom Van Braeckel

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

