#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "media.h"
#include "tools.h"

/*
 * Some info:
 * ----------
 * xdg-open uses .desktop files, which can be found in /usr/share/applications/
 * Local .desktop files of the user can be found in ~/.local/share/applications/
 *
 * You can get a file's default open command with:
 * xdg-mime query default $(file --mime-type -b filename)
 *
 * You can set it with:
 * xdg-mime default file.desktop mime/type
 *
 * xdg-open saves these associations of the user in ~/.local/share/applications/mimeapps.list
 */
#define OPEN_COMMAND		"DISPLAY=:%d xdg-open "

#define XDG_QUERY_DEFAULT	"xdg-mime query default "

static void xdg_open(char *fullpath, int display_number)
{
	char command[4096];
	snprintf(command, 40, OPEN_COMMAND, display_number);
	strcat(command, "\"");
	strcat(command, fullpath);
	strcat(command, "\" &");
	printf("Executing command to open file: %s\n", command);
	system(command);
}

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

	if (CURRENT_TOOL == TOOL_OPENER) {
		// If we have an in-world handler for the file, use it
		if (object->regtype == VIDEOFILE || object->regtype == VIDEOSOURCEFILE || object->regtype == AUDIOFILE || object->regtype == PROCESS) {
			if (object == TDFSB_MEDIA_FILE) {
				toggle_media_pipeline();
			} else {
				cleanup_media_player();	// Stop all other playing media
				play_media(fullpath, object);
			}
		} else if (object->openwith) {
			cleanup_media_player();
			play_media(fullpath, object);	// Start an X server
			xdg_open(fullpath, 1);	// Start the program that opens the file
			calculate_scale(object);
			toreturn = 1;
		}
	} else if (CURRENT_TOOL == TOOL_EXTERNAL_OPENER) {
		xdg_open(fullpath, 0);
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
	CURRENT_TOOL = TOOL_OPENER;	// The tool (or weapon) we are currently holding
	tool[TOOL_OPENER] = "Open file/folder in 3D world";
	tool[TOOL_EXTERNAL_OPENER] = "Open file/folder in 2D desktop";
	tool[TOOL_WEAPON] = "Laser shooter demo (file not really deleted :-)";
}

char *xdg_query_default(const char *mimetype)
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
