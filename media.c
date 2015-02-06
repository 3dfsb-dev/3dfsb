#include <stdio.h>
#include <unistd.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#include <gst/gst.h>
#pragma GCC diagnostic pop

#include "3dfsb.h"
#include "media.h"
#include "resources/xorg.conf.h"
#include "tools.h"

#ifndef _GLUfuncptr
#define _GLUfuncptr void*
#endif

#define CAPS "video/x-raw,format=RGB"

// TurboVNC does strange things to the SHIFT key sequences
//#define STARTX "/opt/TurboVNC/bin/vncserver -geometry 1920x1080 &"

// setxkbmap is needed, otherwise the X server will have a us layout, which has different keys...
#define STARTX "Xorg -noreset +extension GLX +extension RANDR +extension RENDER -logfile /tmp/Xorg.log.1 -config /tmp/xorg.conf :1 & sleep 1; DISPLAY=:1 /etc/X11/Xsession & sleep 1; DISPLAY=:1 setxkbmap %s"	// The %s gets replaced by the correct xkbmap

//#define STOPX "/opt/TurboVNC/bin/vncserver -kill :1 &"
// killall Xorg != kill ($pidof Xorg)
// TODO: save the new Xorg's PID and kill only that one
#define STOPX "kill $(pidof Xorg)"

// Quite ugly, but we need the keyboard map to run setxkbmap after STARTX
#define	GETXKBMAP	"DISPLAY=:0 setxkbmap -query | grep layout | cut -c 13-15"

/* GStreamer stuff */
GstPipeline *pipeline = NULL;

// Used to limit the number of video texture changes for performance
unsigned int framecounter;
unsigned int displayedframenumber;

// GstBuffer with the new frame
GstBuffer *videobuffer;

/* Create a new xorg.conf file,
 * which is needed to start a new X server
 */
static void create_xorg_conf(void) {
	FILE *fptr;
	fptr=fopen("/tmp/xorg.conf","wb");
	fwrite(resources_xorg_conf, 1, resources_xorg_conf_len, fptr);
	fclose(fptr);
}

static void startx(void) {
	create_xorg_conf();
	char *keymap = execute_binary(GETXKBMAP);
	char torun[4096];
	snprintf(torun, 4096, STARTX, keymap);
	printf("Starting new X session with: %s\n", torun);
	system(torun);
	sleep(2);
}

/* fakesink handoff callback */
static void on_gst_buffer(GstElement * fakesink, GstBuffer * buf, GstPad * pad, gpointer data)
{
	UNUSED(fakesink);
	UNUSED(pad);
	UNUSED(data);

	framecounter++;
	videobuffer = buf;
}

void cleanup_media_player(void)
{
	if (pipeline && GST_IS_ELEMENT(pipeline)) {
		printf("Cleaning up GStreamer pipeline\n");
		gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_NULL);
		gst_object_unref(pipeline);
	}
	// Kill any running X session
	system(STOPX);		// When we do this synchronously, we get killed (for stalling?)
	TDFSB_MEDIA_FILE = NULL;
}

void update_media_texture(tree_entry * media_tree_entry)
{
	// Ensure we don't refresh the texture if nothing changed
	if (framecounter == displayedframenumber) {
		// printf("Already displaying frame %d, skipping...\n", framecounter);
		return;
	}
	displayedframenumber = framecounter;
	GstMapInfo map;

	if (!videobuffer)
		return;

	gst_buffer_map(videobuffer, &map, GST_MAP_READ);
	if (map.data == NULL)
		return;		// No video frame received yet

	// now map.data points to the video frame that we saved in on_gst_buffer()
	glBindTexture(GL_TEXTURE_2D, media_tree_entry->textureid);

	glTexImage2D(GL_TEXTURE_2D, 0, (GLenum) media_tree_entry->textureformat, media_tree_entry->texturewidth, media_tree_entry->textureheight, 0, (GLenum) media_tree_entry->textureformat, GL_UNSIGNED_BYTE, map.data);

	// Free up memory again
	gst_buffer_unmap(videobuffer, &map);

	// Mark this buffer as cleared, otherwise this function could be re-entered
	videobuffer = NULL;
}

static void putpixel(SDL_Surface * surface, int x, int y, Uint32 pixel)
{
	int surface_bpp = surface->format->BytesPerPixel;
	/* Here p is the address to the pixel we want to set */
	Uint8 *p = (Uint8 *) surface->pixels + y * surface->pitch + x * surface_bpp;

	switch (surface_bpp) {
	case 1:
		*p = pixel;
		break;

	case 2:
		*(Uint16 *) p = pixel;
		break;

	case 3:
		if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
			p[0] = (pixel >> 16) & 0xff;
			p[1] = (pixel >> 8) & 0xff;
			p[2] = pixel & 0xff;
		} else {
			p[0] = pixel & 0xff;
			p[1] = (pixel >> 8) & 0xff;
			p[2] = (pixel >> 16) & 0xff;
		}
		break;

	case 4:
		*(Uint32 *) p = pixel;
		break;
	}
}

static Uint32 getpixel(SDL_Surface * surface, int x, int y)
{
	int surface_bpp = surface->format->BytesPerPixel;
	/* Here p is the address to the pixel we want to retrieve */
	Uint8 *p = (Uint8 *) surface->pixels + y * surface->pitch + x * surface_bpp;

	switch (surface_bpp) {
	case 1:
		return *p;
		break;

	case 2:
		return *(Uint16 *) p;
		break;

	case 3:
		if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
			return p[0] << 16 | p[1] << 8 | p[2];
		else
			return p[0] | p[1] << 8 | p[2] << 16;
		break;

	case 4:
		return *(Uint32 *) p;
		break;

	default:
		return 0;	/* shouldn't happen, but avoids warnings */
	}
}

// Here we do that SDL_BlitSurface of SDL 2.0 does
SDL_Surface *ScaleSurface(SDL_Surface * Surface, double Width, double Height)
{
	int x, y, o_y, o_x;

	if (!Surface || (Width < 0.1) || (Height < 0.1))
		return 0;

	SDL_Surface *_ret = SDL_CreateRGBSurface(Surface->flags, Width, Height, Surface->format->BitsPerPixel, Surface->format->Rmask, Surface->format->Gmask, Surface->format->Bmask, Surface->format->Amask);
	/* SDL_Surface *_ret = SDL_CreateRGBSurface(SDL_SWSURFACE, Width, Height, 32,
	   SDL_Surface *_ret = SDL_CreateRGBSurface(SDL_SWSURFACE, Width, Height, 24,
	   #if SDL_BYTEORDER == SDL_LIL_ENDIAN
	   0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000
	   #else
	   0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF
	   #endif
	   ); */

	double _stretch_factor_x = Width / Surface->w;
	double _stretch_factor_y = Height / Surface->h;
	printf("ScaleSurface has stretch factors %f and %f\n", _stretch_factor_x, _stretch_factor_y);

	for (y = 0; y < Surface->h; y++)
		for (x = 0; x < Surface->w; x++)
			for (o_y = 0; o_y < _stretch_factor_y; ++o_y)
				for (o_x = 0; o_x < _stretch_factor_x; ++o_x)
					putpixel(_ret, _stretch_factor_x * x + o_x, _stretch_factor_y * y + o_y, getpixel(Surface, x, y));

	return _ret;
}

/* Get an image from a (video, videosource or image) file.
 *
 * First we create a pipeline, then get a sample, then get the buffer from it,
 * then map the buffer to a "loader" SDL_Surface, and then scale that into the "converter_to_return" SDL_Surface.
 *
 * Note: this uses around 300MB of memory for each high resolution image... we need to clear it up!
 *
 * @returns NULL when there is a problem to get any image from the file.
 */
texture_description *get_image_from_file(char *filename, unsigned int filetype, unsigned int TDFSB_MAX_TEX_SIZE)
{
	GstElement *sink;
	gint width, height;
	GstSample *sample;
	gchar *descr;
	gint64 duration, position;
	GstStateChangeReturn ret;
	gboolean res;
	GstMapInfo map;
	GError *error = NULL;

	// Return values:
	texture_description *toreturn = NULL;
	SDL_Surface *converter_to_return = NULL;
	unsigned long www = 0;
	unsigned long hhh = 0;
	unsigned long p2w = 0;
	unsigned long p2h = 0;

	if (filetype != VIDEOFILE && filetype != VIDEOSOURCEFILE && filetype != IMAGEFILE && filetype != PROCESS) {
		printf("Error: get_image_from_file can only handle VIDEOFILE, VIDEOSOURCFILE, PROCESS and IMAGEFILE's!\n");
		goto err_out;
	}
	// create a new pipeline
	gchar *uri = gst_filename_to_uri(filename, &error);
	if (error != NULL) {
		g_print("Could not convert filename %s to URI: %s\n", filename, error->message);
		g_error_free(error);
		exit(1);
	}

	if (filetype == VIDEOFILE || filetype == IMAGEFILE) {
		descr = g_strdup_printf("uridecodebin uri=%s ! videoconvert ! videoscale ! appsink name=sink caps=\"" CAPS "\"", uri);
	} else if (filetype == VIDEOSOURCEFILE) {
		descr = g_strdup_printf("v4l2src device=%s ! videoconvert ! videoscale ! appsink name=sink caps=\"" CAPS "\"", filename);
		// Idea for speedup: set queue-size to 1 instead of 2
	} else if (filetype == PROCESS) {
		// Get PID from path name
		// Get Window ID(s) for this PID
		//int pid = 5288;
		int windowid = 37748743;	// used system("xdotool search --pid 5288\n");
		descr = g_strdup_printf("ximagesrc xid=%d ! videoconvert ! videoscale ! appsink name=sink sync=false caps=\"" CAPS "\"", windowid);

		// Experiment with this?
		//system("xdotool set_window  --overrideredirect 0 37748743");

		// Works:
		//system("xdotool windowraise 37748743\n"); sleep(1);   // Wait until the window is raised, otherwise we might end up with a gray screen

		// Changes focus, but does not redraw:
		//system("xdotool windowfocus 37748743\n"); sleep(0.5); // Wait until the window is raised, otherwise we might end up with a gray screen
		// Works, and is faster because we have --sync and therefore don't need sleep()
		// BUT this may change the active desktop... perhaps fix this with get_desktop and set_desktop?
		system("xdotool windowactivate --sync 37748743\n");
	}
	printf("gst-launch-1.0 %s\n", descr);
	pipeline = (GstPipeline *) (gst_parse_launch(descr, &error));

	if (error != NULL) {
		g_print("could not construct pipeline: %s\n", error->message);
		g_error_free(error);
		//exit(-1);
	}

	/* get sink */
	sink = gst_bin_get_by_name(GST_BIN(pipeline), "sink");

	/* set to PAUSED to make the first frame arrive in the sink */
	ret = gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PAUSED);
	switch (ret) {
	case GST_STATE_CHANGE_FAILURE:
		g_print("Failed to pause the file\n");
		//exit(-1);
	case GST_STATE_CHANGE_NO_PREROLL:
		// for live sources, we need to set the pipeline to PLAYING before we can receive a buffer
		gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING);
	default:
		break;
	}
	/* This can block for up to 5 seconds. If your machine is really overloaded,
	 * it might time out before the pipeline prerolled and we generate an error. A
	 * better way is to run a mainloop and catch errors there. */
	ret = gst_element_get_state(GST_ELEMENT(pipeline), NULL, NULL, 5 * GST_SECOND);
	if (ret == GST_STATE_CHANGE_FAILURE) {
		g_print("Failed to get the pipeline state\n");
		//exit(-1);
	}

	if (filetype == VIDEOFILE) {	// VIDEOSOURCEFILE, IMAGEFILE and PROCESS cannot be seeked
		/* get the duration */
		gst_element_query_duration(GST_ELEMENT(pipeline), GST_FORMAT_TIME, &duration);

		if (duration != -1)
			/* we have a duration, seek to 5% */
			position = duration * 5 / 100;
		else
			/* no duration, seek to 1 second, this could EOS */
			position = 1 * GST_SECOND;

		/* seek to the a position in the file. Most files have a black first frame so
		 * by seeking to somewhere else we have a bigger chance of getting something
		 * more interesting. An optimisation would be to detect black images and then
		 * seek a little more */
		gst_element_seek_simple(GST_ELEMENT(pipeline), GST_FORMAT_TIME, GST_SEEK_FLAG_KEY_UNIT | GST_SEEK_FLAG_FLUSH, position);
	}

	/* get the preroll buffer from appsink, this block untils appsink really
	 * prerolls */
	g_signal_emit_by_name(sink, "pull-preroll", &sample, NULL);

	/* if we have a buffer now, convert it to a pixbuf. It's possible that we
	 * don't have a buffer because we went EOS right away or had an error. */
	if (!sample) {
		g_print("Could not get sample from GStreamer pipeline, perhaps a corrupt image?\n");
		converter_to_return = NULL;
		goto err_free_pipeline;
	}

	GstCaps *caps;
	GstStructure *s;

	/* get the snapshot buffer format now. We set the caps on the appsink so
	 * that it can only be an rgb buffer. The only thing we have not specified
	 * on the caps is the height, which is dependant on the pixel-aspect-ratio
	 * of the source material */
	caps = gst_sample_get_caps(sample);
	if (!caps) {
		g_print("could not get snapshot format\n");
		//exit(-1);
	}
	s = gst_caps_get_structure(caps, 0);

	/* we need to get the final caps on the buffer to get the size */
	res = gst_structure_get_int(s, "width", &width);
	res |= gst_structure_get_int(s, "height", &height);
	if (!res) {
		g_print("could not get snapshot dimension\n");
		//exit(-1);
	}

	/* Ugly global variables... */
	www = width;
	hhh = height;
	// find the smallest square texture size that's a power of two and fits around the image width/height
	// Example: image is 350x220 => texture size will be 512x512
	unsigned long int max;
	for (max = 1; (max < www || max < hhh) && max < TDFSB_MAX_TEX_SIZE; max *= 2) ;
	p2h = p2w = max;
	unsigned long cglmode = GL_RGB;

	GstBuffer *buffer = gst_sample_get_buffer(sample);
	if (!buffer) {
		printf("Warning: could not get a buffer from the GStreamer sample!\n");
		goto err_free_sample;
	}
	gst_buffer_map(buffer, &map, GST_MAP_READ);
	if (!map.data) {
		printf("Warning: could not map GStreamer buffer!\n");
		goto err_free_buffer;
	}

/*
	   // Save the preview for debugging (or caching?) purposes
	   // Note: gstreamer video buffers have a stride that is rounded up to the nearest multiple of 4
	   // Damn, the resulting image barely resembles the correct one... it has a pattern of Red Green Blue Black dots instead of B B B B
	   // Usually this indicates some kind of RGBA/RGB mismatch, but I can't find it...
	   GdkPixbuf *pixbuf = gdk_pixbuf_new_from_data(map.data,
	   GDK_COLORSPACE_RGB, FALSE, 8, width, height, // parameter 3 means "has alpha"
	   GST_ROUND_UP_4(width * 3), NULL, NULL);
	   gdk_pixbuf_save(pixbuf, "videopreview.png", "png", &error, NULL);
*/

	SDL_Surface *loader = SDL_CreateRGBSurfaceFrom(map.data, width, height, 24, GST_ROUND_UP_4(width * 3),
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
						       0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000
#else
						       0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF
#endif
	    );
	if (!loader) {
		printf("Warning: could not create SDL_Surface loader from buffer mapping's data map.data!\n");
		goto err_undo_mapping;
	}

	converter_to_return = ScaleSurface(loader, p2w, p2h);
	if (!converter_to_return)
		printf("Warning: cannot ScaleSurface loader to SDL_Surface converter!\n");

	/*
	   // Save the preview for debugging (or caching?) purposes
	   // Note: this is made for images without an alpha mask, otherwise you need to change FALSE to TRUE and www * 3 to www * 4
	   SDL_LockSurface(converter_to_return);
	   error = NULL;
	   pixbuf = gdk_pixbuf_new_from_data(converter_to_return->pixels,
	   GDK_COLORSPACE_RGB, FALSE, 8, p2w, p2h, // parameter 3 means "has alpha", 4 = bits per sample
	   GST_ROUND_UP_4(p2w * 3), NULL, NULL);        // parameter 7 = rowstride
	   gdk_pixbuf_save(pixbuf, "get_image_from_file_converter.png", "png", &error, NULL);
	   if (error != NULL) {
	   g_print("Could not save image preview to file: %s\n", error->message);
	   g_error_free(error);
	   exit(-1);
	   }
	   SDL_UnlockSurface(converter_to_return);
	 */

	// If we do this, the window is gone, and can't be brought back with windowraise
	//system("xdotool windowminimize --sync 37748743\n");

	// sleep(1);    // Wait until the window is raised, otherwise we might end up with a gray screen

	// Cleanups
	SDL_FreeSurface(loader);

	// We don't free the converter surface, that will be done when it has been *used* by the texture setting code,
	// or when we exit this folder and re-do a leodir() operation (= entering another folder)

 err_undo_mapping:
	gst_buffer_unmap(buffer, &map);
 err_free_buffer:
	// TODO?
 err_free_sample:
	gst_sample_unref(sample);
 err_free_pipeline:
	cleanup_media_player();
 err_out:
	if (converter_to_return) {
		toreturn = (texture_description *) malloc(sizeof(texture_description));
		toreturn->texturesurface = converter_to_return;
		toreturn->originalwidth = www;
		toreturn->originalheight = hhh;
		toreturn->textureformat = cglmode;
		return toreturn;
	} else
		return NULL;
}

void toggle_media_pipeline(void)
{
	GstState state;
	gst_element_get_state(GST_ELEMENT(pipeline), &state, NULL, GST_CLOCK_TIME_NONE);

	if (state != GST_STATE_PAUSED) {
		// We are already playing the selected videofile, so pause it
		gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PAUSED);
	} else {
		// We have selected this file already but it is already paused so play it again
		// TODO: check if the video has finished and if it has, start it again, or make it loop
		gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING);
	}
}

void play_media(char *fullpath, tree_entry * object)
{
	GstBus *bus = NULL;
	GstElement *fakesink = NULL;

	GError *error = NULL;
	gchar *uri = gst_filename_to_uri(fullpath, &error);
	if (error != NULL) {
		g_print("Could not convert filename %s to URI: %s\n", fullpath, error->message);
		g_error_free(error);
		exit(1);
	}

	gchar *descr;
	if (object->regtype == VIDEOFILE) {
		descr = g_strdup_printf("uridecodebin uri=%s name=player ! videoconvert ! videoscale ! video/x-raw,width=%d,height=%d,format=RGB ! fakesink name=fakesink0 sync=1 player. ! audioconvert ! playsink", uri, object->texturewidth, object->textureheight);
	} else if (object->regtype == AUDIOFILE) {
		descr = g_strdup_printf("uridecodebin uri=%s ! audioconvert ! playsink", uri);
	} else if (object->regtype == VIDEOSOURCEFILE) {
		descr = g_strdup_printf("v4l2src device=%s ! videoconvert ! videoscale ! video/x-raw,width=%d,height=%d,format=RGB ! fakesink name=fakesink0 sync=1", fullpath, object->texturewidth, object->textureheight);
	} else if (object->regtype == PROCESS) {
		// Get PID from filename
		// Get Window ID(s) for this PID
		//int pid = 5288;
		int windowid = 37748743;
		descr = g_strdup_printf("ximagesrc xid=%d ! videoconvert ! videoscale ! video/x-raw,width=%d,height=%d,format=RGB ! fakesink     name=fakesink0 sync=1", windowid, object->texturewidth, object->textureheight);
		//sleep(1);
		//system("xdotool search --pid 5288\n");
		system("xdotool windowraise 37748743\n");
		sleep(1);
	} else {
		startx();
		object->texturewidth = 2048;
		object->textureheight = 2048;
		object->textureformat = GL_RGB;
		object->originalwidth = 1920;
		object->originalheight = 1080;
		// Notes:
		// - framerate=60/1 does not seem to have any effect in speeding up the cursor movements
		// - sync=0 does not seem to have any effect in speeding up the cursor movements
		// - use-damage=0 seems to have a *slowdown* effect on the framerate
		descr = g_strdup_printf("ximagesrc display-name=:1 ! videoconvert ! videoscale ! video/x-raw,width=%d,height=%d,format=RGB,framerate=60/1 ! fakesink name=fakesink0 sync=0", object->texturewidth, object->textureheight);
	}
	// Use this for pulseaudio:
	// gchar *descr = g_strdup_printf("uridecodebin uri=%s name=player ! videoconvert ! videoscale ! video/x-raw,width=256,height=256,format=RGB ! fakesink name=fakesink0 sync=1 player. ! audioconvert ! pulsesink client-name=3dfsb", uri);

	printf("gst-launch-1.0 %s\n", descr);
	pipeline = (GstPipeline *) gst_parse_launch(descr, &error);

	if (error != NULL) {
		g_print("could not construct pipeline: %s\n", error->message);
		g_error_free(error);
		exit(-1);
	}
	// Debugging:
	//GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline");

	bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
	gst_bus_add_signal_watch(bus);
	gst_bus_enable_sync_message_emission(bus);
	gst_object_unref(bus);

	fakesink = gst_bin_get_by_name(GST_BIN(pipeline), "fakesink0");
	if (fakesink && GST_IS_ELEMENT(pipeline)) {
		g_object_set(G_OBJECT(fakesink), "signal-handoffs", TRUE, NULL);
		// Set a callback function for the handoff signal (when a new frame is received)
		g_signal_connect(fakesink, "handoff", G_CALLBACK(on_gst_buffer), NULL);
		gst_object_unref(fakesink);
	} else {
		// There is no fakesink, must be because we are playing an audio or videosource file
	}
	framecounter = displayedframenumber = 0;
	gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING);

	TDFSB_MEDIA_FILE = object;
}
