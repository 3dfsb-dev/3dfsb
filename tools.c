#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tools.h"

// xdg-mime query default $(file --mime-type -b filename)
char *execute_binary(void) {

	FILE *fp;
	char path[1035] = { 0 };
	char *toreturn = malloc(sizeof(char) * 10240);

	/* Open the command for reading. */
	//fp = popen("/bin/ls /etc/", "r");
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
