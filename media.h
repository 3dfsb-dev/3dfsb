#ifndef MEDIA_H
#define MEDIA_H

#include "3dfsb.h"

struct texture_description {
	unsigned int originalwidth;
	unsigned int originalheight;
	unsigned long textureformat;
	SDL_Surface * texturesurface;
};

typedef struct texture_description texture_description;

void cleanup_media_player(void);

// Here we do that SDL_BlitSurface of SDL 2.0 does
SDL_Surface *ScaleSurface(SDL_Surface * Surface, double Width, double Height);

/* Get an image from a (video, videosource or image) file.
 *
 * First we create a pipeline, then get a sample, then get the buffer from it,
 * then map the buffer to a "loader" SDL_Surface, and then scale that into the "converter_to_return" SDL_Surface.
 *
 * Note: this uses around 300MB of memory for each high resolution image if we use textures of 8192x8192, so be careful...
 *
 * @returns NULL when there is a problem to get any image from the file.
 */
texture_description *get_image_from_file(char *filename, unsigned int filetype, unsigned int TDFSB_MAX_TEX_SIZE);

void update_media_texture(tree_entry *TDFSB_MEDIA_FILE);

void toggle_media_pipeline(void);
void play_media(char * fullpath, tree_entry * TDFSB_OBJECT_SELECTED);

#endif
