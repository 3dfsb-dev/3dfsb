/***************************************************************************
                          3dfsb.c  -  3D filesystem browser
                             -------------------
    begin                : Fri Feb 23 2001
    copyleft             : (C) 2014 - 2015 by Tom Van Braeckel as 3dfsb
    copyright            : (C) 2001 - 2007 by Leander Seige as tdfsb
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/***************************************************************************
    Everything here is released under the GPL and maintained by Tom Van Braeckel.
    Help is welcome. I would like to invite everyone to make improvements so
    if you have bugreports, additional code, bugfixes, comments, suggestions,
    questions, testing reports, quality measures or something similar, do get in touch!
 ***************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <GL/glut.h>
#include <SDL.h>
#include <SDL_image.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <GL/gl.h>
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

#include <GL/glx.h>
#include "SDL/SDL_syswm.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#include <gst/gst.h>
#pragma GCC diagnostic pop

#include <magic.h>

// For saving pixbuf's to a file for debugging
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-prototypes"
#include <gtk/gtk.h>
#pragma GCC diagnostic pop

// Icons
#include "images/icon_pdf.xpm"

// Own headers
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"	// feel free to fix this warning so this pragma can be removed :-)
#include "common.c"
#pragma GCC diagnostic pop

#ifndef _GLUfuncptr
#define _GLUfuncptr void*
#endif

#ifdef PI
#undef PI
#endif

// To fix warnings about unused variables
// Note: __attribute__ ((unused)) also works but is gcc specific
#define UNUSED(x) (void)(x)

#define PI  3.14159265
#define SQF 0.70710

#define NUMBER_OF_FILETYPES	9
#define	DIRECTORY	0
#define	IMAGEFILE	1
#define	TEXTFILE	2
#define	PDFFILE		3
#define	ZIPFILE		4
#define	VIDEOFILE	5
#define	AUDIOFILE	6
#define	VIDEOSOURCEFILE	7
#define	UNKNOWNFILE	8

#define	MIME_AUDIO	"audio/"
#define	MIME_IMAGE	"image/"
#define	MIME_PDF	"application/pdf"
#define	MIME_TEXT	"text/"
#define	MIME_VIDEO	"video/"
#define	MIME_ZIP	"application/zip"

#define PATH_DEV_V4L	"/dev/video"

#define	NUMBER_OF_TOOLS	2
#define TOOL_SELECTOR	0
#define TOOL_WEAPON	1
int CURRENT_TOOL = TOOL_SELECTOR;	// The tool (or weapon) we are currently holding

const SDL_VideoInfo *info = NULL;
SDL_Event event;
SDL_Surface *window;
int bpp = 0, rgb_size[3];
GLUquadricObj *aua1, *aua2;
SDL_TimerID TDFSB_TIMER_ID;
void (*TDFSB_FUNC_IDLE) (void), (*TDFSB_FUNC_DISP) (void);
void (*TDFSB_FUNC_MOUSE) (int button, int state), (*TDFSB_FUNC_MOTION) (int x, int y);
int (*TDFSB_FUNC_KEY) (unsigned char key);
int (*TDFSB_FUNC_UPKEY) (unsigned char key);

unsigned long int www, hhh, p2w, p2h, cglmode;

long int c1, c3, c4, cc, cc1, cc2, cc3, cc4;

static GLuint TDFSB_DisplayLists = 0;
static GLuint TDFSB_CylinderList = 0;
static GLuint TDFSB_AudioList = 0;
static GLuint TDFSB_HelpList = 0;
static GLuint TDFSB_SolidList = 0;
static GLuint TDFSB_BlendList = 0;

unsigned long int total_objects_in_grid;
unsigned int total_objects_in_grid_ratio;
GLdouble mx, my, mz, r, u, v;

struct tree_entry {
	char *name;
	unsigned long int namelen;
	char *linkpath;
	unsigned int mode, regtype, rasterx, rasterz;
	GLfloat posx, posy, posz, scalex, scaley, scalez;
	// texturewidth = p2w = just the next power of two that fits the texture size!
	unsigned int texturewidth, textureheight, textureformat, textureid;
	unsigned int originalwidth;
	unsigned int originalheight;
	unsigned int tombstone;	// object has been deleted
	unsigned char *textfilecontents;	/* this can point to the contents of a textfile */
	SDL_Surface *texturesurface;
	off_t size;
	struct tree_entry *next;
};

struct tree_entry *root, *FMptr, *TDFSB_OBJECT_SELECTED = NULL, *TDFSB_OA = NULL;
char *FCptr;

char temp_trunc[4096];
char TDFSB_CURRENTPATH[4096], fullpath[4096], yabuf[4096], fpsbuf[12], cfpsbuf[12], throttlebuf[14], ballbuf[20], flybuf[12], classicbuf[12];
//char home[512];
char TDFSB_CUSTOM_EXECUTE_STRING[4096];
char TDFSB_CES_TEMP[4096];
char *alert_kc = "MALFORMED KEYBOARD MAP";
char *alert_kc2 = "USING DEFAULTS";
unsigned char ch, TDFSB_KEY_FINDER = 0;
GLuint TDFSB_TEX_NUM;
GLuint *TDFSB_TEX_NAMES;

char *help_str, *tmpstr, *help_copy;
int cnt;
int forwardkeybuf = 0;
int leftkeybuf = 0;
int rightkeybuf = 0;
int backwardkeybuf = 0;
int upkeybuf = 0;
int downkeybuf = 0;

GLdouble prevlen;

#define NUMBER_OF_EXTENSIONS	3

char *xsuff[NUMBER_OF_EXTENSIONS];
unsigned int tsuff[NUMBER_OF_EXTENSIONS];
char *nsuff[NUMBER_OF_FILETYPES];

GLfloat fh, fh2, mono;

int SWX, SWY, PWX, PWY, aSWX, aSWY, aPWX, aPWY, PWD;
GLdouble centX, centY;
int TDFSB_SHADE = 1, TDFSB_FILENAMES = 2, TDFSB_ICUBE = 1, TDFSB_SHOW_DISPLAY = 3, TDFSB_XL_DISPLAY = 45, TDFSB_HAVE_MOUSE = 1, TDFSB_ANIM_COUNT = 0, TDFSB_DIR_ALPHASORT = 1, TDFSB_OBJECT_SEARCH = 0;
int TDFSB_SPEED_DISPLAY = 200, TDFSB_SHOW_DOTFILES = 0, TDFSB_SHOW_CROSSHAIR = 0, TDFSB_FULLSCREEN = 0, TDFSB_SHOW_HELP = 1, TDFSB_GROUND_CROSS = 0, TDFSB_NOTIFY;
int TDFSB_CONFIG_FULLSCREEN = 0, TDFSB_SHOW_FPS = 0;
int TDFSB_ANIM_STATE = 0;	// This keeps track of the state of the "approach" action that brings you closer to an object in 4 steps
int TDFSB_MODE_FLY = 0, TDFSB_FLY_DISPLAY = 0, TDFSB_CLASSIC_NAV = 0, TDFSB_CLASSIC_DISPLAY = 0, TDFSB_ALERT_KC = 0;
int TDFSB_SHOW_THROTTLE = 0, TDFSB_SHOW_BALL = 0;
int TDFSB_FPS_CONFIG = 0, TDFSB_FPS_CACHE = 0, TDFSB_FPS_REAL = 0, TDFSB_FPS_DISP = 0, TDFSB_FPS_DT = 1000, TDFSB_SHOW_CONFIG_FPS = 0;
int TDFSB_MW_STEPS = 1;
long TDFSB_US_RUN = 0;
int TDFSB_CSE_FLAG = 1;

struct tree_entry *TDFSB_MEDIA_FILE;

// Used to limit the number of video texture changes for performance
unsigned int framecounter;
unsigned int displayedframenumber;

struct timeval ltime, ttime, rtime, wtime;
long sec;

GLdouble TDFSB_OA_DX, TDFSB_OA_DY, TDFSB_OA_DZ;
GLdouble TDFSB_NORTH_X = 0, TDFSB_NORTH_Z = 0;
GLfloat TDFSB_MAXX, TDFSB_MAXZ;
GLfloat TDFSB_MINX, TDFSB_MINZ;
GLfloat TDFSB_STEPX, TDFSB_STEPZ;
GLfloat TDFSB_GG_R = 0.2, TDFSB_GG_G = 0.2, TDFSB_GG_B = 0.6;
GLfloat TDFSB_BG_R = 0.0, TDFSB_BG_G = 0.0, TDFSB_BG_B = 0.0;
GLfloat TDFSB_FN_R = 1.0, TDFSB_FN_G = 1.0, TDFSB_FN_B = 1.0;
GLint TDFSB_BALL_DETAIL = 8, TDFSB_WAS_NOREAD = 0;
unsigned int TDFSB_MAX_TEX_SIZE = 0;
GLfloat mousesense = 1.5;
GLfloat mousespeed = 1.0;	// 1-20, with 1 being maximum
GLfloat headspeed = 2.0;	// 1.1-2.0, with 2.0 being maximum
GLdouble vposx, vposy, vposz, tposx, tposy, tposz, smoox, smooy, smooz, smoou, lastposz, lastposx, uposy;

GLfloat mat_ambient[] = { 0.7, 0.7, 0.7, 1.0 };
GLfloat mat_ambient_red[] = { 1.0, 0.0, 0.0, 0.5 };
GLfloat mat_ambient_grn[] = { 0.0, 1.0, 0.0, 0.5 };
GLfloat mat_ambient_yel[] = { 1.0, 1.0, 0.0, 0.5 };
GLfloat mat_ambient_gry[] = { 1.0, 1.0, 1.0, 0.25 };

GLfloat mat_diffuse[] = { 0.8, 0.8, 0.8, 1.0 };
GLfloat mat_diffuse_red[] = { 1.0, 0.0, 0.0, 0.5 };
GLfloat mat_diffuse_grn[] = { 0.0, 1.0, 0.0, 0.5 };
GLfloat mat_diffuse_yel[] = { 1.0, 1.0, 0.0, 0.5 };
GLfloat mat_diffuse_gry[] = { 1.0, 1.0, 1.0, 0.25 };

GLfloat mat_specular[] = { 0.9, 0.9, 0.9, 1.0 };
GLfloat mat_specular_red[] = { 1.0, 0.0, 0.0, 0.5 };
GLfloat mat_specular_grn[] = { 0.0, 1.0, 0.0, 0.5 };
GLfloat mat_specular_yel[] = { 1.0, 1.0, 0.0, 0.5 };
GLfloat mat_specular_gry[] = { 1.0, 1.0, 1.0, 0.25 };

GLfloat mat_emission[] = { 0.0, 0.0, 0.0, 1.0 };
GLfloat mat_emission_red[] = { 0.4, 0.2, 0.2, 0.5 };
GLfloat mat_emission_grn[] = { 0.2, 0.4, 0.2, 0.5 };
GLfloat mat_emission_yel[] = { 0.4, 0.4, 0.2, 0.5 };
GLfloat mat_emission_gry[] = { 0.3, 0.3, 0.3, 0.25 };

GLfloat mat_shininess[] = { 50.0 };

GLfloat light_position[] = { 1.0, 2.0, 1.0, 0.0 };

DIR *mydir, *tempdir;
struct dirent *myent;

static GLfloat spin = 0.0;

static GLfloat verTex2[] = {
	0.0, 1.0, 0.0, 0.0, 1.0, -1.0, -1.0, 1.0,
	0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 1.0, 1.0,
	1.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0,
	1.0, 1.0, 0.0, 0.0, 1.0, 1.0, -1.0, 1.0,

	0.0, 1.0, 0.0, 0.0, -1.0, -1.0, -1.0, -1.0,
	0.0, 0.0, 0.0, 0.0, -1.0, -1.0, 1.0, -1.0,
	1.0, 0.0, 0.0, 0.0, -1.0, 1.0, 1.0, -1.0,
	1.0, 1.0, 0.0, 0.0, -1.0, 1.0, -1.0, -1.0,

	0.99, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0,	/* please mail me if you know why 1.0 doesn't work here but 0.99 */
	0.99, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, -1.0,
	0.99, 0.99, 1.0, 0.0, 0.0, 1.0, -1.0, -1.0,
	0.99, 0.99, 1.0, 0.0, 0.0, 1.0, -1.0, 1.0,

	0.0, 1.0, -1.0, 0.0, 0.0, -1.0, -1.0, -1.0,
	0.0, 1.0, -1.0, 0.0, 0.0, -1.0, -1.0, 1.0,
	0.0, 0.0, -1.0, 0.0, 0.0, -1.0, 1.0, 1.0,
	0.0, 0.0, -1.0, 0.0, 0.0, -1.0, 1.0, -1.0,

	1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 1.0, -1.0,
	1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 1.0,
	0.0, 0.0, 0.0, 1.0, 0.0, -1.0, 1.0, 1.0,
	0.0, 0.0, 0.0, 1.0, 0.0, -1.0, 1.0, -1.0
};

/*      Texture      Normals         Vertex        */

// Used both for the on-screen display and for the console help text
#define	STATIC_HELP_TEXT \
"3D File System Browser\n" \
"----------------------\n" \
"Esc           quit   F1/F2    speed +/-\n" \
"Mouse move    look   F3/F4      rot +/-\n" \
"UP         forward   F5/F6  ball detail\n" \
"DOWN      backward   F7/F8  max fps +/-\n" \
"L/R arrow   strafe   F9     change tool\n" \
"PgUp/Down or MMB+Mouse move up/downward\n" \
"HOME     start pos   END    ground zero\n" \
"LMB  select object  + RMB|CTRL approach\n" \
"+ENTER  play media or enter directory\n\n" \

unsigned char TDFSB_KC_FLY, TDFSB_KC_HELP, TDFSB_KC_HOME;
unsigned char TDFSB_KC_FS, TDFSB_KC_DOT, TDFSB_KC_RELM;
unsigned char TDFSB_KC_RL, TDFSB_KC_CDU, TDFSB_KC_IMBR;
unsigned char TDFSB_KC_INFO, TDFSB_KC_DISP, TDFSB_KC_CRH;
unsigned char TDFSB_KC_FPS, TDFSB_KC_GCR, TDFSB_KC_SHD;
unsigned char TDFSB_KC_NAME, TDFSB_KC_SORT, TDFSB_KC_CLASS;
unsigned char TDFSB_KC_UP, TDFSB_KC_DOWN, TDFSB_KC_LEFT;
unsigned char TDFSB_KC_RIGHT, TDFSB_KC_SAVE, TDFSB_KC_FTH;
unsigned char TDFSB_KC_FORWARD, TDFSB_KC_BACKWARD;

unsigned char *TDFSB_KEYLIST[] = { &TDFSB_KC_FLY, &TDFSB_KC_HELP, &TDFSB_KC_HOME,
	&TDFSB_KC_FS, &TDFSB_KC_DOT, &TDFSB_KC_RELM,
	&TDFSB_KC_RL, &TDFSB_KC_CDU, &TDFSB_KC_IMBR,
	&TDFSB_KC_INFO, &TDFSB_KC_DISP, &TDFSB_KC_CRH,
	&TDFSB_KC_FPS, &TDFSB_KC_GCR, &TDFSB_KC_SHD,
	&TDFSB_KC_NAME, &TDFSB_KC_SORT, &TDFSB_KC_CLASS,
	&TDFSB_KC_FORWARD, &TDFSB_KC_BACKWARD,
	&TDFSB_KC_UP, &TDFSB_KC_DOWN, &TDFSB_KC_LEFT,
	&TDFSB_KC_RIGHT, &TDFSB_KC_SAVE, &TDFSB_KC_FTH
};

const char *TDFSB_KEYLIST_NAMES[] = { "KeyFlying", "KeyHelp", "KeyJumpHome",
	"KeyFullScreen", "KeyDotFilter", "KeyMouseRelease",
	"KeyReload", "KeyCDup", "KeyImageBricks",
	"KeyGLInfo", "KeyDisplay", "KeyCrossHair",
	"KeyFPS", "KeyGrndCross", "KeyShadeMode",
	"KeyFileNames", "KeyAlphaSort", "KeyClassicNav",
	"KeyForward", "KeyBackward",
	"KeyUp", "KeyDown", "KeyLeft",
	"KeyRight", "KeySaveConfig", "KeyFPSThrottle"
};

unsigned int TDFSB_KEYLIST_NUM = 26;

char *tdfsb_comment[] = { "detail level of the spheres, must be at least 4",
	"directory to start, absolute path",
	"maximum texture size, must be 2^n, 0 for maximum",
	"width of the window in pixel",
	"height of the window in pixel",
	"width fullscreen", "height fullscreen", "depth fullscreen, 0 for same as desktop",
	"R", "G components of the grids color, range 0.0 ... 1.0", "B",
	"images have a volume (toggle later)",
	"show hidden files (toggle later)",
	"sort the files alphabetically (toggle later)",
	"R", "G components of the background color, range 0.0 ... 1.0", "B",
	"start in fullscreen mode (toggle later)", "key dummy",
	"show the crosshair (toggle later)",
	"show the cross on the ground (toggle later)",
	"mouse buttons for forward and backward (toggle later)",
	"fly in the viewing direction (toggle later)",
	"throttle fps to leave cpu time for other apps (toggle later), 0 for maximum speed",
	"velocity of movement (F1/F2), range 1 ... 20",
	"velocity of rotation (F3/F4), range 0.1 ... 2.0",
	"R", "G components of the filename color, range 0.0 ... 1.0", "B",
	"units to lift up per mousewheel step",
	"custom execute string"
};

char *param[] = { "BallDetail", "StartDir", "MaxTexSize", "WindowWidth", "WindowHeight", "FullscreenWidth", "FullscreenHeight", "FullscreenDepth", "GridRed", "GridGreen",
	"GridBlue", "ImageBricks", "ShowDotFiles", "AlphaSort", "BGRed", "BGGreen", "BGBlue", "FullScreen", "Key",
	"ShowCrossHair", "ShowGroundCross", "ClassicNavigation", "FlyingMode", "MaxFPS",
	"MoveVelocity", "LookVelocity", "NameRed", "NameGreen", "NameBlue", "LiftSteps", "CustomExecuteString"
};

void *param_value[] = { &TDFSB_BALL_DETAIL, &TDFSB_CURRENTPATH, &TDFSB_MAX_TEX_SIZE, &SWX, &SWY, &PWX, &PWY,
	&PWD, &TDFSB_GG_R, &TDFSB_GG_G, &TDFSB_GG_B,
	&TDFSB_ICUBE, &TDFSB_SHOW_DOTFILES, &TDFSB_DIR_ALPHASORT,
	&TDFSB_BG_R, &TDFSB_BG_G, &TDFSB_BG_B,
	&TDFSB_CONFIG_FULLSCREEN, NULL, &TDFSB_SHOW_CROSSHAIR, &TDFSB_GROUND_CROSS, &TDFSB_CLASSIC_NAV, &TDFSB_MODE_FLY,
	&TDFSB_FPS_CONFIG, &mousespeed, &headspeed, &TDFSB_FN_R, &TDFSB_FN_G, &TDFSB_FN_B, &TDFSB_MW_STEPS, &TDFSB_CUSTOM_EXECUTE_STRING
};

// Default values
// Too conservative: char *pdef[] = { "20", "/", "256", "400", "300", "640", "480", "0", "0.2", "0.2", "0.6", "yes", "no", "yes", "0.0", "0.0", "0.0", "no", "X", "yes", "no", "no", "no", "25", "2.0", "1.0", "1.0", "1.0", "1.0", "1", "cd \"%s\"; xterm&" };
char *pdef[] = { "20", "/", "0", "1024", "768", "1024", "768",
	"0", "0.2", "0.2",	// TDFSB_GG_R/G/B
	"0.6", "yes", "no", "yes",
	"0.0", "0.0", "0.0",	// TDFSB_BG_R/G/B
	"yes", "X", "yes", "no", "no", "yes",
	"0", "2.0", "1.3",	// FPS, mousespeed, headspeed
	"1.0", "1.0", "1.0",	// TDFSB_FB_R/G/B
	"1", "cd \"%s\"; xterm&"
};
int param_type[] = { 1, 2, 1, 1, 1, 1, 1, 1, 3, 3, 3, 4, 4, 4, 3, 3, 3, 4, 5, 4, 4, 4, 4, 1, 3, 3, 3, 3, 3, 1, 2 };	/* 1=int 2=string 3=float 4=boolean 5=keyboard */

unsigned int paracnt = 31;

static void display(void);
static int keyboard(unsigned char key);
static int keyboardup(unsigned char key);
static int keyfinder(unsigned char key);
static int keyupfinder(unsigned char key);
static int speckey(int key);
static int specupkey(int key);
static void stillDisplay(void);

/* GStreamer stuff */
GstPipeline *pipeline = NULL;
#define CAPS "video/x-raw,format=RGB"

// GstBuffer with the new frame
GstBuffer *videobuffer;

// Mimetype database handle
magic_t magic;

// Asynchronous texture loading
pthread_t async_load_textures_thread_id;

GAsyncQueue *loaded_textures_queue = NULL;

/* fakesink handoff callback */
static void on_gst_buffer(GstElement * fakesink, GstBuffer * buf, GstPad * pad, gpointer data)
{
	UNUSED(fakesink);
	UNUSED(pad);
	UNUSED(data);

	framecounter++;
	videobuffer = buf;
}

static void cleanup_media_player(void)
{
	TDFSB_MEDIA_FILE = NULL;	// Set this to NULL, because the later functions check it to know what they should display
	if (pipeline && GST_IS_ELEMENT(pipeline)) {
		printf("Cleaning up GStreamer pipeline\n");
		gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_NULL);
		gst_object_unref(pipeline);
	}
}

static void ende(int code)
{
	struct tree_entry *help;

	cleanup_media_player();

	magic_close(magic);	// Mimetype database

	glDeleteTextures(TDFSB_TEX_NUM, TDFSB_TEX_NAMES);
	if (TDFSB_TEX_NAMES != NULL)
		free(TDFSB_TEX_NAMES);
	if (total_objects_in_grid != 0) {
		help = root;
		while (help) {
			//printf("Freeing %s\n", help->name);
			FMptr = help;
			FCptr = help->name;
			free(FCptr);
			if (help->textfilecontents != NULL) {
				FCptr = (char *)help->textfilecontents;
				free(FCptr);
			}
			// Free up any texturesurface's that are set
			if (help->texturesurface != NULL) {
				SDL_FreeSurface(help->texturesurface);
				help->texturesurface = NULL;
			}
			if (help->linkpath != NULL) {
				FCptr = help->linkpath;
				free(FCptr);
			}
			help = help->next;
			free(FMptr);
		}
		root = NULL;
		total_objects_in_grid = 0;
	}
	SDL_Quit();
	exit(code);
}

static char *uppercase(char *str)
{
	char *newstr, *p;
	p = newstr = strdup(str);
	while (*p) {
		*p = (char)toupper(*p);
		p++;
	}

	return newstr;
}

static int get_file_type(char *filename)
{
	unsigned int temptype = 0;	// temptype needs to be 0 to check if we found a match later on!
	unsigned int extensionnr;
	const char *mime = magic_file(magic, filename);

	//printf("Got mimetype: %s\n", mime);
	if (!strncmp(mime, MIME_TEXT, 5)) {
		temptype = TEXTFILE;
	} else if (!strncmp(mime, MIME_IMAGE, 6)) {
		temptype = IMAGEFILE;
	} else if (!strncmp(mime, MIME_VIDEO, 6)) {
		temptype = VIDEOFILE;
	} else if (!strncmp(mime, MIME_AUDIO, 6)) {
		temptype = AUDIOFILE;
	} else if (!strncmp(mime, MIME_PDF, 15)) {
		temptype = PDFFILE;
	} else {
		// Some files are not identified by magic_file(), so we fallback to extension-based identification here
		for (extensionnr = 0; extensionnr < NUMBER_OF_EXTENSIONS; extensionnr++) {
			char *xsuff_upper = uppercase(xsuff[extensionnr]);
			char *ext_upper = uppercase(&(filename[strlen(filename) - strlen(xsuff[extensionnr])]));
			if (!strncmp(xsuff_upper, ext_upper, strlen(xsuff[extensionnr]))) {
				temptype = tsuff[extensionnr];
				break;
			}
		}
		// As a last resort, we consider it an unknown file
		if (!temptype)
			temptype = UNKNOWNFILE;
	}
	return temptype;
}

static void play_media(void)
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
	glBindTexture(GL_TEXTURE_2D, TDFSB_MEDIA_FILE->textureid);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TDFSB_MEDIA_FILE->texturewidth, TDFSB_MEDIA_FILE->textureheight, 0, GL_RGB, GL_UNSIGNED_BYTE, map.data);

	// Free up memory again
	gst_buffer_unmap(videobuffer, &map);

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
static SDL_Surface *ScaleSurface(SDL_Surface * Surface, double Width, double Height)
{
	int x, y, o_y, o_x;

	if (!Surface || (Width<0.1) || (Height<0.1))
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

static SDL_Surface *get_image_from_file(char *filename, unsigned int filetype)
{
	if (filetype != VIDEOFILE && filetype != VIDEOSOURCEFILE && filetype != IMAGEFILE) {
		printf("Error: get_image_from_file can only handle VIDEOFILE, VIDEOSOURCFILE and IMAGEFILE's!\n");
		return NULL;
	}
	GstElement *sink;
	gint width, height;
	GstSample *sample;
	gchar *descr;
	GError *error = NULL;
	gint64 duration, position;
	GstStateChangeReturn ret;
	gboolean res;
	GstMapInfo map;

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
	}
	//printf("gst-launch-1.0 %s\n", descr);
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
		g_print("failed to play the file\n");
		//exit(-1);
	case GST_STATE_CHANGE_NO_PREROLL:
		/* for live sources, we need to set the pipeline to PLAYING before we can
		 * receive a buffer. */
		gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING);
		//g_print("live sources not supported yet\n");
		//exit(-1);
	default:
		break;
	}
	/* This can block for up to 5 seconds. If your machine is really overloaded,
	 * it might time out before the pipeline prerolled and we generate an error. A
	 * better way is to run a mainloop and catch errors there. */
	ret = gst_element_get_state(GST_ELEMENT(pipeline), NULL, NULL, 5 * GST_SECOND);
	if (ret == GST_STATE_CHANGE_FAILURE) {
		g_print("failed to play the file\n");
		//exit(-1);
	}

	if (filetype == VIDEOFILE) {	// VIDEOSOURCEFILE's and IMAGEFILE's cannot be seeked
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
	if (sample) {
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

	} else {
		g_print("could not make snapshot\n");
	}

	/* Ugly global variables... */
	www = width;
	hhh = height;
	// find the smallest square texture size that's a power of two and fits around the image width/height
	// Example: image is 350x220 => texture size will be 512x512
	unsigned long int max;
	for (max = 1; (max < www || max < hhh) && max < TDFSB_MAX_TEX_SIZE; max *= 2) ;
	p2h = p2w = max;
	cglmode = GL_RGB;	// We set this globally here, and it is used somewhere later in the code that calls get_image_from_file()

	GstBuffer *buffer = gst_sample_get_buffer(sample);
	gst_buffer_map(buffer, &map, GST_MAP_READ);

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

	SDL_Surface *converter = ScaleSurface(loader, p2w, p2h);
	if (!converter) {
		SDL_FreeSurface(loader);
		printf("Cannot read image %s ! (converting)\n", filename);
		www = 0;
		hhh = 0;
		p2w = 0;
		p2h = 0;
		return NULL;
	}

	/*
	   // Save the preview for debugging (or caching?) purposes
	   // Note: this is made for images without an alpha mask, otherwise you need to change FALSE to TRUE and www * 3 to www * 4
	   SDL_LockSurface(converter);
	   error = NULL;
	   pixbuf = gdk_pixbuf_new_from_data(converter->pixels,
	   GDK_COLORSPACE_RGB, FALSE, 8, p2w, p2h, // parameter 3 means "has alpha", 4 = bits per sample
	   GST_ROUND_UP_4(p2w * 3), NULL, NULL);        // parameter 7 = rowstride
	   gdk_pixbuf_save(pixbuf, "get_image_from_file_converter.png", "png", &error, NULL);
	   if (error != NULL) {
	   g_print("Could not save image preview to file: %s\n", error->message);
	   g_error_free(error);
	   exit(-1);
	   }
	   SDL_UnlockSurface(converter);
	 */

	SDL_FreeSurface(loader);

	gst_buffer_unmap(buffer, &map);
	gst_sample_unref(sample);

	cleanup_media_player();

	return converter;
}

// This new thread loads the textures
static void *async_load_textures(void *arg)
{
	UNUSED(arg);

	// For each object, load its texture and (if possible) set the dimensions...
	struct tree_entry *object;
	for (object = root; object; object = object->next) {

		// For regular files or device files (because the others don't have textures (yet),
		// but I think these can be either symlinks or real files because of the way this "mode" property is devised...
		if ((((object->mode) & 0x1F) != 0) && ((object->mode) & 0x1F) != 2)
			continue;

		char async_fullpath[4096] = { 0 };	// allocate on the stack for automatic free'ing
		strcpy(async_fullpath, TDFSB_CURRENTPATH);
		if (strlen(async_fullpath) > 1)
			strcat(async_fullpath, "/");
		strcat(async_fullpath, object->name);
		printf("Loading texture of %s\n", async_fullpath);

		if (object->regtype == TEXTFILE) {
			FILE *fileptr = fopen(async_fullpath, "r");
			if (!fileptr) {
				printf("TEXT FAILED: %s\n", async_fullpath);
			} else {
				object->textfilecontents = (unsigned char *)malloc(1000 * sizeof(unsigned char));
				fread(object->textfilecontents, sizeof(char), 1000, fileptr);
				fclose(fileptr);
			}
		} else if (object->regtype == IMAGEFILE || object->regtype == VIDEOFILE || object->regtype == VIDEOSOURCEFILE) {
			object->texturesurface = get_image_from_file(async_fullpath, object->regtype);
			object->originalwidth = www;
			object->originalheight = hhh;
			object->textureformat = cglmode;
		} else if (object->regtype == PDFFILE) {
			// Show the PDF logo
			// This is also possible:
			// object->texturesurface = get_image_from_file("images/icon_pdf.png", IMAGEFILE);
			SDL_Surface *loader = IMG_ReadXPMFromArray(icon_pdf);
			SDL_PixelFormat RGBAFormat;
			RGBAFormat.palette = 0;
			RGBAFormat.colorkey = 0;
			RGBAFormat.alpha = 0;
			RGBAFormat.BitsPerPixel = 32;
			RGBAFormat.BytesPerPixel = 4;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			RGBAFormat.Rmask = 0xFF000000;
			RGBAFormat.Rshift = 0;
			RGBAFormat.Rloss = 0;
			RGBAFormat.Gmask = 0x00FF0000;
			RGBAFormat.Gshift = 8;
			RGBAFormat.Gloss = 0;
			RGBAFormat.Bmask = 0x0000FF00;
			RGBAFormat.Bshift = 16;
			RGBAFormat.Bloss = 0;
			RGBAFormat.Amask = 0x000000FF;
			RGBAFormat.Ashift = 24;
			RGBAFormat.Aloss = 0;
#else
			RGBAFormat.Rmask = 0x000000FF;
			RGBAFormat.Rshift = 24;
			RGBAFormat.Rloss = 0;
			RGBAFormat.Gmask = 0x0000FF00;
			RGBAFormat.Gshift = 16;
			RGBAFormat.Gloss = 0;
			RGBAFormat.Bmask = 0x00FF0000;
			RGBAFormat.Bshift = 8;
			RGBAFormat.Bloss = 0;
			RGBAFormat.Amask = 0xFF000000;
			RGBAFormat.Ashift = 0;
			RGBAFormat.Aloss = 0;
#endif
			object->texturesurface = SDL_ConvertSurface(loader, &RGBAFormat, SDL_SWSURFACE);
			object->textureformat = GL_RGBA;
			object->originalwidth = 512;
			object->originalheight = 512;
		}

		if (object->texturesurface) {
			/* Ugly dirty global variables */
			object->texturewidth = object->texturesurface->w;
			object->textureheight = object->texturesurface->h;
			printf("Original dimensions: %dx%d %s TEXTURE: %dx%d\n", object->originalwidth, object->originalheight, async_fullpath, object->texturewidth, object->textureheight);
			if (object->originalwidth < object->originalheight) {
				object->scalex = object->scalez = ((GLfloat) log(((double)object->originalwidth / 128) + 1)) + 1;
				object->scaley = (object->originalheight * (object->scalex)) / www;
			} else {
				object->scaley = ((GLfloat) log(((double)object->originalheight / 128) + 1)) + 1;
				object->scalex = object->scalez = (object->originalwidth * (object->scaley)) / object->originalheight;
			}
			object->posy = object->scaley - 1;	// vertical position of the object
			object->scalez = 0.5;	// flatscreens instead of the default ugly big square blocks

			// Add to queue to redraw/retexture this object in the rendering thread
			g_async_queue_push(loaded_textures_queue, object);
		}
	}			// end of directory entry iterator
	printf("Finished loading all textures, texture loading thread exiting...\n");
	return NULL;
}

static void tdb_gen_list(void)
{
	int mat_state;
	struct tree_entry *help;

	mat_state = 0;

	glNewList(TDFSB_SolidList, GL_COMPILE);
	for (help = root; help; help = help->next) {
		//printf("Adding file %s with mode %x and regtype %x\n", help->name, help->mode, help->regtype);
		if (help->tombstone)
			continue;	// Skip files that are tombstoned

		glPushMatrix();
		mx = help->posx;
		mz = help->posz;
		my = help->posy;
		if (((help->mode) & 0x1F) == 1 || ((help->mode) & 0x1F) == 11) {	// Directory
			glTranslatef(mx, my, mz);
			if ((help->mode) & 0x20)
				glutSolidSphere(0.5, TDFSB_BALL_DETAIL, TDFSB_BALL_DETAIL);
			else
				glutSolidSphere(1, TDFSB_BALL_DETAIL, TDFSB_BALL_DETAIL);
		} else if ((((help->mode) & 0x1F) == 0 || ((help->mode) & 0x1F) == 10) || ((((help->mode) & 0x1F) == 2) && (help->regtype == VIDEOSOURCEFILE))) {	// Regular file, except VIDEOSOURCEFILE's, which are character devices
			if (((help->regtype == IMAGEFILE) || (help->regtype == PDFFILE) || (help->regtype == VIDEOFILE) || (help->regtype == VIDEOSOURCEFILE)) && (((help->mode) & 0x1F) == 0 || ((help->mode) & 0x1F) == 2)) {
				if ((help->mode) & 0x20) {
					glTranslatef(mx, 0, mz);
					if (help->scalex > help->scaley) {
						if (help->scalex > help->scalez)
							glScalef(0.5, 0.5 * (help->scaley) / (help->scalex), 0.5 * (help->scalez) / (help->scalex));	/* x am groessten */
						else
							glScalef(0.5 * (help->scalex) / (help->scalez), 0.5 * (help->scaley) / (help->scalez), 0.5);	/* z am groessten */
					} else if (help->scaley > help->scalez)
						glScalef(0.5 * (help->scalex) / (help->scaley), 0.5, 0.5 * (help->scalez) / (help->scaley));	/* y am groessten */
					else
						glScalef(0.5 * (help->scalex) / (help->scalez), 0.5 * (help->scaley) / (help->scalez), 0.5);	/* z am groessten */
				} else {
					glTranslatef(mx, my, mz);
					glScalef(help->scalex, help->scaley, help->scalez);
				}

				// Link the texture to the cube
				//printf("Linking texture with id %d to cube for object %s\n", help->textureid, help->name);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, help->textureid);
				if (TDFSB_ICUBE)
					cc1 = 20;
				else
					cc1 = 4;
				glBegin(GL_QUADS);
				for (cc = 0; cc < cc1; cc++) {
					glNormal3f(verTex2[cc * 8 + 2], verTex2[cc * 8 + 3], verTex2[cc * 8 + 4]);
					glTexCoord2f(verTex2[cc * 8 + 0], verTex2[cc * 8 + 1]);
					glVertex3f(verTex2[cc * 8 + 5], verTex2[cc * 8 + 6], verTex2[cc * 8 + 7]);
				}
				glEnd();
				glDisable(GL_TEXTURE_2D);
			} else if (help->regtype == UNKNOWNFILE) {
				if ((help->mode) & 0x20) {
					glTranslatef(mx, 0, mz);
					if (help->scalex > help->scaley) {
						if (help->scalex > help->scalez)
							glScalef(0.5, 0.5 * (help->scaley) / (help->scalex), 0.5 * (help->scalez) / (help->scalex));	/* x am groessten */
						else
							glScalef(0.5 * (help->scalex) / (help->scalez), 0.5 * (help->scaley) / (help->scalez), 0.5);	/* z am groessten */
					} else if (help->scaley > help->scalez)
						glScalef(0.5 * (help->scalex) / (help->scaley), 0.5, 0.5 * (help->scalez) / (help->scaley));	/* y am groessten */
					else
						glScalef(0.5 * (help->scalex) / (help->scalez), 0.5 * (help->scaley) / (help->scalez), 0.5);	/* z am groessten */
				} else {
					glTranslatef(mx, my, mz);
					glScalef(help->scalex, help->scaley, help->scalez);
				}

				glutSolidCube(2);
			}
		} else {
			if ((help->mode) != 0x25) {
				glTranslatef(mx, my, mz);
				if (((help->mode) & 0x20))
					glScalef(0.5, 0.5, 0.5);
				glutSolidOctahedron();
			}
		}
		glPopMatrix();
	}
/* drawing the ground grid */
	glPushMatrix();
	glDisable(GL_LIGHTING);
	glColor4f(TDFSB_GG_R, TDFSB_GG_G, TDFSB_GG_B, 1.0);
	for (c1 = 0; c1 <= 20; c1++) {
		glBegin(GL_LINES);
		glVertex3f(TDFSB_MINX + TDFSB_STEPX * c1, -1, TDFSB_MINZ);
		glVertex3f(TDFSB_MINX + TDFSB_STEPX * c1, -1, TDFSB_MAXZ);
		glVertex3f(TDFSB_MINX, -1, TDFSB_MINZ + TDFSB_STEPZ * c1);
		glVertex3f(TDFSB_MAXX, -1, TDFSB_MINZ + TDFSB_STEPZ * c1);
		glEnd();
	}

	glEnable(GL_LIGHTING);
	glPopMatrix();
	glEndList();

	glNewList(TDFSB_BlendList, GL_COMPILE);
	glEnable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Draw text files
	mat_state = 0;
	for (help = root; help; help = help->next) {
		if (help->tombstone)
			continue;	// Skip files that are tombstoned

		mx = help->posx;
		mz = help->posz;
		my = help->posy;
		if (((help->regtype == TEXTFILE) && ((help->mode) & 0x1F) == 0)) {
			if (!mat_state) {
				glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_yel);
				glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient_yel);
				glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular_yel);
				glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission_yel);
				glMaterialfv(GL_BACK, GL_DIFFUSE, mat_diffuse_yel);
				glMaterialfv(GL_BACK, GL_AMBIENT, mat_ambient_yel);
				glMaterialfv(GL_BACK, GL_SPECULAR, mat_specular_yel);
				glMaterialfv(GL_BACK, GL_EMISSION, mat_emission_yel);
				mat_state = 1;
			}
			glPushMatrix();
			if (((help->mode) & 0x20)) {
				glTranslatef(mx, 0, mz);
				glScalef(0.25, 0.55, 0.25);
				glRotatef(90, 1, 0, 0);
			} else {
				glTranslatef(mx, 3.5, mz);
				glScalef(help->scalex, 4.5, help->scalez);
				glRotatef(90, 1, 0, 0);
			}
			glCallList(TDFSB_CylinderList);
			glPopMatrix();
		}
	}

	// Draw audio files
	mat_state = 0;
	for (help = root; help; help = help->next) {
		if (help->tombstone)
			continue;	// Skip files that are tombstoned

		mx = help->posx;
		mz = help->posz;
		my = help->posy;

		if ((((help->regtype) == AUDIOFILE) && ((help->mode) & 0x1F) == 0)) {
			if (!mat_state) {
				glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_grn);
				glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient_grn);
				glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular_grn);
				glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission_grn);
				glMaterialfv(GL_BACK, GL_DIFFUSE, mat_diffuse_grn);
				glMaterialfv(GL_BACK, GL_AMBIENT, mat_ambient_grn);
				glMaterialfv(GL_BACK, GL_SPECULAR, mat_specular_grn);
				glMaterialfv(GL_BACK, GL_EMISSION, mat_emission_grn);
				mat_state = 2;
			}
			glPushMatrix();
			if (((help->mode) & 0x20)) {
				glTranslatef(mx, 0, mz);
				glScalef(0.5, 0.5, 0.5);
				glRotatef(90, 1, 0, 0);
			} else {
				glTranslatef(mx, my, mz);
				glScalef(help->scalex, help->scaley, help->scalez);
				glRotatef(90, 1, 0, 0);
			}
			glCallList(TDFSB_AudioList);
			glPopMatrix();
		}
	}

	// Draw symlinks?
	mat_state = 0;
	for (help = root; help; help = help->next) {
		if (help->tombstone)
			continue;	// Skip files that are tombstoned

		mx = help->posx;
		mz = help->posz;
		my = help->posy;

		if (((help->mode) & 0x1F) >= 10) {	// Symlink check?
			if (!mat_state) {
				glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_red);
				glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient_red);
				glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular_red);
				glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission_red);
				glMaterialfv(GL_BACK, GL_DIFFUSE, mat_diffuse_red);
				glMaterialfv(GL_BACK, GL_AMBIENT, mat_ambient_red);
				glMaterialfv(GL_BACK, GL_SPECULAR, mat_specular_red);
				glMaterialfv(GL_BACK, GL_EMISSION, mat_emission_red);
				mat_state = 3;
			}
			glPushMatrix();
			if (((help->mode) & 0x20)) {
				glTranslatef(help->posx, 0.2, help->posz);
				glScalef(1.2, 1.2, 1.2);
			} else {
				glTranslatef(help->posx, help->posy + .25, help->posz);
				glScalef((help->scalex) + .25, (help->scaley) + .25, (help->scalez) + .25);
			}
			glutSolidCube(2);
			glPopMatrix();
		}
	}

	// Draw symlinks?
	mat_state = 0;
	for (help = root; help; help = help->next) {
		if (help->tombstone)
			continue;	// Skip files that are tombstoned

		mx = help->posx;
		mz = help->posz;
		my = help->posy;

		if (((help->mode) & 0x20)) {	// Symlink check?
			if (!mat_state) {
				glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_gry);
				glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient_gry);
				glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular_gry);
				glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission_gry);
				glMaterialfv(GL_BACK, GL_DIFFUSE, mat_diffuse_gry);
				glMaterialfv(GL_BACK, GL_AMBIENT, mat_ambient_gry);
				glMaterialfv(GL_BACK, GL_SPECULAR, mat_specular_gry);
				glMaterialfv(GL_BACK, GL_EMISSION, mat_emission_gry);
				mat_state = 4;
			}
			glPushMatrix();
			glTranslatef(mx, 0, mz);
			if ((help->mode) == 0x25)
				glutSolidSphere(0.5, TDFSB_BALL_DETAIL, TDFSB_BALL_DETAIL);
			glutSolidSphere(1, TDFSB_BALL_DETAIL, TDFSB_BALL_DETAIL);
			glPopMatrix();
		}
	}
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
	glMaterialfv(GL_BACK, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_BACK, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_BACK, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_BACK, GL_EMISSION, mat_emission);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glEndList();
}

static void set_filetypes(void)
{
	// These filetypes are known to be mis- or non-identified by libmagic, so we fallback to extensions for those.
	// We intentionally don't list all possible options here, because we want to have an idea of which fail to identify, and why.
	xsuff[0] = ".mp3";	// Some .mp3's are misidentified, they seem to be part of a stream and are missing headers...
	tsuff[0] = AUDIOFILE;
	xsuff[1] = ".txt";	// Some .txt's are identified as "application/data"
	tsuff[1] = TEXTFILE;
	xsuff[2] = ".mp4";	// I had an .mp4 file that was actually detected as an "ISO Media" file...
	tsuff[2] = VIDEOFILE;

	nsuff[DIRECTORY] = "DIRECTORY";
	nsuff[IMAGEFILE] = "PICTURE";
	nsuff[TEXTFILE] = "TEXT";
	nsuff[PDFFILE] = "PDF";
	nsuff[ZIPFILE] = "ZIPFILE";
	nsuff[VIDEOFILE] = "VIDEO";
	nsuff[AUDIOFILE] = "AUDIO-MP3";
	nsuff[VIDEOSOURCEFILE] = "VIDEOSOURCE";
	nsuff[UNKNOWNFILE] = "UNKNOWN";
}

static void setup_kc(void)
{
	TDFSB_KC_FLY = ' ';
	TDFSB_KC_HELP = 'h';
	TDFSB_KC_HOME = '0';
	TDFSB_KC_FS = 'f';
	TDFSB_KC_DOT = '.';
	TDFSB_KC_RELM = 'r';
	TDFSB_KC_RL = 'l';
	TDFSB_KC_CDU = 'u';
	TDFSB_KC_IMBR = 'b';
	TDFSB_KC_INFO = 'i';
	TDFSB_KC_DISP = 'y';
	TDFSB_KC_CRH = 'c';
	TDFSB_KC_FPS = 'p';
	TDFSB_KC_GCR = 'g';
	TDFSB_KC_SHD = 'm';
	TDFSB_KC_NAME = 't';
	TDFSB_KC_SORT = 'n';
	TDFSB_KC_CLASS = 'o';
	TDFSB_KC_UP = '1';
	TDFSB_KC_DOWN = '3';
	TDFSB_KC_LEFT = 'a';	// on azerty keyboards, this should be q... but we set the defaults to querty
	TDFSB_KC_RIGHT = 'd';
	TDFSB_KC_SAVE = 'x';
	TDFSB_KC_FTH = '#';
	TDFSB_KC_FORWARD = 'w';	// on azerty keyboards, this should be z... but we set the defaults to querty
	TDFSB_KC_BACKWARD = 's';
}

static void setup_help(void)
{
	help_str = (char *)malloc(1024 * sizeof(char));
	help_copy = (char *)malloc(1024 * sizeof(char));
	tmpstr = (char *)malloc(64 * sizeof(char));

	if (!(help_copy && tmpstr)) {
		printf("Malloc Failure setup_help \n");
		exit(1);
	}

	strcat(help_str, STATIC_HELP_TEXT);

	sprintf(tmpstr, "\"%c\"      filenames   \"%c\"   ground cross\n", TDFSB_KC_NAME, TDFSB_KC_GCR);
	strcat(help_str, tmpstr);

	sprintf(tmpstr, "\"%c\"      crosshair   \"%c\"        display\n", TDFSB_KC_CRH, TDFSB_KC_DISP);
	strcat(help_str, tmpstr);

	sprintf(tmpstr, "\"%c\"      dot files   \"%c\"      print FPS\n", TDFSB_KC_DOT, TDFSB_KC_FPS);
	strcat(help_str, tmpstr);

	sprintf(tmpstr, "\"%c\" rel./get mouse   \"%c\"     fullscreen\n", TDFSB_KC_RELM, TDFSB_KC_FS);
	strcat(help_str, tmpstr);

	sprintf(tmpstr, "\"%c\"     reload dir   \"%c\"   image bricks\n", TDFSB_KC_RL, TDFSB_KC_IMBR);
	strcat(help_str, tmpstr);

	sprintf(tmpstr, "\"%c\"           cd..   \"%c\"      alphasort\n", TDFSB_KC_CDU, TDFSB_KC_SORT);
	strcat(help_str, tmpstr);

	sprintf(tmpstr, "\"%c\"        shading   \"%c\"         flying\n", TDFSB_KC_SHD, TDFSB_KC_FLY);
	strcat(help_str, tmpstr);

	sprintf(tmpstr, "\"%c\"      jump home   \"%c\"    classic nav\n", TDFSB_KC_HOME, TDFSB_KC_CLASS);
	strcat(help_str, tmpstr);

	sprintf(tmpstr, "\"%c\"    save config   \"%c\"   fps throttle\n", TDFSB_KC_SAVE, TDFSB_KC_FTH);
	strcat(help_str, tmpstr);

	sprintf(tmpstr, " \"%c|%c|%c|%c\"            Up|Down|Left|Right\n", TDFSB_KC_UP, TDFSB_KC_DOWN, TDFSB_KC_LEFT, TDFSB_KC_RIGHT);
	strcat(help_str, tmpstr);

	sprintf(tmpstr, "\"%c|%c\"                  Forward|Backward\n\n", TDFSB_KC_FORWARD, TDFSB_KC_BACKWARD);
	strcat(help_str, tmpstr);

	sprintf(tmpstr, "\"%c\"  print GL info    \"%c\" show/hide help\n", TDFSB_KC_INFO, TDFSB_KC_HELP);
	strcat(help_str, tmpstr);

}

static int setup_config(void)
{
	FILE *config;
	char conf_line[2048], conf_param[256], conf_value[1024], homefile[256];
	unsigned int x, y, zzz;

	strcpy(homefile, getenv("HOME"));
	if (homefile[strlen(homefile) - 1] == '/')
		strcat(homefile, ".3dfsb");
	else
		strcat(homefile, "/.3dfsb");

	if ((config = fopen(homefile, "r"))) {
		printf("Reading %s ...\n", homefile);

		while (fgets(conf_line, 256, config)) {
			if (strlen(conf_line) < 4) {
				continue;
			}
			x = 0;
			while (conf_line[x] == ' ' || conf_line[x] == '\t')
				x++;
			if (conf_line[x] == '#' || conf_line[x] == '\n') {
				continue;
			}

			x = 0;
			while (conf_line[x] == ' ' || conf_line[x] == '\t')
				x++;
			y = 0;
			while (conf_line[x] != ' ' && conf_line[x] != '=' && conf_line[x] != '\t' && conf_line[x] != '\n') {
				conf_param[y++] = conf_line[x++];
			}
			conf_param[y] = '\0';
			while (conf_line[x] == ' ' || conf_line[x] == '=' || conf_line[x] == '\t')
				x++;
			y = 0;
			while (conf_line[x] != '#' && conf_line[x] != '\n') {
				conf_value[y++] = conf_line[x++];
			}
/*                        while(conf_line[x]!=' '&&conf_line[x]!='\t'&&conf_line[x]!='\n') { conf_value[y++]=conf_line[x++]; }
*/
			conf_value[y] = '\0';
			zzz = x;

			if (strlen(conf_param) < 1 || strlen(conf_value) < 1)
				continue;

			for (x = 0; x < paracnt; x++)
				if (!strncmp(param[x], conf_param, strlen(param[x]))) {
					if (param_type[x] == 1) {
						*(int *)param_value[x] = atoi(conf_value);
						printf(" * Read \"%d\" for %s\n", *(int *)param_value[x], param[x]);
					} else if (param_type[x] == 2) {
						while (conf_value[strlen(conf_value) - 1] == ' ' || conf_value[strlen(conf_value) - 1] == '\t')
							conf_value[strlen(conf_value) - 1] = '\0';
						strcpy((char *)param_value[x], conf_value);
						printf(" * Read \"%s\" for %s\n", (char *)param_value[x], param[x]);
					} else if (param_type[x] == 3) {
						*(GLfloat *) param_value[x] = (GLfloat) atof(conf_value);
						printf(" * Read \"%f\" for %s\n", *(GLfloat *) param_value[x], param[x]);
					} else if (param_type[x] == 4) {
						if ((strstr(conf_value, "yes")) || (strstr(conf_value, "on")) || (strstr(conf_value, "1")) || (strstr(conf_value, "true")))
							*(GLint *) param_value[x] = 1;
						else
							*(GLint *) param_value[x] = 0;
						printf(" * Read \"%d\" for %s\n", *(GLint *) param_value[x], param[x]);
					} else if (param_type[x] == 5) {
						if (strlen(conf_value) == 3) {
							if (conf_value[0] == '"' && conf_value[2] == '"' && isgraph(conf_value[1]))
								for (x = 0; x < TDFSB_KEYLIST_NUM; x++)
									if (!strcmp(TDFSB_KEYLIST_NAMES[x], conf_param)) {
										*TDFSB_KEYLIST[x] = conf_value[1];
										printf(" * Read \"%c\" for %s\n", *TDFSB_KEYLIST[x], TDFSB_KEYLIST_NAMES[x]);
									}
						} else if (strlen(conf_value) == 1 && strlen(conf_line) > (zzz + 1)) {
							if (conf_value[0] == '"' && conf_line[zzz] == ' ' && conf_line[zzz + 1] == '"')
								for (x = 0; x < TDFSB_KEYLIST_NUM; x++)
									if (!strcmp(TDFSB_KEYLIST_NAMES[x], conf_param)) {
										*TDFSB_KEYLIST[x] = ' ';
										printf(" * Read \"%c\" for %s\n", *TDFSB_KEYLIST[x], TDFSB_KEYLIST_NAMES[x]);
									}
						}
					}
				}
		}

		fclose(config);

		printf(" * some values may be truncated later if out of range\n");

		if (!strstr(TDFSB_CUSTOM_EXECUTE_STRING, "%s")) {
			TDFSB_CSE_FLAG = 0;
			printf("WARNING: no %%s found in custom execute string. will not insert current path or file.\n");
		} else if (strstr(&(strstr(TDFSB_CUSTOM_EXECUTE_STRING, "%s"))[2], "%s")) {
			printf("WARNING: more than one %%s found in the custom execute string! falling back to default string.\n");
			sprintf(TDFSB_CUSTOM_EXECUTE_STRING, "cd \"%%s\"; xterm&");
			TDFSB_CSE_FLAG = 1;
		} else {
			TDFSB_CSE_FLAG = 1;
		}

		if (TDFSB_BALL_DETAIL < 4)
			TDFSB_BALL_DETAIL = 4;
		if (TDFSB_FPS_CONFIG) {
			if (TDFSB_FPS_CONFIG < 1) {
				TDFSB_FPS_CONFIG = 0;
				TDFSB_US_RUN = 0;
			} else {
				TDFSB_US_RUN = 1000000 / TDFSB_FPS_CONFIG;
			}
		}

		TDFSB_FPS_REAL = TDFSB_FPS_CACHE = TDFSB_FPS_CONFIG;

		mousespeed = (GLfloat) ceilf(mousespeed);
		headspeed = (GLfloat) (ceilf(headspeed * 10) / 10);

		if (mousespeed > 20)
			mousespeed = 20;
		else if (mousespeed < 1)
			mousespeed = 1;
		if (headspeed > 2.0)
			headspeed = 2.0;
		else if (headspeed < 0.1)
			headspeed = 0.1;

		if (TDFSB_BG_R > 1.0) {
			TDFSB_BG_R = 1.0;
		} else if (TDFSB_BG_R < 0.0) {
			TDFSB_BG_R = 0.0;
		}
		if (TDFSB_BG_G > 1.0) {
			TDFSB_BG_G = 1.0;
		} else if (TDFSB_BG_G < 0.0) {
			TDFSB_BG_G = 0.0;
		}
		if (TDFSB_BG_B > 1.0) {
			TDFSB_BG_B = 1.0;
		} else if (TDFSB_BG_B < 0.0) {
			TDFSB_BG_B = 0.0;
		}
		if (TDFSB_GG_R > 1.0) {
			TDFSB_GG_R = 1.0;
		} else if (TDFSB_GG_R < 0.0) {
			TDFSB_GG_R = 0.0;
		}
		if (TDFSB_GG_G > 1.0) {
			TDFSB_GG_G = 1.0;
		} else if (TDFSB_GG_G < 0.0) {
			TDFSB_GG_G = 0.0;
		}
		if (TDFSB_GG_B > 1.0) {
			TDFSB_GG_B = 1.0;
		} else if (TDFSB_GG_B < 0.0) {
			TDFSB_GG_B = 0.0;
		}
		if (TDFSB_FN_R > 1.0) {
			TDFSB_FN_R = 1.0;
		} else if (TDFSB_FN_R < 0.0) {
			TDFSB_FN_R = 0.0;
		}
		if (TDFSB_FN_G > 1.0) {
			TDFSB_FN_G = 1.0;
		} else if (TDFSB_FN_G < 0.0) {
			TDFSB_FN_G = 0.0;
		}
		if (TDFSB_FN_B > 1.0) {
			TDFSB_FN_B = 1.0;
		} else if (TDFSB_FN_B < 0.0) {
			TDFSB_FN_B = 0.0;
		}

		if (TDFSB_MAX_TEX_SIZE) {
			for (cc = 1; cc <= TDFSB_MAX_TEX_SIZE; cc *= 2) ;
			TDFSB_MAX_TEX_SIZE = cc / 2;
		}

		if (realpath(TDFSB_CURRENTPATH, &temp_trunc[0]) != &temp_trunc[0])
			if (realpath(getenv("HOME"), &temp_trunc[0]) != &temp_trunc[0])
				strcpy(&temp_trunc[0], "/");
		strcpy(TDFSB_CURRENTPATH, temp_trunc);

		for (x = 0; x < TDFSB_KEYLIST_NUM - 1; x++) {
			for (y = x + 1; y < TDFSB_KEYLIST_NUM; y++)
				if (*TDFSB_KEYLIST[x] == *TDFSB_KEYLIST[y]) {
					TDFSB_ALERT_KC = 1000;
					printf("MALFORMED KEYBOARD MAP: %s conflicts with %s\n", TDFSB_KEYLIST_NAMES[x], TDFSB_KEYLIST_NAMES[y]);
				}
		}

		if (TDFSB_ALERT_KC)
			setup_kc();

		return (0);
	} else {
		printf("couldn't open config file\n");
		printf("creating config file\n");
		if ((config = fopen(homefile, "w"))) {
			fprintf(config, "# 3DFSB Example Config File\n\n");
			for (x = 0; x < paracnt; x++)
				if (param_type[x] != 5)
					fprintf(config, "%-18s = %-6s # %s\n", param[x], pdef[x], tdfsb_comment[x]);

			fprintf(config, "\n# Key bindings\n\n");
			for (x = 0; x < TDFSB_KEYLIST_NUM; x++)
				fprintf(config, "%-18s = \"%c\"\n", TDFSB_KEYLIST_NAMES[x], *TDFSB_KEYLIST[x]);

			fclose(config);
		} else
			printf("couldn't create config file\n");

		return (1);
	}
}

static void save_config(void)
{
	FILE *config;
	char homefile[256];
	unsigned int x;

	strcpy(homefile, getenv("HOME"));
	if (homefile[strlen(homefile) - 1] == '/')
		strcat(homefile, ".3dfsb");
	else
		strcat(homefile, "/.3dfsb");

	if ((config = fopen(homefile, "w"))) {
		fprintf(config, "# TDFSB Saved Config File\n\n");
		for (x = 0; x < paracnt; x++) {
			if (param_type[x] == 1)
				fprintf(config, "%-18s = %-6d \t# %s\n", param[x], *(int *)param_value[x], tdfsb_comment[x]);
			else if (param_type[x] == 2)
				fprintf(config, "%-18s = %s \t# %s\n", param[x], (char *)param_value[x], tdfsb_comment[x]);
			else if (param_type[x] == 3)
				fprintf(config, "%-18s = %f \t# %s\n", param[x], *(GLfloat *) param_value[x], tdfsb_comment[x]);
			else if (param_type[x] == 4)
				fprintf(config, "%-18s = %s \t# %s\n", param[x], *(int *)param_value[x] ? "yes" : "no", tdfsb_comment[x]);
		}

		fprintf(config, "\n# Key bindings\n\n");
		for (x = 0; x < TDFSB_KEYLIST_NUM; x++)
			fprintf(config, "%-18s = \"%c\"\n", TDFSB_KEYLIST_NAMES[x], *TDFSB_KEYLIST[x]);

		fclose(config);
	} else
		printf("couldn't create config file\n");

}

static void viewm(void)
{
	if ((centY = (asin(-tposy)) * (((double)SWY) / PI)) > SWY / 2)
		centY = SWY / 2;
	else if (centY < (-SWY / 2))
		centY = -SWY / 2;

	if (tposz > 0)
		centX = (GLdouble) acos(tposx / ((GLdouble) cos((((double)centY) / (double)((double)SWY / PI))))) * (GLdouble) ((GLdouble) SWX / mousesense / PI);
	else
		centX = -(GLdouble) acos(tposx / ((GLdouble) cos((((double)centY) / (double)((double)SWY / PI))))) * (GLdouble) ((GLdouble) SWX / mousesense / PI);
}

static void check_still(void)
{
	if (!(forwardkeybuf + backwardkeybuf + leftkeybuf + rightkeybuf + upkeybuf + downkeybuf))
		TDFSB_FUNC_IDLE = stillDisplay;
}

static void stop_move(void)
{
	forwardkeybuf = backwardkeybuf = leftkeybuf = rightkeybuf = upkeybuf = downkeybuf = 0;
}

static Uint32 fps_timer(void)
{
	sprintf(fpsbuf, "FPS: %d", (int)((1000 * TDFSB_FPS_DISP) / TDFSB_FPS_DT) - 1);
	TDFSB_FPS_DISP = 0;
	return (TDFSB_FPS_DT);
}

static void init(void)
{
	unsigned int charpos;

	TDFSB_DisplayLists = glGenLists(3);
	TDFSB_CylinderList = TDFSB_DisplayLists + 0;
	TDFSB_HelpList = TDFSB_DisplayLists + 1;
	TDFSB_AudioList = TDFSB_DisplayLists + 2;
	TDFSB_SolidList = glGenLists(1);
	TDFSB_BlendList = glGenLists(1);

/* building audio object */
	aua2 = gluNewQuadric();
	gluQuadricDrawStyle(aua2, GLU_FILL);
	gluQuadricNormals(aua2, GLU_SMOOTH);
	glNewList(TDFSB_AudioList, GL_COMPILE);
	glTranslatef(0.0, 0.0, 0.0);
	glRotatef(90, 1.0, 0.0, 0.0);
	gluCylinder(aua2, 0.2, 1.0, 0.5, 16, 1);
	glRotatef(180, 0.0, 1.0, 0.0);
	gluCylinder(aua2, 0.2, 1.0, 0.5, 16, 1);
	glEndList();
	gluDeleteQuadric(aua2);

/* building  Textfilecylinder */
	aua1 = gluNewQuadric();
	gluQuadricDrawStyle(aua1, GLU_FILL);
	gluQuadricNormals(aua1, GLU_SMOOTH);
	glNewList(TDFSB_CylinderList, GL_COMPILE);
	glTranslatef(0.0, 0.0, -1.0);
	gluCylinder(aua1, 0.6, 0.6, 2.0, 16, 1);
	glTranslatef(0.0, 0.0, 2.0);
	gluDisk(aua1, 0.0, 0.6, 16, 1);
	glTranslatef(0.0, 0.0, -2.0);
	gluDisk(aua1, 0.0, 0.6, 16, 1);
	glEndList();
	gluDeleteQuadric(aua1);

	glNewList(TDFSB_HelpList, GL_COMPILE);
	prevlen = 0;
	cnt = 0;
	strcpy(help_copy, help_str);
	tmpstr = strtok(help_copy, "\n");
	while (tmpstr != NULL) {
		cnt++;
		glTranslatef(-prevlen, -14 / 0.09, 0);
		c3 = (int)strlen(tmpstr);
		for (charpos = 0; charpos < c3; charpos++) {
			glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, tmpstr[charpos]);
		}
		prevlen = c3 * 104.76;
		tmpstr = strtok(NULL, "\n");
	}

	glEndList();

	SDL_WarpMouse(SWX / 2, SWY / 2);
	vposy = 0;
	lastposx = lastposz = vposx = vposz = -10;
	smooy = tposy = 0;
	smoox = tposx = SQF;
	smooz = tposz = SQF;
	smoou = 0;
	uposy = 0;
	viewm();

	TDFSB_TEX_NUM = 0;
	TDFSB_TEX_NAMES = NULL;

	glClearColor(TDFSB_BG_R, TDFSB_BG_G, TDFSB_BG_B, 0.0);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
	glMaterialfv(GL_BACK, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_BACK, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_BACK, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_BACK, GL_SHININESS, mat_shininess);
	glMaterialfv(GL_BACK, GL_EMISSION, mat_emission);

	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	mono = glutStrokeWidth(GLUT_STROKE_MONO_ROMAN, 'W');

	// Initialize the textures queue
	loaded_textures_queue = g_async_queue_new();
}

static void reshape(int w, int h)
{
	centY = ((((GLdouble) h) * centY) / ((GLdouble) SWY));
	centX = ((((GLdouble) w) * centX) / ((GLdouble) SWX));
	SWX = w;
	SWY = h;

	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
}

static void insert(char *value, char *linkpath, unsigned int mode, off_t size, unsigned int type, unsigned int texturewidth, unsigned int textureheight, unsigned int textureformat, unsigned int textureid, unsigned int originalwidth, unsigned int originalheight, GLfloat posx, GLfloat posy, GLfloat posz, GLfloat scalex, GLfloat scaley, GLfloat scalez)
{
	char *temp;
	struct tree_entry *help;

	temp = (char *)malloc(strlen(value) + 1);
	if (temp == NULL) {
		printf("low mem while inserting object!\n");
		ende(1);
	}
	strcpy(temp, value);

	if (root == NULL) {
		root = (struct tree_entry *)malloc(sizeof(struct tree_entry));
		if (root == NULL) {
			printf("low mem while inserting object!\n");
			ende(1);
		}
		root->name = temp;
		root->namelen = strlen(temp);
		root->linkpath = linkpath;
		root->mode = mode;
		root->regtype = type;
		root->texturewidth = texturewidth;
		root->textureheight = textureheight;
		root->textureformat = textureformat;
		root->textureid = textureid;
		root->originalwidth = originalwidth;
		root->originalheight = originalheight;
		root->tombstone = 0;
		root->posx = posx;
		root->posy = posy;
		root->posz = posz;
		root->scalex = scalex;
		root->scaley = scaley;
		root->scalez = scalez;
		root->textfilecontents = NULL;
		root->texturesurface = NULL;	// this gets filled in later
		root->size = size;
		root->next = NULL;
	} else {
		help = root;
		while (help->next)
			help = help->next;
		help->next = (struct tree_entry *)malloc(sizeof(struct tree_entry));
		if ((help->next) == NULL) {
			printf("low mem while inserting object!\n");
			ende(1);
		}
		(help->next)->name = temp;
		(help->next)->namelen = strlen(temp);
		(help->next)->linkpath = linkpath;
		(help->next)->mode = mode;
		(help->next)->regtype = type;
		(help->next)->texturewidth = texturewidth;
		(help->next)->textureheight = textureheight;
		(help->next)->textureformat = textureformat;
		(help->next)->textureid = textureid;
		(help->next)->originalwidth = originalwidth;
		(help->next)->originalheight = originalheight;
		(help->next)->tombstone = 0;
		(help->next)->posx = posx;
		(help->next)->posy = posy;
		(help->next)->posz = posz;
		(help->next)->scalex = scalex;
		(help->next)->scaley = scaley;
		(help->next)->scalez = scalez;
		(help->next)->textfilecontents = NULL;
		(help->next)->texturesurface = NULL;
		(help->next)->size = size;
		(help->next)->next = NULL;
	}
}

static char **leoscan(char *ls_path)
{
	DIR *ls_dir;
	struct dirent *ls_ent;
	char *ls_tmp, *ls_swap, **ls_entlist;
	signed long int count;
	unsigned long int ls, lsc;

	ls_dir = opendir(ls_path);

	if (!ls_dir) {
		printf("Cannot open Directory %s\n", ls_path);
		ende(1);
	}

	count = 0;
	while (readdir(ls_dir))
		count++;
	rewinddir(ls_dir);
	if (!count) {
		closedir(ls_dir);
		printf("Error Directory\n");
		ende(1);
	}
	count++;

	if (!(ls_entlist = (char **)malloc((size_t) (sizeof(char **) * count)))) {
		closedir(ls_dir);
		printf("Low Mem While Reading Directory\n");
		ende(1);
	}

	ls_entlist[--count] = NULL;
	while ((ls_ent = readdir(ls_dir))) {
		--count;
		if (count < 0) {
			closedir(ls_dir);
			printf("Directory Changed While Accessing\n");
			ende(1);
		}
		if (!(ls_tmp = (char *)malloc((size_t) 1 + (strlen(ls_ent->d_name))))) {
			closedir(ls_dir);
			printf("Low Mem While Reading Directory Entry\n");
			ende(1);
		}
		memcpy((void *)ls_tmp, (void *)&ls_ent->d_name, ((size_t) 1 + (strlen(ls_ent->d_name))));
		ls_entlist[count] = ls_tmp;
	}

	closedir(ls_dir);

	if (count) {
		printf("Directory Changed While Accessing\n");
		ende(1);
	}

	if (TDFSB_DIR_ALPHASORT) {	/* yes,yes,yes only bubblesort, will change that later... ;) */
		printf("Sorting...\n");
		count = 1;
		while (count) {
			count = 0;
			ls = 0;
			while (ls_entlist[ls + 1]) {
				lsc = 0;
				while ((lsc <= strlen(ls_entlist[ls])) && (lsc <= strlen(ls_entlist[ls + 1]))) {
					if (ls_entlist[ls][lsc] > ls_entlist[ls + 1][lsc]) {
						ls_swap = ls_entlist[ls];
						ls_entlist[ls] = ls_entlist[ls + 1];
						ls_entlist[ls + 1] = ls_swap;
						count = 1;
						break;
					} else if (ls_entlist[ls][lsc] == ls_entlist[ls + 1][lsc]) {
						lsc++;
					} else {
						break;
					}
				}
				ls++;
			}
		}
	}

	return (ls_entlist);
}

static void leodir(void)
{
	unsigned int mode = 0, temptype = 0, texturewidth = 0, textureheight = 0, textureformat = 0, textureid = 0;
	char *linkpath;
	GLfloat locpx = 0, locpy = 0, locpz = 0;
	GLfloat locsx = 1, locsy = 1, locsz = 1;	// Don't leave these uninitialized, because they are not always set for the first object (for root, so "..")
	GLfloat maxz = 0, momx = 0, momz = 0;
	GLfloat nextz = 0;
	char **entry_list, *entry;
	unsigned long int n;
	struct stat buf;
	struct tree_entry *help;

	printf("------------------DIRECTORY %s\n", TDFSB_CURRENTPATH);
	c1 = 0;

/* cleaning up */

	pthread_cancel(async_load_textures_thread_id);

	glDeleteTextures(TDFSB_TEX_NUM, TDFSB_TEX_NAMES);
	if (TDFSB_TEX_NAMES != NULL)
		free(TDFSB_TEX_NAMES);
	TDFSB_WAS_NOREAD = 0;
	TDFSB_TEX_NUM = 0;
	TDFSB_TEX_NAMES = NULL;

	cleanup_media_player();	// stop any playing media

	if (total_objects_in_grid != 0) {
		help = root;
		while (help) {
			FMptr = help;
			FCptr = help->name;
			free(FCptr);
			if (help->textfilecontents != NULL) {
				FCptr = (char *)help->textfilecontents;
				free(FCptr);
			}
			// Free up any texturesurface's that are set
			if (help->texturesurface != NULL) {
				SDL_FreeSurface(help->texturesurface);
				help->texturesurface = NULL;
			}
			if (help->linkpath != NULL) {
				FCptr = help->linkpath;
				free(FCptr);
			}
			help = help->next;
			free(FMptr);
		}
		root = NULL;
		total_objects_in_grid = 0;
	}

/* analyzing new directory */
	entry_list = leoscan(TDFSB_CURRENTPATH);
	n = 0;
	do {
		entry = entry_list[n++];
		linkpath = NULL;
		if (entry && ((TDFSB_SHOW_DOTFILES || entry[0] != '.') || !strcmp(entry, ".."))) {
			total_objects_in_grid++;
			temptype = 0;
			linkpath = NULL;
			strcpy(fullpath, TDFSB_CURRENTPATH);
			if (strlen(fullpath) > 1)
				strcat(fullpath, "/");
			strcat(fullpath, entry);
			printf("Loading: %s ", entry);
			lstat(fullpath, &buf);

/* What ist it? */
			// LSB 5 of "mode" contains a flag that says whether the file is a symlinkk or not
			// LSB 4-1 of "mode" contain an enum; 0 for regular file, 1 for directory, 2 for char, 3 for block, 4 for fifo
			if (S_ISREG(buf.st_mode) != 0)
				mode = 0;
			else if (S_ISDIR(buf.st_mode) != 0)
				mode = 1;
			else if (S_ISCHR(buf.st_mode) != 0)
				mode = 2;
			else if (S_ISBLK(buf.st_mode) != 0)
				mode = 3;
			else if (S_ISFIFO(buf.st_mode) != 0)
				mode = 4;
/*			else if (S_ISSOCK(buf.st_mode)!=0)	mode=6; removed for BeOS compatibility */
			else if (S_ISLNK(buf.st_mode) != 0) {
				cc = readlink(fullpath, yabuf, 4095);
				yabuf[cc] = '\0';
				linkpath = (char *)malloc(sizeof(char) * (strlen(yabuf) + 1));
				if (linkpath == NULL) {
					printf("low mem while detecting link\n");
					ende(1);
				}
				strcpy(linkpath, yabuf);

				printf("LINK: %s -> %s ", fullpath, yabuf);
				if (yabuf[0] != '/') {
					strcpy(fullpath, TDFSB_CURRENTPATH);
					if (strlen(fullpath) > 1)
						strcat(fullpath, "/");
					strcat(fullpath, yabuf);
				} else
					strcpy(fullpath, yabuf);
				lstat(fullpath, &buf);

				if (S_ISREG(buf.st_mode) != 0)
					mode = 0x20;
				else if (S_ISDIR(buf.st_mode) != 0)
					mode = 0x21;
				else if (S_ISCHR(buf.st_mode) != 0)
					mode = 0x22;
				else if (S_ISBLK(buf.st_mode) != 0)
					mode = 0x23;
				else if (S_ISFIFO(buf.st_mode) != 0)
					mode = 0x24;
				else if (S_ISLNK(buf.st_mode) != 0)
					mode = 0x25;
/*			                                                else if (S_ISSOCK(buf.st_mode)!=0)	mode=0x26; BeOS again */

				printf(" mode: 0x%x ", mode);
			}

/* Check Permissions */
			// Note: when there is no permission, the mode is modified again...
			if ((mode & 0x1F) == 1) {	// Is it a directory?
				if ((access(fullpath, R_OK) < 0) || (access(fullpath, X_OK) < 0)) {
					mode = ((mode & 0x1F) + 10) | (mode & 0x20);
					TDFSB_WAS_NOREAD = 1;
				}
			} else if (access(fullpath, R_OK) < 0) {
				mode = ((mode & 0x1F) + 10) | (mode & 0x20);
				TDFSB_WAS_NOREAD = 1;
			}

/* Data File Identifizierung */
			if ((mode & 0x1F) == 0) {	// Is it a regular file?
				temptype = get_file_type(fullpath);
			} else if (((mode & 0x1F) == 2) && strncmp(fullpath, PATH_DEV_V4L, strlen(PATH_DEV_V4L)) == 0) {
				printf("This is a v4l file!\n");
				temptype = VIDEOSOURCEFILE;
			} else if ((mode & 0x01) == 1) {
				temptype = DIRECTORY;
			}
			// Count the number of textures we'll need, so we can allocate them already below
			if (temptype == IMAGEFILE || temptype == VIDEOFILE || temptype == VIDEOSOURCEFILE || temptype == PDFFILE)
				TDFSB_TEX_NUM++;

			// Setting some default scale (before having read the file) and position
			// Note: locpx (posx) and locpz (posz) are calculated later on, when the grid size is determined
			if ((mode & 0x1F) == 1) {	// Is it a directory?
				locpy = 0;
			} else {
				// Setting some temporary parameters for dummy objects; the real data file loading will happen later on
				locsx = locsz = ((GLfloat) log(((double)buf.st_size / 81920) + 1)) + 1;
				locsy = ((GLfloat) log(((double)buf.st_size / 10240) + 1)) + 1;
				locpy = locsy - 1;	// vertical position of the object
			}

			insert(entry, linkpath, mode, buf.st_size, temptype, texturewidth, textureheight, textureformat, textureid, www, hhh, locpx, locpy, locpz, locsx, locsy, locsz);
			free(entry);
		}

	} while (entry);

	free(entry_list);

/* calculate object's x,z positions (y is altitude and stays untouched here) */

	total_objects_in_grid_ratio = (int)sqrt((double)total_objects_in_grid);
	help = root;
	cc = 0;
	maxz = 0;
	momx = 0;
	momz = 0;
	nextz = 0;

	unsigned int rowpos;	// position in the current row
	for (rowpos = 0; rowpos <= total_objects_in_grid_ratio; rowpos++) {
		momz += maxz + nextz + 2 * (log(help->scalez + 1)) + 4;
		maxz = 0;
		momx = 0;
		for (c1 = 0; c1 <= total_objects_in_grid_ratio; c1++) {
			if (help->scalez > maxz) {
				maxz = help->scalez;
			}
			momx = momx + (help->scalex);
			help->posx = momx;
			help->posz = momz;
			help->rasterx = c1;
			help->rasterz = rowpos;
			momx = momx + (help->scalex);
			help = help->next;
			if (!help)
				break;
			momx = momx + 2 * (log(help->scalex + 1)) + 4;	/* Wege zw den Objekte, min 1 */
		}
		if (!help)
			break;
		FMptr = help;
		nextz = 0;
		for (c1 = 0; c1 <= total_objects_in_grid_ratio; c1++) {
			if (FMptr->scalez > nextz)
				nextz = FMptr->scalez;
			FMptr = FMptr->next;
			if (!FMptr)
				break;
		}
	}

	/* Calculate Ground Grid */
	// Calculate the X and Z spacing of the grid lines depending on the maximum size of the items to display in the grid
	TDFSB_MAXX = TDFSB_MAXZ = 0;
	help = root;
	while (help) {
		if (help->posx + help->scalex > TDFSB_MAXX)
			TDFSB_MAXX = help->posx + help->scalex;
		if (help->posz + help->scalez > TDFSB_MAXZ)
			TDFSB_MAXZ = help->posz + help->scalez;
		if (help->next)
			help = help->next;
		else
			help = NULL;
	}
	TDFSB_MAXX += 10;
	TDFSB_MAXZ += 10;
	TDFSB_MINX = root->posx - 10;
	TDFSB_MINZ = root->posz - 10;
	TDFSB_STEPX = (TDFSB_MAXX - TDFSB_MINX) / 20;
	TDFSB_STEPZ = (TDFSB_MAXZ - TDFSB_MINZ) / 20;
	printf("GRIDCELLSIZE: %f|%f\n", TDFSB_STEPX, TDFSB_STEPZ);

/* Reorganize Fileobject Locations : UGLY!!! ;)	*/
	help = root;
	while (help) {
		FMptr = help;
		do {
			if (!(FMptr->next)) {
				break;
			}
			if (((FMptr->next)->rasterz) != (help->rasterz)) {
				break;
			}
			FMptr = FMptr->next;
		} while (1);
		momx = (TDFSB_MAXX - 10 - (FMptr->posx + FMptr->scalex)) / 2;
		while (help != FMptr) {
			help->posx += momx;
			help = help->next;
		}
		help->posx += momx;
		if (help->next)
			help = help->next;
		else
			help = NULL;
	}

/* Creating Texture IDs/Names (which get filled in later, in another thread, function async_load_textures()) */
	if (!(TDFSB_TEX_NAMES = (GLuint *) malloc((TDFSB_TEX_NUM + 1) * sizeof(GLuint)))) {
		printf("low mem while alloc texture names\n");
		ende(1);
	}
	glGenTextures(TDFSB_TEX_NUM, TDFSB_TEX_NAMES);

	// Assign them to the directory entry objects
	c1 = 0;
	for (help = root; help; help = help->next) {
		temptype = help->regtype;
		// TODO: this is also checked somewhere above, can't we do this in the same loop as above instead of a seperate one?
		if (temptype == IMAGEFILE || temptype == VIDEOFILE || temptype == VIDEOSOURCEFILE || temptype == PDFFILE)
			help->textureid = TDFSB_TEX_NAMES[c1++];
	}

/* Creating Display List */

	tdb_gen_list();

/* Kontrolloutput */

	help = root;
	for (help = root; help; help = help->next) {
		printf("[%d,%d] ", help->rasterx, help->rasterz);
		printf("%s %ld  ", help->name, (long int)help->size);
		if (((help->mode) & 0x1F) == 1) {
			printf("DIR");
		} else {
			printf("%d-%s-FILE", help->regtype, nsuff[help->regtype]);
		}
		printf("\n");

		// The position of the .. folder is the northpole
		if (strcmp(help->name, "..") == 0) {
			TDFSB_NORTH_X = help->posx;
			TDFSB_NORTH_Z = help->posz;
			printf(" * (%f,%f) is the Northpole.\n", TDFSB_NORTH_X, TDFSB_NORTH_Z);
		}
	}

	vposy = 0;
	lastposx = lastposz = vposx = vposz = -10;
	smooy = tposy = 0;
	smoox = tposx = SQF;
	smooz = tposz = SQF;
	smoou = 0;
	uposy = 0;
	viewm();

	strcpy(fullpath, "3dfsb: ");
	strcat(fullpath, TDFSB_CURRENTPATH);
	SDL_WM_SetCaption(fullpath, "3dfsb");

	printf("Display . files: ");
	if (TDFSB_SHOW_DOTFILES)
		printf("YES.\n");
	else
		printf("NO.\n");

	TDFSB_ANIM_COUNT = 0;
	TDFSB_ANIM_STATE = 0;

	// Start a new thread that loads the textures (of images and video files) and sets them
	int err = pthread_create(&async_load_textures_thread_id, NULL, &async_load_textures, NULL);
	if (err != 0)
		printf("Can't create thread :[%s]", strerror(err));
	else
		printf("Thread created successfully\n");

	return;
}

/* TDFSB IDLE FUNCTIONS */

static void move(void)
{
	if (upkeybuf) {
		uposy = vposy = vposy + 1 / mousespeed;
	} else if (downkeybuf) {
		uposy = vposy = vposy - 1 / mousespeed;
	}

	if (leftkeybuf) {
		vposz = vposz - tposx / mousespeed;
		vposx = vposx + tposz / mousespeed;
	} else if (rightkeybuf) {
		vposz = vposz + tposx / mousespeed;
		vposx = vposx - tposz / mousespeed;
	}

	if (forwardkeybuf) {
		vposz = vposz + tposz / mousespeed;
		vposx = vposx + tposx / mousespeed;
		if (TDFSB_MODE_FLY)
			uposy = vposy = vposy + tposy / mousespeed;
	} else if (backwardkeybuf) {
		vposz = vposz - tposz / mousespeed;
		vposx = vposx - tposx / mousespeed;
		if (TDFSB_MODE_FLY)
			uposy = vposy = vposy - tposy / mousespeed;
	}

	vposy += (uposy - vposy) / 2;

	if (vposy < 0)
		vposy = 0;

	//printf("vpos = (%f,%f,%f)\n", vposx, vposy, vposz);
	TDFSB_FUNC_DISP();
}

static void stillDisplay(void)
{
	TDFSB_FUNC_DISP();
}

static void startstillDisplay(void)
{
	if (TDFSB_CONFIG_FULLSCREEN)
		keyboard(TDFSB_KC_FS);
	//stop_move();
	TDFSB_FUNC_IDLE = stillDisplay;
	TDFSB_FUNC_DISP();
}

static void ground(void)
{
	if (TDFSB_ANIM_STATE) {
		uposy = vposy -= TDFSB_OA_DY;
	}
	if (!(--TDFSB_ANIM_COUNT)) {
		TDFSB_ANIM_STATE = 0;
		TDFSB_ANIM_COUNT = 0;
		uposy = vposy = 0;
		stop_move();
		TDFSB_FUNC_IDLE = stillDisplay;
	}
	TDFSB_FUNC_DISP();
}

/* Fly to an object */
static void approach(void)
{
	switch (TDFSB_ANIM_STATE) {
	case 1:		// Approach vposx,uposy,vposz until we are close in either dimension
		vposx = vposx + TDFSB_OA_DX;
		uposy = vposy = vposy + TDFSB_OA_DY;
		vposz = vposz + TDFSB_OA_DZ;
		if (fabs(vposx - TDFSB_OA->posx) < 0.1 && fabs(vposz - TDFSB_OA->posz) < 0.1) {
			TDFSB_ANIM_COUNT = 50;
			TDFSB_ANIM_STATE = 2;
			TDFSB_OA_DX = -tposx / 50;
			TDFSB_OA_DZ = (-1 - tposz) / 50;
			TDFSB_OA_DY = -tposy / 50;
		}
		break;

	case 2:
		if (TDFSB_OA->regtype == IMAGEFILE || TDFSB_OA->regtype == VIDEOFILE || TDFSB_OA->regtype == VIDEOSOURCEFILE || TDFSB_OA->regtype == PDFFILE) {	// If we have a texture
			if (TDFSB_ANIM_COUNT) {
				smoox += TDFSB_OA_DX;
				tposx = smoox;
				smooz += TDFSB_OA_DZ;
				tposz = smooz;
				smooy += TDFSB_OA_DY;
				tposy = smooy;
				smoou = 0;
				uposy = 0;
				centX = SWX;
				centY = 0;
				TDFSB_ANIM_COUNT--;
			} else {
				TDFSB_ANIM_COUNT = 50;
				TDFSB_ANIM_STATE = 3;
				TDFSB_OA_DZ = (TDFSB_OA->posz + TDFSB_OA->scalex + 2 - vposz) / 50;
				TDFSB_OA_DX = (TDFSB_OA->posx + TDFSB_OA->scalex + 6 - vposx) / 50;
			}
		} else if (TDFSB_OA->regtype == TEXTFILE) {	// If we don't have a texture, it might be a textfile
			TDFSB_ANIM_COUNT = 50;
			TDFSB_ANIM_STATE = 3;
			TDFSB_OA_DZ = (TDFSB_OA->posz + TDFSB_OA->scalez + 4 - vposz) / 50;
			TDFSB_OA_DX = (TDFSB_OA->posx + TDFSB_OA->scalex + 4 - vposx) / 50;
		} else if (TDFSB_OA->regtype == DIRECTORY) {
			TDFSB_ANIM_COUNT = 50;
			TDFSB_ANIM_STATE = 3;
		} else {
			TDFSB_ANIM_COUNT = 0;
			TDFSB_ANIM_STATE = 0;
			stop_move();
			TDFSB_FUNC_IDLE = stillDisplay;
		}

		break;

	case 3:
		if (TDFSB_ANIM_COUNT) {
			vposz += TDFSB_OA_DZ * (-tposz);
			vposx += TDFSB_OA_DX * (-tposx);
			smoou = 0;
			uposy = 0;
			TDFSB_ANIM_COUNT--;
		} else {
			TDFSB_ANIM_COUNT = 50;
			TDFSB_ANIM_STATE = 4;
			TDFSB_OA_DY = (TDFSB_OA->posy + TDFSB_OA->scaley / 2 - 1 - vposy) / 50;
		}
		break;

	case 4:
		if (TDFSB_ANIM_COUNT) {
			uposy = vposy += TDFSB_OA_DY;
			/* smoou=0; uposy=0; */
			if (vposy < 0) {
				TDFSB_ANIM_COUNT = 0;
				uposy = vposy = 0;
			} else
				TDFSB_ANIM_COUNT--;
		} else {
			TDFSB_ANIM_COUNT = 0;
			TDFSB_ANIM_STATE = 0;
			stop_move();
			TDFSB_FUNC_IDLE = stillDisplay;
			if (vposy < 0)
				vposy = 0;
			uposy = vposy;
		}
		break;

	case 0:
	default:
		break;
	}

	TDFSB_FUNC_DISP();

}

static void nullDisplay(void)
{

	TDFSB_FUNC_DISP();

}

static void noDisplay(void)
{
	char warpmess[] = "WARPING...";
	unsigned int charpos;

	//stop_move();
	SDL_WarpMouse(SWX / 2, SWY / 2);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, (GLfloat) SWX, 0.0, (GLfloat) SWY);
	glDisable(GL_LIGHTING);
	glTranslatef(SWX / 2 - glutStrokeLength(GLUT_STROKE_ROMAN, (unsigned char *)warpmess) * 0.075, SWY / 2, 0);
	glLineWidth(1);
	glColor3f(0.5, 1.0, 0.5);
	strcpy(fullpath, warpmess);
	c3 = (int)strlen(fullpath);
	glScalef(0.15, 0.15, 0.15);
	for (charpos = 0; charpos < c3; charpos++) {
		glutStrokeCharacter(GLUT_STROKE_ROMAN, fullpath[charpos]);
	}
	glEnable(GL_LIGHTING);
	SDL_GL_SwapBuffers();

	leodir();
	TDFSB_FUNC_DISP = display;
	TDFSB_FUNC_IDLE = stillDisplay;
}

// When a user points to and clicks on an object, the tool is applied on it
static void apply_tool_on_object(struct tree_entry *object)
{
	if (CURRENT_TOOL == TOOL_WEAPON) {
		// printf("TODO: Start some animation on the object to show it is being deleted ");
		object->tombstone = 1;	// Mark the object as deleted
	}
	// Refresh (fairly static) GLCallLists with Solids and Blends so that the tool applications will be applied
	tdb_gen_list();
}

/* The scene consists of some pre-made static elements (SolidList and BlendList)
 * on top of which some dynamic things are drawn (such as audio animation and textfile contents).
 * Textures are set and updated on the solids for displaying video. */
static void display(void)
{
	double odist, vlen, senx, seny, senz, find_dist;
	struct tree_entry *find_entry;
	struct tree_entry *object;
	unsigned int charpos;

	find_entry = NULL;
	find_dist = 10000000;

	if (TDFSB_MEDIA_FILE) {
		play_media();
		// TODO: add error handling + what if the video is finished?
	}
	// Check and try to retexture objects if needed
	struct tree_entry *object_to_retexture = g_async_queue_try_pop(loaded_textures_queue);
	// TODO: from time to time, object_to_retexture contains not NULL, but some garbage data...
	if (object_to_retexture != NULL) {
		printf("Object %s finished loading, drawing on texture ID %d\n", object_to_retexture->name, object_to_retexture->textureid);
		tdb_gen_list();	// Recalculate the blocks, because the scale of the object_to_retexture has been corrected
		if (object_to_retexture->texturesurface != NULL) {
			//printf("Texturesurface of object is not NULL, setting it...\n");

			SDL_LockSurface(object_to_retexture->texturesurface);

			/*
			   // Save the preview for debugging (or caching?) purposes
			   GError *error = NULL;
			   GdkPixbuf *pixbuf = gdk_pixbuf_new_from_data(object_to_retexture->texturesurface->pixels,
			   GDK_COLORSPACE_RGB, FALSE, 8, object_to_retexture->texturewidth, object_to_retexture->textureheight,
			   GST_ROUND_UP_4(object_to_retexture->texturewidth * 3), NULL, NULL);  // parameter 7 = rowstride
			   gdk_pixbuf_save(pixbuf, "textureimagepreview.png", "png", &error, NULL);
			   if (error != NULL) {
			   g_print("Could not save image preview to file: %s\n", error->message);
			   g_error_free(error);
			   exit(-1);
			   }
			 */

			glBindTexture(GL_TEXTURE_2D, object_to_retexture->textureid);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			glTexImage2D(GL_TEXTURE_2D, 0, (GLenum) object_to_retexture->textureformat, object_to_retexture->texturewidth, object_to_retexture->textureheight, 0, (GLenum) object_to_retexture->textureformat, GL_UNSIGNED_BYTE, object_to_retexture->texturesurface->pixels);

			SDL_UnlockSurface(object_to_retexture->texturesurface);
			SDL_FreeSurface(object_to_retexture->texturesurface);
			object_to_retexture->texturesurface = NULL;
		}
		object_to_retexture = NULL;
	}
	if (TDFSB_HAVE_MOUSE) {
		SDL_WarpMouse(SWX / 2, SWY / 2);
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_LIGHTING);
	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();
	light_position[0] = vposx;
	light_position[1] = vposy;
	light_position[2] = vposz;
	light_position[3] = 1;
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glPushMatrix();
	glCallList(TDFSB_SolidList);
	glPopMatrix();

	glDisable(GL_LIGHTING);

	spin = (int)fmod(spin, 360);
	spin++;
	c1 = 0;

	if (TDFSB_GROUND_CROSS) {
		glBegin(GL_LINES);
		glColor4f(0.3, 0.4, 0.6, 1.0);
		glVertex3f(vposx + 2, -1, vposz);
		glVertex3f(vposx - 2, -1, vposz);
		glVertex3f(vposx, -1, vposz + 2);
		glVertex3f(vposx, -1, vposz - 2);
		glEnd();
	}
	// Search for selected object
	// We keep doing this while the left mouse button is pressed, so it might be called many times per second!
	// We go through all of the objects until we find a match
	// We calculate the distance between us and the object (odist) then we do some calculation and finally find a matching object...
	for (object = root; object; object = object->next) {
		if (object->tombstone)
			continue;	// Skip files that are tombstoned

		if (TDFSB_OBJECT_SEARCH) {	// If we need to search for a selected object...
			if (!((object->mode) & 0x20))
				odist = sqrt((object->posx - vposx) * (object->posx - vposx) + (object->posy - vposy) * (object->posy - vposy) + (object->posz - vposz) * (object->posz - vposz));
			else
				odist = sqrt((object->posx - vposx) * (object->posx - vposx) + (vposy) * (vposy) + (object->posz - vposz) * (object->posz - vposz));

			if (TDFSB_KEY_FINDER) {
				if (TDFSB_KEY_FINDER == object->name[0])	// If the first letter matches...
					// If this is the first match, then set it
					if (find_entry ? (find_entry->name[0] != TDFSB_KEY_FINDER) : 1) {
						find_entry = object;
						tposx = (object->posx - vposx) / odist;
						tposz = (object->posz - vposz) / odist;
						tposy = (object->posy - vposy) / odist;
						viewm();
					}
			} else {
				vlen = sqrt((tposx * tposx) + (tposy * tposy) + (tposz * tposz));
				odist = odist / vlen;

				if (odist < find_dist) {
					senx = tposx * odist + vposx;
					seny = tposy * odist + vposy;
					senz = tposz * odist + vposz;
					if (!((object->mode) & 0x20)) {
						if ((senx > object->posx - object->scalex) && (senx < object->posx + object->scalex))
							if ((seny > object->posy - object->scaley) && (seny < object->posy + object->scaley))
								if (((senz > object->posz - object->scalez) && (senz < object->posz + object->scalez)) || ((object->regtype == IMAGEFILE || object->regtype == VIDEOFILE || object->regtype == VIDEOSOURCEFILE) && ((senz > object->posz - object->scalez - 1) && (senz < object->posz + object->scalez + 1)))) {
									find_entry = object;
									find_dist = odist;
								}
					} else {
						if ((senx > (object->posx) - 1) && (senx < (object->posx) + 1))
							if ((seny > -1) && (seny < 1))
								if ((senz > (object->posz) - 1) && (senz < (object->posz) + 1)) {
									find_entry = object;
									find_dist = odist;
								}
					}
					// If find_entry was found, then do the selected tool action on it
					if (find_entry)
						apply_tool_on_object(find_entry);
				}
			}

			if (find_entry) {
				TDFSB_OBJECT_SELECTED = find_entry;
			} else {
				TDFSB_OBJECT_SELECTED = NULL;
			}
		}
		// If the application of the tool above deleted the object, then skip all the rest
		if (object->tombstone)
			continue;	// Skip files that are tombstoned

		mx = object->posx;
		mz = object->posz;
		my = object->posy;

		if (TDFSB_FILENAMES) {
			glPushMatrix();
			if (!((object->mode) & 0x20))
				glTranslatef(mx, my + 1, mz);
			else
				glTranslatef(mx, 1.5, mz);

			if (TDFSB_FILENAMES == 1) {
				glRotatef(spin + (fmod(c1, 10) * 36), 0, 1, 0);
				if (!((object->mode) & 0x20))
					glTranslatef(object->scalex, 1.5, object->scalez);
				glRotatef(90 - 45 * (object->scalez / object->scalex), 0, 1, 0);
			} else {
				u = mx - vposx;
				v = mz - vposz;
				r = sqrt(u * u + v * v);
				if (v >= 0) {
					glRotated(fmod(135 + asin(u / r) * (180 / PI), 360), 0, 1, 0);
				} else {
					glRotated(fmod(315 - asin(u / r) * (180 / PI), 360), 0, 1, 0);
				}
				if (((object->mode) == 0x00) && ((object->regtype) == IMAGEFILE || object->regtype == PDFFILE))
					glRotatef(-45, 0, 1, 0);
				if (!((object->mode) & 0x20)) {
					glTranslatef((object->scalex) + 1, 1.5, (object->scalez) + 1);
					glRotatef(90 - 45 * (object->scalez / object->scalex), 0, 1, 0);
				} else {
					glTranslatef(0, 0.5, 0);
					glRotatef(45, 0, 1, 0);
				}
			}

			// TODO: the ".." entry's font is too small...
			glScalef(0.005, 0.005, 0.005);

			if (!((object->mode) & 0x20)) {
				// Why are these colors so high, instead of the default 1.0, 1.0, 1.0?
				//printf("setting color to %d,%d,%d\n", TDFSB_FN_R, TDFSB_FN_G, TDFSB_FN_B);
				glColor4f(TDFSB_FN_R, TDFSB_FN_G, TDFSB_FN_B, 1.0);
				glTranslatef(-glutStrokeLength(GLUT_STROKE_ROMAN, (unsigned char *)object->name) / 2, 0, 0);
				for (charpos = 0; charpos < object->namelen; charpos++)
					glutStrokeCharacter(GLUT_STROKE_ROMAN, object->name[charpos]);
			} else {
				fh = ((((GLfloat) spin) - 180) / 360);
				if (fh < 0) {
					fh = fabs(fh);
					glColor4f(1 - (fh), 1 - (fh), 1 - (fh), 1.0);
					glTranslatef(-glutStrokeLength(GLUT_STROKE_ROMAN, (unsigned char *)object->name) / 2, 0, 0);
					for (charpos = 0; charpos < object->namelen; charpos++)
						glutStrokeCharacter(GLUT_STROKE_ROMAN, object->name[charpos]);

				} else {
					fh = fabs(fh);
					glColor4f(0.85 - (fh), 1 - (fh), 1 - (fh), 1.0);
					glTranslatef(-glutStrokeLength(GLUT_STROKE_ROMAN, (unsigned char *)object->linkpath) / 2, 0, 0);
					for (charpos = 0; charpos < strlen(object->linkpath); charpos++)
						glutStrokeCharacter(GLUT_STROKE_ROMAN, object->linkpath[charpos]);
				}
			}
			glPopMatrix();
		}
/* see if we entered a sphere */
		if (((object->mode) == 1) || (((object->mode) & 0x20) && (((object->mode) & 0x1F) < 10))) {
			if (vposx > mx - 1 && vposx < mx + 1 && vposz > mz - 1 && vposz < mz + 1 && vposy < 1.5) {
				temp_trunc[0] = 0;
				strcat(TDFSB_CURRENTPATH, "/");
				strcat(TDFSB_CURRENTPATH, object->name);
				if ((object->mode != 0x21) && (object->mode & 0x20))
					strcat(TDFSB_CURRENTPATH, "/..");
				if (realpath(TDFSB_CURRENTPATH, &temp_trunc[0]) != &temp_trunc[0]) {
					printf("Cannot resolve path \"%s\".\n", TDFSB_CURRENTPATH);
					ende(1);
				} else {
					strcpy(TDFSB_CURRENTPATH, temp_trunc);
					TDFSB_FUNC_IDLE = nullDisplay;
					TDFSB_FUNC_DISP = noDisplay;
					return;
				}
			}
		}
/* animate text files */
		if ((object->regtype == TEXTFILE) && (object->textfilecontents) && ((object->mode) & 0x1F) == 0) {
			// Note: every frame, we increment this, and if it is bigger than a threshold (150),
			// we move the pointer in the file (stored in object->textureheight)
			object->textureformat = object->textureformat + 10;
			glPushMatrix();
			if (((object->mode) & 0x20)) {
				cc = 16;
				c1 = 8;
				glScalef(0.0625, 0.125, 0.0625);
			} else
				cc = c1 = 1;
			glScalef(0.005, 0.005, 0.005);
			glColor4f(1.0, 1.0, 0.0, 1.0);
			c3 = (int)strlen((char *)object->textfilecontents);
			glTranslatef((200 * mx) * cc, -100 * c1 + 1500 + object->textureformat, (200 * mz) * cc);
			u = mx - vposx;
			v = mz - vposz;
			r = sqrt(u * u + v * v);
			if (v >= 0) {
				glRotated(fmod(180 + asin(u / r) * (180 / PI), 360), 0, 1, 0);
			} else {
				glRotated(fmod(0 - asin(u / r) * (180 / PI), 360), 0, 1, 0);
			}
			glTranslatef(+mono / 2, 0, 0);
			charpos = 0;
			do {
				glTranslatef(-mono, -150, 0);
				glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, object->textfilecontents[charpos + (object->textureheight)]);
				charpos++;
			}
			while (charpos < c3 && charpos < 10);
			if (object->textureformat >= 150) {
				(object->textureheight) = (object->textureheight) + 1;
				if ((object->textureheight) >= c3 - 10)
					(object->textureheight) = 0;
				(object->textureformat) = 0;
			}
			glPopMatrix();
		}
		c1++;
	}	// end of for loop over directory entry objects

/* animate audio file */
	if (TDFSB_MEDIA_FILE && TDFSB_MEDIA_FILE->regtype == AUDIOFILE) {
		glPushMatrix();
		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_grn);
		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient_grn);
		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular_grn);
		glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission_grn);
		glMaterialfv(GL_BACK, GL_DIFFUSE, mat_diffuse_grn);
		glMaterialfv(GL_BACK, GL_AMBIENT, mat_ambient_grn);
		glMaterialfv(GL_BACK, GL_SPECULAR, mat_specular_grn);
		glMaterialfv(GL_BACK, GL_EMISSION, mat_emission_grn);
		glEnable(GL_LIGHTING);
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		if (((TDFSB_MEDIA_FILE->mode) & 0x20)) {
			glTranslatef(TDFSB_MEDIA_FILE->posx, 0.0, TDFSB_MEDIA_FILE->posz);
			glScalef(0.25, 0.25, 0.25);
		} else {
			glTranslatef(TDFSB_MEDIA_FILE->posx, TDFSB_MEDIA_FILE->posy, TDFSB_MEDIA_FILE->posz);
		}
		glRotatef(90, 1.0, 0.0, 0.0);
		glScalef(TDFSB_MEDIA_FILE->scalex * 0.6, 1 + TDFSB_MEDIA_FILE->scaley + TDFSB_MEDIA_FILE->scaley * (-(GLfloat) (abs((((int)spin) & 0xF) - 8)) / 10), TDFSB_MEDIA_FILE->scaley * 0.6);
		glCallList(TDFSB_AudioList);
		glRotatef(90, 1.0, 0.0, 0.0);
		glScalef(0.5, 0.75 * TDFSB_MEDIA_FILE->scaley * ((GLfloat) (abs((((int)spin) & 0xF) - 8)) / 10), 0.5);
		glCallList(TDFSB_AudioList);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
		glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
		glMaterialfv(GL_BACK, GL_DIFFUSE, mat_diffuse);
		glMaterialfv(GL_BACK, GL_AMBIENT, mat_ambient);
		glMaterialfv(GL_BACK, GL_SPECULAR, mat_specular);
		glMaterialfv(GL_BACK, GL_EMISSION, mat_emission);
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		glDisable(GL_LIGHTING);
		glPopMatrix();
	}

	glPushMatrix();
	glCallList(TDFSB_BlendList);
	glPopMatrix();

	// Draw cube around selected object
	if (CURRENT_TOOL == TOOL_SELECTOR && TDFSB_OBJECT_SELECTED) {
		glPushMatrix();

		if ((TDFSB_OBJECT_SELECTED->mode) & 0x20) {
			glTranslatef(TDFSB_OBJECT_SELECTED->posx, 0, TDFSB_OBJECT_SELECTED->posz);
			glScalef(1.5, 1, 1.5);
		} else {
			glTranslatef(TDFSB_OBJECT_SELECTED->posx, TDFSB_OBJECT_SELECTED->posy, TDFSB_OBJECT_SELECTED->posz);
			if (TDFSB_OBJECT_SELECTED->scalex > TDFSB_OBJECT_SELECTED->scalez)
				glScalef(TDFSB_OBJECT_SELECTED->scalex * 1.5, 0.9, TDFSB_OBJECT_SELECTED->scalex * 1.5);
			else
				glScalef(TDFSB_OBJECT_SELECTED->scalez * 1.5, 0.9, TDFSB_OBJECT_SELECTED->scalez * 1.5);
		}
		glColor4f(0.5, 1.0, 0.0, 1.0);
		glRotatef(fmod(spin, 360), 0, 1, 0);
		glutWireCube(2.0);
		glPopMatrix();
	}

	smoox += (tposx - smoox) / 2;
	smooy += (tposy - smooy) / 2;
	smooz += (tposz - smooz) / 2;

	// Draw the laser
	// This could be greatly improved, with a more realistic laser: http://codepoke.net/2011/12/27/opengl-libgdx-laser-fx/
	// and a laser gun in the view somewhere, and a better laser trajectory.
	// But hey, it's a start!
	GLfloat old_width;
	glGetFloatv(GL_LINE_WIDTH, &old_width);
	glLineWidth(3);
	// If the weapon is used and the left mouse button is clicked (TDFSB_OBJECT_SEARCH) or an object is selected (TDFSB_OBJECT_SELECTED)
	if (CURRENT_TOOL == TOOL_WEAPON && (TDFSB_OBJECT_SEARCH || TDFSB_OBJECT_SELECTED)) {
		glBegin(GL_LINES);
		glColor4f(1.0, 0.0, 0, 1.0);	// red
		// Laser starts under us, and a bit to the side, so that it seems to be coming out of our gun
		glVertex4f(vposx - 0.5, vposy - 1, vposz, 1.0f);
		glVertex4f(vposx + smoox, vposy + smooy, vposz + smooz, 1.0f);
		// This makes the line too jumpy:
		// glVertex4f(TDFSB_OBJECT_SELECTED->posx, TDFSB_OBJECT_SELECTED->posy, TDFSB_OBJECT_SELECTED->posz, 1.0f);
		// This should make the laser into an endless vector, but this doesn't this work...
		// if (!TDFSB_OBJECT_SELECTED) glVertex4f(vposx + smoox, vposy + smooy, vposz + smooz, 0.0f);
		glEnd();
	}
	glLineWidth(old_width);

/* on screen displays */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, (GLfloat) SWX, 0.0, (GLfloat) SWY);

	if (TDFSB_SHOW_CROSSHAIR) {
		if (SWX > SWY)
			fh2 = SWX * 0.025;
		else
			fh2 = SWY * 0.025;
		if (fh2 > 18)
			fh2 = 18;
		glPushMatrix();
		glColor3f(0.5, 1.0, 0.25);
		glBegin(GL_LINES);
		glVertex3f(SWX * 0.50 + fh2, SWY * 0.50, 1);
		glVertex3f(SWX * 0.50 - fh2, SWY * 0.50, 1);
		glVertex3f(SWX * 0.50, SWY * 0.50 + fh2, 1);
		glVertex3f(SWX * 0.50, SWY * 0.50 - fh2, 1);
		glEnd();
		glPopMatrix();
	}

	if (TDFSB_SHOW_DISPLAY) {
		strcpy(fullpath, TDFSB_CURRENTPATH);
		c3 = (int)strlen(fullpath);
		cc = (GLfloat) glutStrokeLength(GLUT_STROKE_ROMAN, (unsigned char *)fullpath) * 0.14;
		if (cc < (SWX / 10))
			cc = SWX / 10;
		glPushMatrix();

		if (TDFSB_SHOW_DISPLAY > 1) {
			glColor4f(0.8, 1.0, 0.8, 0.25);
			glEnable(GL_BLEND);
			glDepthMask(GL_FALSE);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glBegin(GL_QUADS);
			glVertex3f(5, 45, 1);
			glVertex3f(cc + 20 + TDFSB_XL_DISPLAY, 45, 1);
			glVertex3f(cc + 20 + TDFSB_XL_DISPLAY, 5, 1);
			glVertex3f(5, 5, 1);
			glEnd();
			if (TDFSB_SHOW_DISPLAY > 2) {
				glColor4f(0.4, 0.8, 0.6, 0.5);
				glBegin(GL_POLYGON);
				glVertex3f(25 + 15 * 1, 25 + 15 * 0, 1);
				glVertex3f(25 + 15 * 0.866, 25 + 15 * 0.5, 1);
				glVertex3f(25 + 15 * 0.5, 25 + 15 * 0.866, 1);
				glVertex3f(25 + 15 * 0, 25 + 15 * 1, 1);
				glVertex3f(25 - 15 * 0.5, 25 + 15 * 0.866, 1);
				glVertex3f(25 - 15 * 0.866, 25 + 15 * 0.5, 1);
				glVertex3f(25 - 15 * 1, 25 + 15 * 0, 1);
				glVertex3f(25 - 15 * 0.866, 25 - 15 * 0.5, 1);
				glVertex3f(25 - 15 * 0.5, 25 - 15 * 0.866, 1);
				glVertex3f(25 - 15 * 0, 25 - 15 * 1, 1);
				glVertex3f(25 + 15 * 0.5, 25 - 15 * 0.866, 1);
				glVertex3f(25 + 15 * 0.866, 25 - 15 * 0.5, 1);
				glEnd();

				glPushMatrix();
				glTranslatef(25, 25, 0);
				glRotatef(-90 - ((180 * atan2(((double)tposx), ((double)tposz))) / PI) + ((180 * atan2((((double)vposx - (double)TDFSB_NORTH_X)), (((double)vposz - (double)TDFSB_NORTH_Z)))) / PI)
					  , 0, 0, 1);
				glBegin(GL_LINES);
				glVertex3f(0, 0, 1);
				glColor4f(1.0, 1.0, 1.0, 1.0);
				glVertex3f(15 * sqrt(tposz * tposz + tposx * tposx), 0, 1);
				glEnd();
				glPopMatrix();
			}
			glDepthMask(GL_TRUE);
			glDisable(GL_BLEND);
		}

		glTranslatef(10 + TDFSB_XL_DISPLAY, 18, 1);
		glColor3f(0.4, 0.8, 0.6);
		glScalef(0.14, 0.14, 1);
		for (charpos = 0; charpos < c3; charpos++) {
			glutStrokeCharacter(GLUT_STROKE_ROMAN, fullpath[charpos]);
		}

		glPopMatrix();
	}

	if (TDFSB_SHOW_HELP) {
		glPushMatrix();
		glTranslatef(10, SWY, 0);
		glScalef(0.08, 0.08, 1);
		glColor3f(0.4, 0.8, 0.6);
		glCallList(TDFSB_HelpList);
		glPopMatrix();
	} else {
		// If an object is selected, then show it onscreen
		if (TDFSB_OBJECT_SELECTED) {
			glPushMatrix();
			glTranslatef(10, SWY - 18, 0);
			glScalef(0.12, 0.12, 1);
			glColor3f(0.5, 1.0, 0.25);
			for (charpos = 0; charpos < strlen(TDFSB_OBJECT_SELECTED->name); charpos++) {
				glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, TDFSB_OBJECT_SELECTED->name[charpos]);
			}
			glPopMatrix();
		} else if (TDFSB_SHOW_FPS) {
			glPushMatrix();
			glTranslatef(10, SWY - 18, 0);
			glScalef(0.12, 0.12, 1);
			glColor3f(0.4, 0.8, 0.6);
			for (charpos = 0; charpos < strlen(fpsbuf); charpos++) {
				glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, fpsbuf[charpos]);
			}
			glPopMatrix();
		}
		if (TDFSB_SHOW_CONFIG_FPS) {
			glPushMatrix();
			glTranslatef(10, SWY - 36, 0);
			glScalef(0.12, 0.12, 1);
			glColor3f(0.4, 0.8, 0.6);
			for (charpos = 0; charpos < strlen(cfpsbuf); charpos++) {
				glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, cfpsbuf[charpos]);
			}
			glPopMatrix();
			TDFSB_SHOW_CONFIG_FPS--;
		}
		if (TDFSB_SHOW_THROTTLE) {
			glPushMatrix();
			glTranslatef(10, SWY - 54, 0);
			glScalef(0.12, 0.12, 1);
			glColor3f(0.4, 0.8, 0.6);
			for (charpos = 0; charpos < strlen(throttlebuf); charpos++) {
				glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, throttlebuf[charpos]);
			}
			glPopMatrix();
			TDFSB_SHOW_THROTTLE--;
		}
		if (TDFSB_SHOW_BALL) {
			glPushMatrix();
			glTranslatef(10, SWY - 72, 0);
			glScalef(0.12, 0.12, 1);
			glColor3f(0.4, 0.8, 0.6);
			for (charpos = 0; charpos < strlen(ballbuf); charpos++) {
				glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, ballbuf[charpos]);
			}
			glPopMatrix();
			TDFSB_SHOW_BALL--;
		}
		if (TDFSB_FLY_DISPLAY) {
			glPushMatrix();
			glTranslatef(SWX - 104.76 * 11 * 0.1, SWY - 18, 0);
			glScalef(0.10, 0.10, 1);
			glColor3f(0.6, 0.4, 0.0);
			for (charpos = 0; charpos < 11; charpos++) {
				glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, flybuf[charpos]);
			}
			glPopMatrix();
			TDFSB_FLY_DISPLAY--;
		}
		if (TDFSB_CLASSIC_DISPLAY) {
			glPushMatrix();
			glTranslatef(SWX - 104.76 * 12 * 0.1, SWY - 36, 0);
			glScalef(0.10, 0.10, 1);
			glColor3f(0.5, 0.5, 0.0);
			for (charpos = 0; charpos < 11; charpos++) {
				glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, classicbuf[charpos]);
			}
			glPopMatrix();
			TDFSB_CLASSIC_DISPLAY--;
		}
	}

	if (TDFSB_SPEED_DISPLAY) {
		glColor4f(0.8, 1.0, 0.8, 0.25);
		glPushMatrix();
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBegin(GL_QUADS);
		glVertex3f(SWX - 5 - 4 * 20, 50, 1);
		glVertex3f(SWX - 5, 50, 1);
		glVertex3f(SWX - 5, 30, 1);
		glVertex3f(SWX - 5 - 4 * 20, 30, 1);
		glVertex3f(SWX - 5 - 4 * 20, 25, 1);
		glVertex3f(SWX - 5, 25, 1);
		glVertex3f(SWX - 5, 5, 1);
		glVertex3f(SWX - 5 - 4 * 20, 5, 1);
		glEnd();
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		glColor3f(0.4, 0.8, 0.6);
		glBegin(GL_QUADS);
		glVertex3f(SWX - 5 - 4 * (21 - mousespeed), 50, 1);
		glVertex3f(SWX - 5, 50, 1);
		glVertex3f(SWX - 5, 30, 1);
		glVertex3f(SWX - 5 - 4 * (21 - mousespeed), 30, 1);
		glVertex3f(SWX - 5 - 4 * (10 * headspeed), 25, 1);
		glVertex3f(SWX - 5, 25, 1);
		glVertex3f(SWX - 5, 5, 1);
		glVertex3f(SWX - 5 - 4 * (10 * headspeed), 5, 1);
		glEnd();
		glTranslatef(SWX - 95 - (GLfloat) glutStrokeLength(GLUT_STROKE_ROMAN, (unsigned char *)"W") * 0.14, 32, 0);
		glScalef(0.14, 0.14, 1);
		glutStrokeCharacter(GLUT_STROKE_ROMAN, 'W');
		glTranslatef(-(GLfloat) glutStrokeLength(GLUT_STROKE_ROMAN, (unsigned char *)"H"), -25 * (1 / 0.14), 0);
		glutStrokeCharacter(GLUT_STROKE_ROMAN, 'H');
		glPopMatrix();
		TDFSB_SPEED_DISPLAY--;
	}

	if (TDFSB_ALERT_KC) {
		glPushMatrix();
		glTranslatef(SWX / 2 - 104.76 * strlen(alert_kc) * 0.1 * 0.5, SWY / 4 + 9, 0);
		glScalef(0.10, 0.10, 1);
		glColor3f(1.0, 0.25, 0.25);
		for (charpos = 0; charpos < strlen(alert_kc); charpos++) {
			glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, alert_kc[charpos]);
		}
		glPopMatrix();
		glPushMatrix();
		glTranslatef(SWX / 2 - 104.76 * strlen(alert_kc2) * 0.1 * 0.5, SWY / 4 - 9, 0);
		glScalef(0.10, 0.10, 1);
		glColor3f(1.0, 0.25, 0.25);
		for (charpos = 0; charpos < strlen(alert_kc2); charpos++) {
			glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, alert_kc2[charpos]);
		}
		glPopMatrix();
		TDFSB_ALERT_KC--;
	}

	glLoadIdentity();
	gluPerspective(60, (GLfloat) SWX / (GLfloat) SWY, 0.5, 2000);

	gluLookAt(vposx, vposy, vposz, vposx + smoox, vposy + smooy, vposz + smooz, 0.0, 1.0, 0.0);

	glFinish();
	SDL_GL_SwapBuffers();

	rtime = ttime;
	gettimeofday(&ttime, NULL);

	if (ttime.tv_sec - rtime.tv_sec)
		sec = (1000000 - rtime.tv_usec) + ttime.tv_usec;
	else
		sec = ttime.tv_usec - rtime.tv_usec;

	TDFSB_FPS_REAL = (1000000 / sec);

	if ((TDFSB_FPS_CONFIG && TDFSB_FPS_REAL) && TDFSB_FPS_REAL != TDFSB_FPS_CONFIG)
		TDFSB_US_RUN += ((1000 * 800 / TDFSB_FPS_CONFIG) - (1000 * 800 / TDFSB_FPS_REAL));

	if (TDFSB_FPS_CONFIG) {
		if (ttime.tv_sec - ltime.tv_sec)
			sec = (1000000 - ltime.tv_usec) + ttime.tv_usec;
		else
			sec = ttime.tv_usec - ltime.tv_usec;

		if (sec < TDFSB_US_RUN) {
			sec = (TDFSB_US_RUN - sec);
			wtime.tv_sec = 0;
			wtime.tv_usec = sec;
			select(0, NULL, NULL, NULL, &wtime);
		}
		gettimeofday(&ltime, NULL);
	}

	if (TDFSB_SHOW_FPS)
		TDFSB_FPS_DISP++;
}

static void MouseMove(int x, int y)
{
	if (!TDFSB_ANIM_STATE) {
		centX = centX - (GLdouble) (headspeed * (floor(((double)SWX) / 2) - (double)x));
		centY = centY - (GLdouble) (headspeed * (floor(((double)SWY) / 2) - (double)y));
		if (centY >= SWY / 2)
			centY = SWY / 2;
		if (centY <= ((-SWY) / 2))
			centY = ((-SWY) / 2);
		tposz = ((GLdouble) sin((((double)centX) / (double)(SWX / mousesense / PI)))) * ((GLdouble) cos((((double)centY) / (double)(SWY / PI))));
		tposx = ((GLdouble) cos((((double)centX) / (double)(SWX / mousesense / PI)))) * ((GLdouble) cos((((double)centY) / (double)(SWY / PI))));
		tposy = -((GLdouble) sin((((double)centY) / (double)(SWY / PI))));
	}
}

static void MouseLift(int x, int y)
{
	if (!TDFSB_ANIM_STATE) {
		uposy += ((GLfloat) ((GLfloat) (SWY / 2) - (GLfloat) y) / 25);

		centX = centX - (GLdouble) (headspeed * (floor(((double)SWX) / 2) - (double)x));
/*        	centY=centY-(GLdouble)(headspeed*(floor(((double)SWY)/2)-(double)y));*/
		if (centY >= SWY / 2)
			centY = SWY / 2;
		if (centY <= ((-SWY) / 2))
			centY = ((-SWY) / 2);
		tposz = ((GLdouble) sin((((double)centX) / (double)(SWX / mousesense / PI)))) * ((GLdouble) cos((((double)centY) / (double)(SWY / PI))));
		tposx = ((GLdouble) cos((((double)centX) / (double)(SWX / mousesense / PI)))) * ((GLdouble) cos((((double)centY) / (double)(SWY / PI))));
		tposy = -((GLdouble) sin((((double)centY) / (double)(SWY / PI))));
	}
}

static void mouse(int button, int state)
{
	if (TDFSB_ANIM_STATE)
		return;		// We don't react to mouse buttons when we are in animation state, such as flying somewhere

	switch (button) {

	case SDL_BUTTON_LEFT:
		if (!TDFSB_CLASSIC_NAV) {
			if (state == SDL_PRESSED) {
				TDFSB_OBJECT_SELECTED = NULL;
				TDFSB_OBJECT_SEARCH = 1;
				TDFSB_KEY_FINDER = 0;
				TDFSB_FUNC_KEY = keyfinder;
				TDFSB_FUNC_UPKEY = keyupfinder;
			} else {
				TDFSB_OBJECT_SELECTED = NULL;
				TDFSB_OBJECT_SEARCH = 0;
				TDFSB_KEY_FINDER = 0;
				TDFSB_FUNC_KEY = keyboard;
				TDFSB_FUNC_UPKEY = keyboardup;
			}
			break;
		} else {
			if (state == SDL_PRESSED) {
				forwardkeybuf = 1;
				backwardkeybuf = 0;
				TDFSB_FUNC_IDLE = move;
			} else {
				forwardkeybuf = 0;
				check_still();
			}
			break;
		}

	case SDL_BUTTON_RIGHT:
		if (!TDFSB_CLASSIC_NAV) {
			if (state == SDL_PRESSED && TDFSB_OBJECT_SELECTED) {
				stop_move();
				TDFSB_OA = TDFSB_OBJECT_SELECTED;
				TDFSB_OA_DX = (TDFSB_OA->posx - vposx) / 100;
				TDFSB_OA_DY = (TDFSB_OA->posy + TDFSB_OA->scaley + 4 - vposy) / 100;
				TDFSB_OA_DZ = (TDFSB_OA->posz - vposz) / 100;
				TDFSB_ANIM_STATE = 1;
				TDFSB_OBJECT_SELECTED = NULL;
				TDFSB_OBJECT_SEARCH = 0;
				TDFSB_FUNC_KEY = keyboard;
				TDFSB_FUNC_UPKEY = keyboardup;
				TDFSB_KEY_FINDER = 0;
				TDFSB_FUNC_IDLE = approach;
			}
			break;
		} else {
			if (state == SDL_PRESSED) {
				backwardkeybuf = 1;
				forwardkeybuf = 0;
				TDFSB_FUNC_IDLE = move;
			} else {
				backwardkeybuf = 0;
				check_still();
			}
			break;
		}

	case SDL_BUTTON_MIDDLE:
		if (state == SDL_PRESSED) {
			TDFSB_FUNC_MOTION = MouseLift;
			TDFSB_FUNC_IDLE = move;
		} else {
			TDFSB_FUNC_MOTION = MouseMove;
			TDFSB_FUNC_IDLE = move;
			check_still();
			uposy = vposy;
		}
		break;

	case 4:		// SDL_SCROLLUP?
		if (state == SDL_PRESSED)
			uposy = vposy = vposy + TDFSB_MW_STEPS;
		if (vposy < 0)
			vposy = uposy = 0;
		break;

	case 5:		// SDL_SCROLLDOWN?
		if (state == SDL_PRESSED)
			uposy = vposy = vposy - TDFSB_MW_STEPS;
		if (vposy < 0)
			vposy = uposy = 0;
		break;

	case 6:
		printf("No function for button 6 yet\n");
		break;

	default:
		break;
	}
}

static int speckey(int key)
{
	if (!TDFSB_ANIM_STATE) {

		// Update the fullpath variable
		strcpy(fullpath, TDFSB_CURRENTPATH);
		if (strlen(fullpath) > 1)
			strcat(fullpath, "/");
		if (TDFSB_OBJECT_SELECTED)
			strcat(fullpath, TDFSB_OBJECT_SELECTED->name);

		switch (key) {

		case SDLK_TAB:
			if (TDFSB_OBJECT_SELECTED) {
				TDFSB_OBJECT_SELECTED = NULL;
				TDFSB_OBJECT_SEARCH = 0;
				TDFSB_KEY_FINDER = 0;
				TDFSB_FUNC_KEY = keyboard;
				TDFSB_FUNC_UPKEY = keyboardup;
			}
			if (SDL_GetModState() & 0x0003)
				snprintf(TDFSB_CES_TEMP, 4096, "cd \"%s\"; xterm&", TDFSB_CURRENTPATH);
			else {
				if (TDFSB_CSE_FLAG)
					str_replace(TDFSB_CUSTOM_EXECUTE_STRING, strlen(TDFSB_CUSTOM_EXECUTE_STRING), "%s", fullpath);
				else
					strncpy(TDFSB_CES_TEMP, TDFSB_CUSTOM_EXECUTE_STRING, 4096);
			}
			system(TDFSB_CES_TEMP);
			printf("EXECUTE COMMAND: %s\n", TDFSB_CES_TEMP);
			if (TDFSB_HAVE_MOUSE) {
				TDFSB_HAVE_MOUSE = 0;
				TDFSB_FUNC_MOUSE = NULL;
				TDFSB_FUNC_MOTION = NULL;
				SDL_ShowCursor(SDL_ENABLE);
			}

			break;

		case SDLK_F1:
			if (mousespeed > 1)
				mousespeed = mousespeed - 1;
			TDFSB_SPEED_DISPLAY = 100;
			break;
		case SDLK_F2:
			if (mousespeed < 20)
				mousespeed = mousespeed + 1;
			TDFSB_SPEED_DISPLAY = 100;
			break;

		case SDLK_F4:
			headspeed = headspeed - .1;
			if (headspeed < 0.1)
				headspeed = 0.1;
			TDFSB_SPEED_DISPLAY = 100;
			break;
		case SDLK_F3:
			headspeed = headspeed + .1;
			if (headspeed > 2.0)
				headspeed = 2.0;
			TDFSB_SPEED_DISPLAY = 100;
			break;

		case SDLK_F5:
			TDFSB_BALL_DETAIL++;
			sprintf(ballbuf, "Ball Detail: %d\n", (int)TDFSB_BALL_DETAIL);
			TDFSB_SHOW_BALL = 100;
			break;
		case SDLK_F6:
			if (TDFSB_BALL_DETAIL > 4) {
				TDFSB_BALL_DETAIL--;
			}
			sprintf(ballbuf, "Ball Detail: %d\n", (int)TDFSB_BALL_DETAIL);
			TDFSB_SHOW_BALL = 100;
			break;

		case SDLK_F7:
			TDFSB_FPS_CONFIG++;
			TDFSB_US_RUN = 1000000 / TDFSB_FPS_CONFIG;
			sprintf(cfpsbuf, "MaxFPS: %d", TDFSB_FPS_CONFIG);
			if (TDFSB_FPS_CONFIG)
				strcpy(throttlebuf, "Throttle: ON");
			else
				strcpy(throttlebuf, "Throttle:OFF");
			TDFSB_SHOW_CONFIG_FPS = 100;
			break;
		case SDLK_F8:
			if (TDFSB_FPS_CONFIG > 0)
				TDFSB_FPS_CONFIG--;
			if (TDFSB_FPS_CONFIG > 0) {
				TDFSB_US_RUN = 1000000 / TDFSB_FPS_CONFIG;
			}
			sprintf(cfpsbuf, "MaxFPS: %d", TDFSB_FPS_CONFIG);
			if (TDFSB_FPS_CONFIG)
				strcpy(throttlebuf, "Throttle: ON");
			else
				strcpy(throttlebuf, "Throttle:OFF");
			TDFSB_SHOW_CONFIG_FPS = 100;
			break;
		case SDLK_F9:
			CURRENT_TOOL++;
			if (CURRENT_TOOL >= NUMBER_OF_TOOLS)
				CURRENT_TOOL = 0;
			break;

		case SDLK_HOME:
			vposy = 0;
			lastposx = lastposz = vposx = vposz = -10;
			smooy = tposy = 0;
			smoox = tposx = SQF;
			smooz = tposz = SQF;
			smoou = 0;
			uposy = 0;
			viewm();
			break;

		case SDLK_UP:
			forwardkeybuf = 1;
			backwardkeybuf = 0;
			TDFSB_FUNC_IDLE = move;
			break;
		case SDLK_DOWN:
			backwardkeybuf = 1;
			forwardkeybuf = 0;
			TDFSB_FUNC_IDLE = move;
			break;
		case SDLK_LEFT:
			leftkeybuf = 1;
			rightkeybuf = 0;
			TDFSB_FUNC_IDLE = move;
			break;
		case SDLK_RIGHT:
			rightkeybuf = 1;
			leftkeybuf = 0;
			TDFSB_FUNC_IDLE = move;
			break;

		case SDLK_PAGEUP:
			upkeybuf = 1;
			downkeybuf = 0;
			TDFSB_FUNC_IDLE = move;
			break;
		case SDLK_PAGEDOWN:
			downkeybuf = 1;
			upkeybuf = 0;
			TDFSB_FUNC_IDLE = move;
			break;
		case SDLK_END:
			TDFSB_OA_DY = vposy / 25;
			TDFSB_ANIM_COUNT = 25;
			TDFSB_ANIM_STATE = 1;
			TDFSB_FUNC_IDLE = ground;
			break;
		case SDLK_RCTRL:
			if (TDFSB_OBJECT_SELECTED) {
				stop_move();
				TDFSB_OA = TDFSB_OBJECT_SELECTED;
				TDFSB_OA_DX = (TDFSB_OA->posx - vposx) / 100;
				if (!((TDFSB_OA->mode) & 0x20)) {	// if not a symlink
					TDFSB_OA_DY = (TDFSB_OA->posy + TDFSB_OA->scaley + 4 - vposy) / 100;
				} else {
					TDFSB_OA_DY = (5 - vposy) / 100;
				}
				TDFSB_OA_DZ = (TDFSB_OA->posz - vposz) / 100;

				// Fly straight into directories
				if (TDFSB_OA->regtype == DIRECTORY) {
					TDFSB_OA_DY = ((TDFSB_OA->posy - vposy) / 100) * 2;	// Straight into the center, not above
					// Do it at double speed
					TDFSB_OA_DX = TDFSB_OA_DX * 2;
					TDFSB_OA_DZ = TDFSB_OA_DZ * 2;
				}
				TDFSB_ANIM_STATE = 1;

				TDFSB_OBJECT_SELECTED = NULL;
				TDFSB_OBJECT_SEARCH = 0;
				TDFSB_FUNC_KEY = keyboard;
				TDFSB_FUNC_UPKEY = keyboardup;
				TDFSB_KEY_FINDER = 0;
				TDFSB_FUNC_IDLE = approach;
			}
			break;
		case SDLK_RETURN:
			if (TDFSB_OBJECT_SELECTED && (TDFSB_OBJECT_SELECTED->regtype == VIDEOFILE || TDFSB_OBJECT_SELECTED->regtype == VIDEOSOURCEFILE || TDFSB_OBJECT_SELECTED->regtype == AUDIOFILE)) {
				if (TDFSB_OBJECT_SELECTED == TDFSB_MEDIA_FILE) {
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

				} else {
					cleanup_media_player();	// Stop all other playing media

					//printf("Starting GStreamer pipeline for URI %s\n", fullpath);

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
					if (TDFSB_OBJECT_SELECTED->regtype == VIDEOFILE) {
						descr = g_strdup_printf("uridecodebin uri=%s name=player ! videoconvert ! videoscale ! video/x-raw,width=%d,height=%d,format=RGB ! fakesink name=fakesink0 sync=1 player. ! audioconvert ! playsink", uri, TDFSB_OBJECT_SELECTED->texturewidth, TDFSB_OBJECT_SELECTED->textureheight);
					} else if (TDFSB_OBJECT_SELECTED->regtype == AUDIOFILE) {
						descr = g_strdup_printf("uridecodebin uri=%s ! audioconvert ! playsink", uri);
					} else if (TDFSB_OBJECT_SELECTED->regtype == VIDEOSOURCEFILE) {
						descr = g_strdup_printf("v4l2src device=%s ! videoconvert ! videoscale ! video/x-raw,width=%d,height=%d,format=RGB ! fakesink name=fakesink0 sync=1", fullpath, TDFSB_OBJECT_SELECTED->texturewidth, TDFSB_OBJECT_SELECTED->textureheight);
					}
					// Use this for pulseaudio:
					// gchar *descr = g_strdup_printf("uridecodebin uri=%s name=player ! videoconvert ! videoscale ! video/x-raw,width=256,height=256,format=RGB ! fakesink name=fakesink0 sync=1 player. ! audioconvert ! pulsesink client-name=3dfsb", uri);

					//printf("gst-launch-1.0 %s\n", descr);
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

					framecounter = 0;
					gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING);

					TDFSB_MEDIA_FILE = TDFSB_OBJECT_SELECTED;
				}
			}
			break;

		default:
			return (1);
			break;
		}
	}

	return (0);
}

static int specupkey(int key)
{
	if (!TDFSB_ANIM_STATE) {
		if (key == SDLK_UP) {
			forwardkeybuf = 0;
			check_still();
		} else if (key == SDLK_DOWN) {
			backwardkeybuf = 0;
			check_still();
		} else if (key == SDLK_LEFT) {
			leftkeybuf = 0;
			check_still();
		} else if (key == SDLK_RIGHT) {
			rightkeybuf = 0;
			check_still();
		} else if (key == SDLK_PAGEUP) {
			upkeybuf = 0;
			check_still();
		} else if (key == SDLK_PAGEDOWN) {
			downkeybuf = 0;
			check_still();
		} else
			return (1);

		return (0);
	} else
		return (0);
}

static int keyfinder(unsigned char key)
{
	TDFSB_KEY_FINDER = key;
	return (0);
}

static int keyupfinder(unsigned char key)
{
	UNUSED(key);		// Can't change the function signature because keyfinder() *does* need the key
	TDFSB_KEY_FINDER = 0;
	return (0);
}

static int keyboardup(unsigned char key)
{
	if (!TDFSB_ANIM_STATE) {
		if (key == TDFSB_KC_FORWARD) {
			forwardkeybuf = 0;
			check_still();
		} else if (key == TDFSB_KC_BACKWARD) {
			backwardkeybuf = 0;
			check_still();
		} else if (key == TDFSB_KC_LEFT) {
			leftkeybuf = 0;
			check_still();
		} else if (key == TDFSB_KC_RIGHT) {
			rightkeybuf = 0;
			check_still();
		} else if (key == TDFSB_KC_UP) {
			upkeybuf = 0;
			check_still();
		} else if (key == TDFSB_KC_DOWN) {
			downkeybuf = 0;
			check_still();
		} else
			return (1);

		return (0);
	} else
		return (0);
}

static int keyboard(unsigned char key)
{

	if (!TDFSB_ANIM_STATE) {
		if (key == 27) {
			printf("\nBye bye...\n\n");
			ende(0);
		}

		else if (key == TDFSB_KC_FLY) {
			TDFSB_MODE_FLY = 1 - TDFSB_MODE_FLY;
			if (TDFSB_MODE_FLY)
				strcpy(flybuf, "Flying: ON");
			else
				strcpy(flybuf, "Flying:OFF");
			TDFSB_FLY_DISPLAY = 100;
		}

		else if (key == TDFSB_KC_HELP) {
			TDFSB_SHOW_HELP = 1 - TDFSB_SHOW_HELP;
			if (TDFSB_SHOW_HELP) {
				printf("\n=======================================\n");
				puts(help_str);
				printf("=======================================\n\n");
			}
		}

		else if (key == TDFSB_KC_FS) {
			if (TDFSB_FULLSCREEN) {
				if ((window = SDL_SetVideoMode(aSWX, aSWY, bpp, SDL_OPENGL | SDL_RESIZABLE)) == 0) {
					printf("SDL ERROR Video mode set failed: %s\n", SDL_GetError());
					ende(1);
				}
				reshape(window->w, window->h);
				TDFSB_CONFIG_FULLSCREEN = 0;

			} else {
				aSWX = SWX;
				aSWY = SWY;
				if ((window = SDL_SetVideoMode(PWX, PWY, PWD, SDL_OPENGL | SDL_FULLSCREEN)) == 0) {
					printf("SDL ERROR Video mode set failed: %s\n", SDL_GetError());
					ende(1);
				}
				reshape(window->w, window->h);
				TDFSB_CONFIG_FULLSCREEN = 1;
			}
			TDFSB_FULLSCREEN = 1 - TDFSB_FULLSCREEN;
		}

		else if (key == TDFSB_KC_DOT) {
			TDFSB_SHOW_DOTFILES = 1 - TDFSB_SHOW_DOTFILES;
			TDFSB_FUNC_IDLE = nullDisplay;
			TDFSB_FUNC_DISP = noDisplay;
		}

		else if (key == TDFSB_KC_RELM) {
			if (TDFSB_HAVE_MOUSE) {
				TDFSB_HAVE_MOUSE = 0;
				TDFSB_FUNC_MOUSE = NULL;
				TDFSB_FUNC_MOTION = NULL;
				SDL_ShowCursor(SDL_ENABLE);
			} else {
				SDL_WarpMouse(SWX / 2, SWY / 2);
				smoox = tposx;
				smooy = tposy;
				smooz = tposz;
				TDFSB_FUNC_MOUSE = mouse;
				TDFSB_FUNC_MOTION = MouseMove;
				SDL_ShowCursor(SDL_DISABLE);
				TDFSB_HAVE_MOUSE = 1;
			}
		}

		else if (key == TDFSB_KC_RL) {
			TDFSB_FUNC_IDLE = nullDisplay;
			TDFSB_FUNC_DISP = noDisplay;
		}

		else if (key == TDFSB_KC_CDU) {
			strcat(TDFSB_CURRENTPATH, "/..");
			temp_trunc[0] = 0;
			if (realpath(TDFSB_CURRENTPATH, &temp_trunc[0]) != &temp_trunc[0]) {
				printf("Cannot resolve path \"%s\".\n", TDFSB_CURRENTPATH);
				ende(1);
			} else {
				strcpy(TDFSB_CURRENTPATH, temp_trunc);
				TDFSB_FUNC_IDLE = nullDisplay;
				TDFSB_FUNC_DISP = noDisplay;
				return 0;
			}
			TDFSB_FUNC_IDLE = nullDisplay;
			TDFSB_FUNC_DISP = noDisplay;
		}

		else if (key == TDFSB_KC_IMBR) {
			if (TDFSB_ICUBE == 1) {
				TDFSB_ICUBE = 0;
			} else {
				TDFSB_ICUBE = 1;
			}
			TDFSB_FUNC_IDLE = nullDisplay;
			TDFSB_FUNC_DISP = noDisplay;
		}

		else if (key == TDFSB_KC_INFO) {
			printf("\n");
			printf("GL_RENDERER   = %s\n", (const char *)glGetString(GL_RENDERER));
			printf("GL_VERSION    = %s\n", (const char *)glGetString(GL_VERSION));
			printf("GL_VENDOR     = %s\n", (const char *)glGetString(GL_VENDOR));
			printf("GL_EXTENSIONS = %s\n", (const char *)glGetString(GL_EXTENSIONS));
			printf("\n");
			printf("Max Texture %d x %d \n", (int)TDFSB_MAX_TEX_SIZE, (int)TDFSB_MAX_TEX_SIZE);
			printf("\n");
		}

		else if (key == TDFSB_KC_DISP) {
			if (TDFSB_SHOW_DISPLAY == 1) {
				TDFSB_SHOW_DISPLAY = 2;
				TDFSB_XL_DISPLAY = 0;
				TDFSB_SPEED_DISPLAY = 200;
			} else if (TDFSB_SHOW_DISPLAY == 2) {
				TDFSB_SHOW_DISPLAY = 3;
				TDFSB_XL_DISPLAY = 45;
				TDFSB_SPEED_DISPLAY = 200;
			} else if (TDFSB_SHOW_DISPLAY == 3) {
				TDFSB_SHOW_DISPLAY = 0;
				TDFSB_XL_DISPLAY = 0;
			} else {
				TDFSB_SHOW_DISPLAY = 1;
				TDFSB_XL_DISPLAY = 0;
				TDFSB_SPEED_DISPLAY = 200;
			}
		}

		else if (key == TDFSB_KC_CRH) {
			TDFSB_SHOW_CROSSHAIR = 1 - TDFSB_SHOW_CROSSHAIR;
		}

		else if (key == TDFSB_KC_FPS) {
			TDFSB_SHOW_FPS = 1 - TDFSB_SHOW_FPS;
			if (TDFSB_SHOW_FPS) {
				sprintf(fpsbuf, "FPS: <wait>");
				TDFSB_FPS_DISP = 0;
				TDFSB_TIMER_ID = SDL_AddTimer(TDFSB_FPS_DT, (SDL_NewTimerCallback) fps_timer, 0);
			} else {
				SDL_RemoveTimer(TDFSB_TIMER_ID);
			}
		}

		else if (key == TDFSB_KC_GCR) {
			TDFSB_GROUND_CROSS = 1 - TDFSB_GROUND_CROSS;
		}

		else if (key == TDFSB_KC_SHD) {
			if (TDFSB_SHADE == 1) {
				glShadeModel(GL_FLAT);
				TDFSB_SHADE = 0;
			} else {
				glShadeModel(GL_SMOOTH);
				TDFSB_SHADE = 1;
			}
		}

		else if (key == TDFSB_KC_NAME) {
			TDFSB_FILENAMES++;
			if (TDFSB_FILENAMES == 3)
				TDFSB_FILENAMES = 0;
		}

		else if (key == TDFSB_KC_SORT) {
			TDFSB_DIR_ALPHASORT = 1 - TDFSB_DIR_ALPHASORT;
			TDFSB_FUNC_IDLE = nullDisplay;
			TDFSB_FUNC_DISP = noDisplay;
		}

		else if (key == TDFSB_KC_HOME) {
			strcpy(TDFSB_CURRENTPATH, getenv("HOME"));
			TDFSB_FUNC_IDLE = nullDisplay;
			TDFSB_FUNC_DISP = noDisplay;
		}

		else if (key == TDFSB_KC_CLASS) {
			TDFSB_CLASSIC_NAV = 1 - TDFSB_CLASSIC_NAV;
			if (TDFSB_CLASSIC_NAV)
				strcpy(classicbuf, "Classic: ON");
			else
				strcpy(classicbuf, "Classic:OFF");
			TDFSB_CLASSIC_DISPLAY = 100;
		} else if (key == TDFSB_KC_SAVE) {
			save_config();
		}

		else if (key == TDFSB_KC_FTH) {
			if (TDFSB_FPS_CONFIG) {
				strcpy(throttlebuf, "Throttle:OFF");
				TDFSB_FPS_CACHE = TDFSB_FPS_CONFIG;
				TDFSB_FPS_CONFIG = 0;
			} else {
				strcpy(throttlebuf, "Throttle: ON");
				TDFSB_FPS_CONFIG = TDFSB_FPS_CACHE;
			}

			if (TDFSB_FPS_CONFIG)
				strcpy(throttlebuf, "Throttle: ON");
			else
				strcpy(throttlebuf, "Throttle:OFF");

			sprintf(cfpsbuf, "MaxFPS: %d", TDFSB_FPS_CONFIG);
			TDFSB_SHOW_CONFIG_FPS = 100;
			TDFSB_SHOW_THROTTLE = 100;
		}

		else if (key == TDFSB_KC_FORWARD) {
			forwardkeybuf = 1;
			backwardkeybuf = 0;
			TDFSB_FUNC_IDLE = move;
		} else if (key == TDFSB_KC_BACKWARD) {
			backwardkeybuf = 1;
			forwardkeybuf = 0;
			TDFSB_FUNC_IDLE = move;
		} else if (key == TDFSB_KC_UP) {
			upkeybuf = 1;
			downkeybuf = 0;
			TDFSB_FUNC_IDLE = move;
		} else if (key == TDFSB_KC_DOWN) {
			downkeybuf = 1;
			upkeybuf = 0;
			TDFSB_FUNC_IDLE = move;
		} else if (key == TDFSB_KC_LEFT) {
			leftkeybuf = 1;
			rightkeybuf = 0;
			TDFSB_FUNC_IDLE = move;
		} else if (key == TDFSB_KC_RIGHT) {
			rightkeybuf = 1;
			leftkeybuf = 0;
			TDFSB_FUNC_IDLE = move;
		}

		else
			return (1);

		return (0);

	} else
		return (0);
}

int main(int argc, char **argv)
{
	int fake_glut_argc;

	if (argc > 3) {
		printf("Wrong args(3).\n");
		exit(0);
	} else if (argc == 2) {
		if ((!strcmp(argv[1], "--version") || !strcmp(argv[1], "-V"))) {
			printf("TDFSB 0.0.10\n");
			exit(0);
		} else {
			printf("Wrong args(1).\n");
			exit(0);
		}
	}

	SWX = SWY = PWX = PWY = 0;
	TDFSB_CURRENTPATH[0] = 0;

	fake_glut_argc = 1;
	glutInit(&fake_glut_argc, argv);

	set_filetypes();
	setup_kc();
	if (setup_config())
		setup_config();
	setup_help();

	if (argc == 3) {
		if ((!strcmp(argv[1], "--dir") || !strcmp(argv[1], "-D"))) {
			if (realpath(argv[2], &temp_trunc[0]) != &temp_trunc[0])
				if (realpath(getenv("HOME"), &temp_trunc[0]) != &temp_trunc[0])
					strcpy(&temp_trunc[0], "/");
			strcpy(TDFSB_CURRENTPATH, temp_trunc);
		} else {
			printf("Wrong args(2).\n");
			exit(0);
		}
	}

	if (strlen(TDFSB_CURRENTPATH) < 1)
		if (realpath(getenv("HOME"), &temp_trunc[0]) != &temp_trunc[0])
			strcpy(&temp_trunc[0], "/");
	strcpy(TDFSB_CURRENTPATH, temp_trunc);

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
		printf("SDL ERROR Video initialization failed: %s\n", SDL_GetError());
		ende(1);
	}
	// Init GStreamer
	gst_init(&argc, &argv);

	// Init mimetype database
	magic = magic_open(MAGIC_MIME_TYPE);
	magic_load(magic, NULL);

	info = SDL_GetVideoInfo();
	if (!info) {
		printf("SDL ERROR Video query failed: %s\n", SDL_GetError());
		ende(1);
	}

	bpp = info->vfmt->BitsPerPixel;
	if (!PWD)
		PWD = bpp;
	switch (bpp) {
	case 8:
		rgb_size[0] = 2;
		rgb_size[1] = 3;
		rgb_size[2] = 3;
		break;
	case 15:
	case 16:
		rgb_size[0] = 5;
		rgb_size[1] = 5;
		rgb_size[2] = 5;
		break;
	default:
		rgb_size[0] = 8;
		rgb_size[1] = 8;
		rgb_size[2] = 8;
		break;
	}

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, rgb_size[0]);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, rgb_size[1]);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, rgb_size[2]);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	if ((window = SDL_SetVideoMode(SWX, SWY, bpp, SDL_OPENGL | SDL_RESIZABLE)) == 0) {
		printf("SDL ERROR Video mode set failed: %s\n", SDL_GetError());
		ende(1);
	}

	SDL_WarpMouse(SWX / 2, SWY / 2);

	SDL_EnableUNICODE(1);
	SDL_EnableKeyRepeat(0, 0);
	SDL_ShowCursor(SDL_DISABLE);

	TDFSB_FUNC_IDLE = startstillDisplay;
	TDFSB_FUNC_DISP = display;
	TDFSB_FUNC_MOTION = MouseMove;
	TDFSB_FUNC_MOUSE = mouse;
	TDFSB_FUNC_KEY = keyboard;
	TDFSB_FUNC_UPKEY = keyboardup;

	GLint max_texture_size;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);	// OpenGL does not have a glGetUIntegerv(), which would be more correct
	if (TDFSB_MAX_TEX_SIZE > (unsigned int)max_texture_size || TDFSB_MAX_TEX_SIZE == 0)
		TDFSB_MAX_TEX_SIZE = max_texture_size;

	glViewport(0, 0, SWX, SWY);

	init();

	leodir();

	while (1) {
		TDFSB_FUNC_IDLE();
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				ende(0);
				break;
			case SDL_VIDEORESIZE:
				if ((window = SDL_SetVideoMode(event.resize.w, event.resize.h, bpp, SDL_OPENGL | SDL_RESIZABLE)) == 0) {
					printf("SDL ERROR Video mode set failed: %s\n", SDL_GetError());
					ende(1);
				}
				reshape(event.resize.w, event.resize.h);
				break;
			case SDL_MOUSEMOTION:
				if (TDFSB_FUNC_MOTION)
					TDFSB_FUNC_MOTION(event.motion.x, event.motion.y);
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				if (TDFSB_FUNC_MOUSE)
					TDFSB_FUNC_MOUSE(event.button.button, event.button.state);
				break;
			case SDL_KEYDOWN:
				if (speckey(event.key.keysym.sym))
					TDFSB_FUNC_KEY((unsigned char)(event.key.keysym.unicode & 0x7F));
				break;
			case SDL_KEYUP:
				if (specupkey(event.key.keysym.sym))
					TDFSB_FUNC_UPKEY((unsigned char)(event.key.keysym.sym & 0x7F));		// no unicode for key up's
				break;
			default:
				break;
			}
		}
	}

	return (0);
}
