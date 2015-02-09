We're pleased to announce that we'll be applying for [Google's Summer of Code 2015](http://www.google-melange.com/gsoc/homepage/google/gsoc2015).

Important information can be found in Google's [FAQs](http://www.google-melange.com/gsoc/events/google/gsoc2015) and [Timeline](http://www.google-melange.com/gsoc/events/google/gsoc2015). 

Idea list for GSoC 2015
=======================
The list of ideas below is to be used for Google Summer of Code 2015.

Open source 3D Engine
---------------------
Goal: Switching to an open source 3D engine for faster development and better graphics (+ Blender integration?)

Virtual Reality headset support
-------------------------------
- Goal: supporting Oculus Rift, Samsung VR and Cardboard VR
- Preferably handled by the 3D engine quite transparently, so this becomes more testing

Faster search for files/folders
-------------------------------
- With slash / to start searching
- You'll fly a path between the objects to the first match
- The matches will become bigger and the non-matches smaller, until the non-matches are gone

Smaller, independent tasks, great for getting familiar with the code
--------------------------------------------------------------------
- Faster search for files/folders
- Upgrade to SDL 2.0 with FULLSCREEN_DESKTOP so we don't have to change resolution when going to fullscreen anymore
- do away with classic mode
- Show default program icons instead of hard-coded icons if no preview texture is available:
	The .desktop files have Icon=<name> and this refers to ~/.local/share/icons/ and /usr/share/icons/
	Example: /usr/share/icons/hicolor/256x256/apps/evince.png
	More info: http://standards.freedesktop.org/icon-theme-spec/icon-theme-spec-latest.html
- Cleanup the code by removing the global variables and replacing them with simple programming patterns that keep variables local
- Add a nice slashscreen
- Add screen-capture-to-videofile capability
- Add "demo mode" for screensavers that automatically and randomly browses through your files (stays in one folder and below)
- Add a more clear "press h to hide this help text" and "edit the file ~/.3dfsb to change your settings"
- Create (and release) debian packages
- Create (and release) readme as a man page
- improve approach: move exactly close enough to the video so that it fills the whole screen perfectly => can be reused for other objects,
just depends on the width of the image, actually... but test it at different resolutions!
- Add a CMake install target with a .desktop file like this one: https://aur.archlinux.org/packages/3dfsb/
- Don't reset the key (repeat?) upon entering a new directory - I couldn't get this to work right away...


