#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "media.h"
#include "tools.h"

#define XDG_QUERY_DEFAULT	"/usr/bin/xdg-mime query default "

// When a user points to and clicks on an object, the tool is applied on it
int apply_tool_on_object(struct tree_entry *object, char *currentpath)
{
	int toreturn = 0;

	// We'll need the full path
	char fullpath[4096] = { 0 };
	strcat(fullpath, currentpath);
	if (strlen(fullpath) > 1)
		strcat(fullpath, "/");
	strcat(fullpath, object->name);

	if (CURRENT_TOOL == TOOL_SELECTOR) {
		// If we have an in-world handler for the file, use it
		if (object->regtype == VIDEOFILE || object->regtype == VIDEOSOURCEFILE || object->regtype == AUDIOFILE || object->regtype == PROCESS) {
			if (object == TDFSB_MEDIA_FILE) {
				toggle_media_pipeline();
			} else {
				cleanup_media_player();	// Stop all other playing media
				play_media(fullpath, object);
				TDFSB_MEDIA_FILE = object;
			}
		}
		// TODO:
		// - if it is a binary, then start it in-world
		// - else, if we have an xdg-open result, then start that one to open it in-world
		/*
		} else if (object && object->regtype == TEXTFILE) {
			if (object == TDFSB_MEDIA_FILE) {
				// Re-connect the input
				INPUT_OBJECT = TDFSB_MEDIA_FILE;
			} else {
				cleanup_media_player();	// Stop all other playing media
				play_media(fullpath, object);
				TDFSB_MEDIA_FILE = object;
				calculate_scale(TDFSB_MEDIA_FILE);
				tdb_gen_list();	// refresh scene, because where there was a textfile, will now be a cube
			}
		}
		*/
	} else if (CURRENT_TOOL == TOOL_OPENER) {
		char command[4096];
		strcpy(command, OPEN_COMMAND);
		strcat(command, "\"");
		strcat(command, fullpath);
		strcat(command, "\" &");
		printf("Executing command to open file: %s\n", command);
		system(command);
		release_mouse();
	} else if (CURRENT_TOOL == TOOL_WEAPON) {
		// printf("TODO: Start some animation on the object to show it is being deleted ");
		object->tombstone = 1;	// Mark the object as deleted
		// Refresh (fairly static) GLCallLists with Solids and Blends so that the tool applications will be applied
		toreturn = 1;

	} else {
		printf("ERROR: invalid tool type %d cannot be applied on object %s\n", CURRENT_TOOL, object->name);
	}
	return toreturn;
}

void init_tools(void)
{
	CURRENT_TOOL = TOOL_SELECTOR;	// The tool (or weapon) we are currently holding
	tool[TOOL_SELECTOR] = "Do something with it";
	tool[TOOL_OPENER] = "Open file";
	tool[TOOL_WEAPON] = "Laser shooter (file not deleted)";
}

char *xdg_query_default(char *mimetype)
{
	char *command = malloc(sizeof(char) * 1024);

	strcpy(command, XDG_QUERY_DEFAULT);
	strcat(command, mimetype);

	char *default_program_desktop_file = execute_binary(command);

	free(command);

	return default_program_desktop_file;

}

char *execute_binary(char *toexec)
{
	FILE *fp;
	int i;
	char path[1035] = { 0 };
	char *toreturn = malloc(sizeof(char) * 1024);

	// Zero initialize (isn't there a zmalloc in user space?!)
	for (i = 0; i < 1024; i++)
		toreturn[i] = 0;

	// xdg-mime query default $(file --mime-type -b filename)
	fp = popen(toexec, "r");
	if (fp == NULL) {
		printf("ERROR: Failed to run the command\n");
	}

	/* Read the output a line at a time - output it. */
	while (fgets(path, sizeof(path) - 1, fp) != NULL) {
		strcat(toreturn, path);
	}

	/* close */
	pclose(fp);

	return toreturn;

}
