
sudo apt-get install gstreamer0.10-plugins-*

How does video get displayed?

- When you select it by clicking, nothing happens except that it gets colored
- When you play it by pressing enter, speckey(key=13) enter handling is called which does the setup (and calls SMPEG_play())
- Then, display() is called continuously, and does play_mpeg() because TDFSB_MPEG_FILE is set, which sets the correct frame

So we have to somehow tell GStreamer to render the video on the SDL_Surface; somehow do the equivalent of this:
	SMPEG *TDFSB_MPEG_HANDLE = NULL;
	SDL_Surface *TDFSB_MPEG_SURFACE = NULL;
	SMPEG_setdisplay(TDFSB_MPEG_HANDLE, TDFSB_MPEG_SURFACE, 0, 0);

Options: http://gstreamer-devel.966125.n4.nabble.com/video-sink-to-update-OpenGL-texture-td972959.html

- Use fakesink, get the data out of it with http://gstreamer.freedesktop.org/data/doc/gstreamer/head/gstreamer-plugins/html/gstreamer-plugins-fakesink.html#GstFakeSink-handoff
- With appsink, which gives you the raw decoded data to do something with it:
http://gstreamer.freedesktop.org/data/doc/gstreamer/head/manual/html/section-data-spoof.html
=> good example where they get a PNG image out of a video

- Use http://gstreamer.freedesktop.org/data/doc/gstreamer/head/gst-plugins-base-plugins/html/gst-plugins-base-plugins-uridecodebin.html
like http://stackoverflow.com/questions/7594516/gstreamer-video-to-opengl-texture
which should give you a theGStPixelBuffer somehow...

- The 3 (glfilterapp),
- and 4 (create a glfilter) are already fully implemented.
	=> but not flexible: https://github.com/ystreet/gst-plugins-gl/blob/master/tests/examples/generic/recordgraphic/main.cpp
	then in the drawcallback you have to call src = glGetCurrentContext(), then glCopyContext(src, dst, ...),  just one time.
	You can also use dst = glGetCurrentContext()  in your gl framework thread, to get the gestination.
	glXCopyContext(disp, glcontextSrc, glcontextDst, GL_TEXTURE_BIT);
- The 1 (copyContext), we have to test it.
- The 2, (use the same context) 
	get the gl context of the gst pipeline and then copy it (just once, right after the pipeline initialization if I'm not mistaken) to the gl context of my app using wglCopyContext/glXCopyContext. 

Examples of GStreamer and OpenGL:
http://cgit.freedesktop.org/gstreamer/gst-plugins-bad/tree/tests/examples/gl

Example of GStreamer, SDL and OpenGL:
http://cgit.freedesktop.org/gstreamer/gst-plugins-bad/tree/tests/examples/gl/sdl/sdlshare.c
=> 1.0 or more recent material
=> Finally works! Good example!

Simple example:
gst-launch-1.0 -v videotestsrc ! glfiltercube ! glimagesink

gst-launch-1.0 uridecodebin uri='file:///home/tom/Stanley Kubrick - Barry Lyndon (1975).avi' ! videoconvert ! videoscale ! ximagesink 
=> this works, but not in the code...
I now got the example to work, it's fine, so I suspect a bug in my code...

Documentation:
file:///home/tom/sources/gstreamer/docs/gst/html/index.html


(gdb) c
Continuing.

Breakpoint 3, speckey (key=13) at tdfsb.c:3825
3825				if (TDFSB_OBJECT_SELECTED) {
(gdb) bt
#0  speckey (key=13) at tdfsb.c:3825
#1  0x0000000000403b38 in main (argc=<optimized out>, argv=0x7fffffffe5a8) at tdfsb.c:4525
(gdb) c
Continuing.
MPEG Start /home/tvb/big.mpg
[New Thread 0x7fffec68e700 (LWP 6226)]
[New Thread 0x7fffebe8d700 (LWP 6227)]
[New Thread 0x7fffeb68c700 (LWP 6228)]

Breakpoint 4, play_mpeg () at tdfsb.c:383
383	{
(gdb) bt
#0  play_mpeg () at tdfsb.c:383
#1  0x0000000000407e28 in display () at tdfsb.c:2707
#2  0x0000000000403a46 in main (argc=<optimized out>, argv=0x7fffffffe5a8) at tdfsb.c:4489
(gdb) l
378		SDL_Quit();
379		exit(code);
380	}
381	
382	void play_mpeg()
383	{
384		SMPEG_getinfo(TDFSB_MPEG_HANDLE, &TDFSB_MPEG_INFO);
385		if (TDFSB_MPEG_FRAMENO != TDFSB_MPEG_INFO.current_frame) {
386			TDFSB_MPEG_FRAMENO = TDFSB_MPEG_INFO.current_frame;
387			SDL_LockSurface(TDFSB_MPEG_SURFACE);
(gdb) s
384		SMPEG_getinfo(TDFSB_MPEG_HANDLE, &TDFSB_MPEG_INFO);
(gdb) display TDFSB_MPEG_INFO
1: TDFSB_MPEG_INFO = {has_audio = 1, has_video = 1, width = 352, height = 240, current_frame = 0, current_fps = 0, 
  audio_string = "MPEG-1 Layer 2 224kbit/s 44100Hz stereo", '\000' <repeats 40 times>, audio_current_frame = 0, current_offset = 6149, 
  total_size = 103606244, current_time = 0, total_time = 597.20327777777777}
(gdb) display TDFSB_MPEG_FRAMENO

