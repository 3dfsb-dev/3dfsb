#ifndef TOOLS_H
#define TOOLS_H

#include "3dfsb.h"

#define NUMBER_OF_TOOLS		3
#define TOOL_OPENER		0
#define TOOL_EXTERNAL_OPENER	1
#define TOOL_WEAPON		2

#define OPEN_STRING		" will be opened with "
#define OPEN_STRING_LENGTH	21

int CURRENT_TOOL;		// The tool (or weapon) we are currently holding

char *tool[NUMBER_OF_TOOLS];

int apply_tool_on_object(struct tree_entry *object, char *currentpath);

void init_tools(void);

char *xdg_query_default(const char *mimetype);

char *execute_binary(char *toexec);

#endif
