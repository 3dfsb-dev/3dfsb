#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tools.h"

char *execute_binary(void) {

	FILE *fp;
	char path[1035] = { 0 };
	char *toreturn = malloc(sizeof(char) * 10240);

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
