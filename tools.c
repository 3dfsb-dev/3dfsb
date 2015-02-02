#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tools.h"

char *execute_binary(void)
{
	FILE *fp;
	int i;
	char path[1035] = { 0 };
	char *toreturn = malloc(sizeof(char) * 1024);

	// Zero initialize (isn't there a zmalloc in user space?!)
	for (i=0;i<1024;i++)
		toreturn[i] = 0;

	// xdg-mime query default $(file --mime-type -b filename)
	fp = popen("/usr/bin/xdg-mime query default text/plain", "r");
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
