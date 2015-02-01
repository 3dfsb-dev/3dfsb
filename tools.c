#include <stdio.h>

#include "tools.h"

// xdg-mime query default $(file --mime-type -b filename)
//char *execute_binary() { 
void execute_binary(void) { 

	FILE *fp;
	char path[1035];
	//char * toreturn = malloc(1024);

	/* Open the command for reading. */
	fp = popen("/bin/ls /etc/", "r");
	if (fp == NULL) {
		printf("ERROR: Failed to run the command\n" );
	}

	/* Read the output a line at a time - output it. */
	while (fgets(path, sizeof(path)-1, fp) != NULL) {
		//strcat(toreturn, path);
		printf("%s", path);
	}

	/* close */
	pclose(fp);

//	return path;

}
