#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tools.h"

#define XDG_QUERY_DEFAULT	"/usr/bin/xdg-mime query default "

char *tool[NUMBER_OF_TOOLS];

void init_tools(void)
{
	tool[TOOL_SELECTOR] = "Do something with it";
	tool[TOOL_OPENER] = "Open file";
	tool[TOOL_WEAPON] = "Laser shooter (file not deleted)";
}

char *xdg_query_default(char *mimetype) {
	char *command = malloc(sizeof(char) * 1024);

	strcpy(command,XDG_QUERY_DEFAULT);
	strcat(command,mimetype);

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
	for (i=0;i<1024;i++)
		toreturn[i] = 0;

	// xdg-mime query default $(file --mime-type -b filename)
	fp = popen(toexec, "r");
	if (fp == NULL) {
		printf("ERROR: Failed to run the command\n" );
	}

	/* Read the output a line at a time - output it. */
	while (fgets(path, sizeof(path)-1, fp) != NULL) {
		strcat(toreturn, path);
	}

	/* close */
	pclose(fp);

	return toreturn;

}
