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

#define NUMBER_OF_TOOLS 3
#define TOOL_SELECTOR   0
#define TOOL_OPENER     1
#define TOOL_WEAPON     2

struct tree_entry {
	char *name;
	char *mimetype;
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

#endif
