We're pleased to announce that we'll be applying for [Google's Summer of Code 2015](http://www.google-melange.com/gsoc/homepage/google/gsoc2015).

Important information can be found in Google's [FAQs](http://www.google-melange.com/gsoc/events/google/gsoc2015) and [Timeline](http://www.google-melange.com/gsoc/events/google/gsoc2015). 

Idea list for GSoC 2015
=======================
The list of ideas below is to be used for Google Summer of Code 2015.

Project: move to open source 3D Engine
--------------------------------------
**Brief explanation:** currently, 3DFSB does not rely on any existing 3D Engine, and uses raw OpenGL calls for all its drawing operations. This project will entail integrating 3DFSB with an existing, powerful, free open source 3D engine for all its visual operations.

**Full explanation:** currently, 3DFSB does not rely on any existing 3D Engine. It uses raw OpenGL calls to do all drawing. While this has its benefits (such as small binary size and few external dependencies), it also has a big downside because it means that adding a new visualization, tool, or other visual effect takes a lot of time. Doing a good "laser beam" effect, for instance, is hard without an engine. Adding smoke, water, and reflection effects are also time consuming to implement. Perhaps most importantly, having a 3D object for 3DFSB created by a 3D modelling artist with a 3D modelling tool (such as blender) is currently impossible. So in short, everything we do will take a lot more time and will probably be inferior to how it would be done in a real, specialized 3D engine.

**Mentor:** Tom Van Braeckel

**Used technologies:** C, Blender, an open source 3D engine

**Expected results:** a new major version of 3DFSB that uses a free open source 3D engine for all its drawing operations and imports a few externally modelled 3D objects from a 3D modelling tool such as Blender.


Project: add virtual reality headset support
--------------------------------------------
**Brief explanation:** to enhance the experience of interacting with your files in the 3D world, we will support several virtual reality headsets so that you can truly "walk" between your files and interact with them. Preferably, most details of the headset will be handled transparently by the 3D engine, so that most of the work that needs to be done will be tweaking and testing.

**Mentor:** Tom Van Braeckel

**Used technologies:** C, various virtual reality headsets

**Expected results:** a new version of 3DFSB that supports the Oculus Rift, Samsung VR and Cardboard VR.

**Bonus:** a press release article that announces the VR headset support, to be submitted to a major virtual reality news magazine.


Project: add cool and fast navigation
-------------------------------------
**Brief explanation:** in a folder with more than 10 files and folders, it becomes difficult to quickly find files and folders, even when they are sorted alphabetically. To make this easier for the users, we will add "flying, incremental search" functionality that allows you to quickly filter out files and fly to the first match that is found.

**Details:**
- The user will type slash ('/') to start searching
- The user will fly to the first match
- The matches will become bigger and the non-matches smaller, until the non-matches are gone

**Mentor:** Tom Van Braeckel

**Used technologies:** C

**Expected results:** a new version of 3DFSB that has the new cool and fast navigation feature implemented.

Project: add support for the Windows operating system
-----------------------------------------------------
**Brief explanation:** A Windows-compatible version of the 3D File System Browser would make it available to a lot more users. All technologies used by 3DFSB are compatible with Microsoft Windows already or have Windows alternatives that can be used in place.

**Mentor:** Tom Van Braeckel

**Used technologies:** C, Visual Studio

**Expected results:** a new major version of 3DFSB that can be compiled for and installed on Microsoft Windows.

**Bonus:** a press release article that announces the Windows support, to be submitted to a major news magazine, targeted at Windows users.

Project: plugin system for more tools
-------------------------------------
**Brief explanation:** Currently, there is a limited set of tools that we support for the users to operate on files and folders. We would like to encourage the creation of new tools by making a plugin-like system. This system will make the code more modular and will make it easier for developers to create additional tools to be used in 3DFSB.

**Mentor:** Tom Van Braeckel

**Used technologies:** C, Visual Studio

**Expected results:** a new major version of 3DFSB in which the existing tools are simple plugins.

**Bonus:** adding one additional tool that proves that the plugin system is really easy to use, for example, a "Compress Directory" tool.


Small, independent tasks
------------------------
These are great for getting familiar with the code during the "warming up" phase of Google's Summer of Code.

- Create a 3D model of a tool that is used in the 3D World in 3D modelling software, such as Blender, and import it in 3DFSB. This model can then be used as a proof-of-concept and a test to show that we can easily import 3D models made by visual artists.
- Upgrade to SDL 2.0 with FULLSCREEN_DESKTOP so we don't have to change resolution when going to fullscreen anymore
- Remove the old, unsupported "classic mode" navigation from the source code
- Show default program icons instead of hard-coded icons if no preview texture is available:
    - The .desktop files have Icon=<name> and this refers to ~/.local/share/icons/ and /usr/share/icons/
    - Example: /usr/share/icons/hicolor/256x256/apps/evince.png
    - More info: http://standards.freedesktop.org/icon-theme-spec/icon-theme-spec-latest.html
- Cleanup the code by removing the global variables and replacing them with simple programming patterns that keep variables local
- Add a nice slashscreen
- Add screen-capture-to-videofile capability
- Add "demo mode" for screensavers that automatically and randomly browses through your files (stays in one folder and below)
- Add a more clear "press h to hide this help text" and "edit the file ~/.3dfsb to change your settings"
- Create (and release) debian packages
- Create (and release) readme as a man page
- Improve the approach action for textured objects (video, images, /dev/video*, files opened in the 3D world,...); move exactly to the position and orientation where the texture fills the entire screen perfectly. Test for different texture sizes and screen resolutions.
- Add a CMake install target with a .desktop file like this one: https://aur.archlinux.org/packages/3dfsb/
- Don't reset the key repeat state upon entering a new directory so that, if you were pressing the forward key and entered a directory, you keep moving forward, even after entering the new directory.

