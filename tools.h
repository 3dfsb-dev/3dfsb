#ifndef TOOLS_H
#define TOOLS_H

#define NUMBER_OF_TOOLS 3
#define TOOL_SELECTOR   0
#define TOOL_OPENER     1
#define TOOL_WEAPON     2

void init_tools(void);

char *xdg_query_default(char *mimetype);

char *execute_binary(char *toexec);

#endif
