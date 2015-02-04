#ifndef TDFSB_H
#define TDFSB_H

#include <GL/gl.h>		// For GLfloat
#include <SDL.h>		// For SDL_Surface

// To fix warnings about unused variables
// Note: __attribute__ ((unused)) also works but is gcc specific
#define UNUSED(x) (void)(x)

#define PI  3.14159265
#define SQF 0.70710

#define NUMBER_OF_FILETYPES     10
#define DIRECTORY       0
#define IMAGEFILE       1
#define TEXTFILE        2
#define PDFFILE         3
#define ZIPFILE         4
#define VIDEOFILE       5
#define AUDIOFILE       6
#define VIDEOSOURCEFILE 7
#define UNKNOWNFILE     8
#define PROCESS		9

#define MIME_AUDIO      "audio/"
#define MIME_IMAGE      "image/"
#define MIME_PDF        "application/pdf"
#define MIME_TEXT       "text/"
#define MIME_VIDEO      "video/"
#define MIME_ZIP        "application/zip"

#define PATH_DEV_V4L    "/dev/video"

// Initial width of the transparent background in the left bottom of the screen
#define TDFSB_XL_DISPLAY_INIT	180

struct tree_entry {
	char *name;
	char *mimetype;
	char *openwith;		// with what to open the file, currently this is the name of the .desktop file, such as leafpad.desktop
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

typedef struct tree_entry tree_entry;

void release_mouse(void);

tree_entry *calculate_scale(tree_entry * object);

void check_standstill(void);

void move(void);
void stop_move(void);
void ground(void);
void approach(void);

void activate_object(tree_entry *object);

void viewm(void);

void reshape(int w, int h);

void ende(int code);

void nullDisplay(void);
void noDisplay(void);

Uint32 fps_timer(void);

/* Global function variables */
void (*TDFSB_FUNC_IDLE) (void), (*TDFSB_FUNC_DISP) (void);
void (*TDFSB_FUNC_MOUSE) (int button, int state), (*TDFSB_FUNC_MOTION) (int x, int y);
int (*TDFSB_FUNC_KEY) (unsigned char key);
int (*TDFSB_FUNC_UPKEY) (unsigned char key);

/*
 * Global variables
 */
int TDFSB_ANIM_STATE;	// This keeps track of the state of the "approach" action that brings you closer to an object in 4 steps
GLdouble centX, centY;
GLdouble tposx, tposy, tposz;
GLdouble vposx, vposy, vposz, uposy;
GLdouble smoox, smooy, smooz, smoou;
GLdouble lastposz, lastposx;
int forwardkeybuf, leftkeybuf, rightkeybuf, backwardkeybuf, upkeybuf, downkeybuf;

GLdouble TDFSB_OA_DX, TDFSB_OA_DY, TDFSB_OA_DZ;

int TDFSB_SHADE, TDFSB_FILENAMES, TDFSB_SHOW_DISPLAY, TDFSB_XL_DISPLAY, TDFSB_HAVE_MOUSE, TDFSB_ANIM_COUNT, TDFSB_OBJECT_SEARCH;
int TDFSB_SPEED_DISPLAY, TDFSB_SHOW_HELP, TDFSB_SHOW_FPS, TDFSB_FLY_DISPLAY, TDFSB_CLASSIC_DISPLAY;
int TDFSB_SHOW_THROTTLE, TDFSB_SHOW_BALL, TDFSB_FPS_DISP, TDFSB_FPS_DT, TDFSB_SHOW_CONFIG_FPS;

tree_entry *TDFSB_OBJECT_SELECTED;
tree_entry *INPUT_OBJECT;
tree_entry *TDFSB_OA;

/* The help text is printed out by:
 * - the input system's "help" key handler for the console out)
 * - the display() function for the on-screen display
 */
char *help_str;

unsigned char TDFSB_KEY_FINDER;

char cfpsbuf[12], throttlebuf[14], flybuf[12], classicbuf[12], ballbuf[20], fpsbuf[12];

#endif
