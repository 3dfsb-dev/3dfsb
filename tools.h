#ifndef TOOLS_H
#define TOOLS_H

#include "3dfsb.h"

#define NUMBER_OF_TOOLS 3
#define TOOL_SELECTOR   0
#define TOOL_OPENER     1
#define TOOL_WEAPON     2

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
#define OPEN_COMMAND		"xdg-open "
#define OPEN_STRING		" will be opened with "
#define OPEN_STRING_LENGTH	21

int CURRENT_TOOL;	// The tool (or weapon) we are currently holding

char *tool[NUMBER_OF_TOOLS];

int apply_tool_on_object(struct tree_entry *object, char *currentpath);

void init_tools(void);

char *xdg_query_default(char *mimetype);

char *execute_binary(char *toexec);

#endif
