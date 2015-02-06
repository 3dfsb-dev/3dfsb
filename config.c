#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "config.h"

const char *TDFSB_KEYLIST_NAMES[] = { "KeyFlying", "KeyHelp", "KeyJumpHome",
	"KeyFullScreen", "KeyDotFilter", "KeyMouseRelease",
	"KeyReload", "KeyCDup", "KeyImageBricks",
	"KeyGLInfo", "KeyDisplay", "KeyCrossHair",
	"KeyFPS", "KeyGrndCross", "KeyShadeMode",
	"KeyFileNames", "KeyAlphaSort", "KeyClassicNav",
	"KeyForward", "KeyBackward",
	"KeyUp", "KeyDown", "KeyLeft",
	"KeyRight", "KeySaveConfig", "KeyFPSThrottle"
};

char *tdfsb_comment[] = { "detail level of the spheres, must be at least 4",
	"directory to start, absolute path",
	"maximum texture size, must be 2^n, 0 for maximum",
	"width of the window in pixel",
	"height of the window in pixel",
	"width fullscreen", "height fullscreen", "depth fullscreen, 0 for same as desktop",
	"R", "G components of the grids color, range 0.0 ... 1.0", "B",
	"images have a volume (toggle later)",
	"show hidden files (toggle later)",
	"sort the files alphabetically (toggle later)",
	"R", "G components of the background color, range 0.0 ... 1.0", "B",
	"start in fullscreen mode (toggle later)", "key dummy",
	"show the crosshair (toggle later)",
	"show the cross on the ground (toggle later)",
	"mouse buttons for forward and backward (toggle later)",
	"fly in the viewing direction (toggle later)",
	"throttle fps to leave cpu time for other apps (toggle later), 0 for maximum speed",
	"velocity of movement (F1/F2), range 1 ... 20",
	"velocity of rotation (F3/F4), range 0.1 ... 2.0",
	"R", "G components of the filename color, range 0.0 ... 1.0", "B",
	"units to lift up per mousewheel step",
	"custom execute string"
};

char *param[] = { "BallDetail", "StartDir", "MaxTexSize", "WindowWidth", "WindowHeight", "FullscreenWidth", "FullscreenHeight", "FullscreenDepth", "GridRed", "GridGreen",
	"GridBlue", "ImageBricks", "ShowDotFiles", "AlphaSort", "BGRed", "BGGreen", "BGBlue", "FullScreen", "Key",
	"ShowCrossHair", "ShowGroundCross", "ClassicNavigation", "FlyingMode", "MaxFPS",
	"MoveVelocity", "LookVelocity", "NameRed", "NameGreen", "NameBlue", "LiftSteps", "CustomExecuteString"
};

// Number of configuration entries
unsigned int TDFSB_KEYLIST_NUM = 26;

unsigned char *TDFSB_KEYLIST[] = { &TDFSB_KC_FLY, &TDFSB_KC_HELP, &TDFSB_KC_HOME,
	&TDFSB_KC_FS, &TDFSB_KC_DOT, &TDFSB_KC_RELEASE_MOUSE,
	&TDFSB_KC_RL, &TDFSB_KC_CDU, &TDFSB_KC_IMBR,
	&TDFSB_KC_INFO, &TDFSB_KC_DISP, &TDFSB_KC_CRH,
	&TDFSB_KC_FPS, &TDFSB_KC_GCR, &TDFSB_KC_SHD,
	&TDFSB_KC_NAME, &TDFSB_KC_SORT, &TDFSB_KC_CLASS,
	&TDFSB_KC_FORWARD, &TDFSB_KC_BACKWARD,
	&TDFSB_KC_UP, &TDFSB_KC_DOWN, &TDFSB_KC_LEFT,
	&TDFSB_KC_RIGHT, &TDFSB_KC_SAVE, &TDFSB_KC_FTH
};

void *param_value[] = { &TDFSB_BALL_DETAIL, &TDFSB_CURRENTPATH, &TDFSB_MAX_TEX_SIZE, &SWX, &SWY, &PWX, &PWY,
	&PWD, &TDFSB_GG_R, &TDFSB_GG_G, &TDFSB_GG_B,
	&TDFSB_ICUBE, &TDFSB_SHOW_DOTFILES, &TDFSB_DIR_ALPHASORT,
	&TDFSB_BG_R, &TDFSB_BG_G, &TDFSB_BG_B,
	&TDFSB_CONFIG_FULLSCREEN, NULL, &TDFSB_SHOW_CROSSHAIR, &TDFSB_GROUND_CROSS, &TDFSB_CLASSIC_NAV, &TDFSB_MODE_FLY,
	&TDFSB_FPS_CONFIG, &mousespeed, &headspeed, &TDFSB_FN_R, &TDFSB_FN_G, &TDFSB_FN_B, &TDFSB_MW_STEPS, &TDFSB_CUSTOM_EXECUTE_STRING
};

// Default values
char *pdef[] = { "20", "/", "0", "1024", "768", "1024", "768",
	"0", "0.2", "0.2",	// TDFSB_GG_R/G/B
	"0.6", "yes", "no", "yes",
	"0.0", "0.0", "0.0",	// TDFSB_BG_R/G/B
	"yes", "X", "yes", "no", "no", "yes",
	"0", "2.0", "1.3",	// FPS, mousespeed, headspeed
	"1.0", "1.0", "1.0",	// TDFSB_FB_R/G/B
	"1", "cd \"%s\"; x-terminal-emulator&"
};
int param_type[] = { 1, 2, 1, 1, 1, 1, 1, 1, 3, 3, 3, 4, 4, 4, 3, 3, 3, 4, 5, 4, 4, 4, 4, 1, 3, 3, 3, 3, 3, 1, 2 };	/* 1=int 2=string 3=float 4=boolean 5=keyboard */

unsigned int paracnt = 31;

char temp_trunc[4096];

GLint TDFSB_BALL_DETAIL = 8, TDFSB_WAS_NOREAD = 0;
unsigned int TDFSB_MAX_TEX_SIZE = 1024;	// This cause Out-Of-Memory at 8192 and beyond if I enter a directory with high resolution images or video. So 1024 is a nice default, this uses around 150MB of RAM when going into a directory with around 300 high resolution images.
GLfloat mousesense = 1.5;
GLfloat mousespeed = 1.0;	// 1-20, with 1 being maximum
GLfloat headspeed = 2.0;	// 1.1-2.0, with 2.0 being maximum

GLfloat TDFSB_GG_R = 0.2, TDFSB_GG_G = 0.2, TDFSB_GG_B = 0.6;
GLfloat TDFSB_BG_R = 0.0, TDFSB_BG_G = 0.0, TDFSB_BG_B = 0.0;
GLfloat TDFSB_FN_R = 1.0, TDFSB_FN_G = 1.0, TDFSB_FN_B = 1.0;

int TDFSB_ICUBE = 1;
int TDFSB_SHOW_DOTFILES = 0;
int TDFSB_DIR_ALPHASORT = 1;
int TDFSB_FULLSCREEN = 0;

int TDFSB_CONFIG_FULLSCREEN = 0;
int TDFSB_SHOW_CROSSHAIR = 1;
int TDFSB_GROUND_CROSS = 0;
int TDFSB_CLASSIC_NAV = 0;
int TDFSB_MODE_FLY = 1;

int TDFSB_FPS_CONFIG = 0;
int TDFSB_MW_STEPS = 1;
int TDFSB_CSE_FLAG = 1;
long TDFSB_US_RUN = 0;
int TDFSB_FPS_CACHE = 0;
int TDFSB_FPS_REAL = 0;
int TDFSB_ALERT_KC = 0;

int read_or_create_config_file(void)
{
	FILE *config;
	char conf_line[2048], conf_param[256], conf_value[1024], homefile[256];
	unsigned int x, y, zzz;

	strcpy(homefile, getenv("HOME"));
	if (homefile[strlen(homefile) - 1] == '/')
		strcat(homefile, ".3dfsb");
	else
		strcat(homefile, "/.3dfsb");

	if ((config = fopen(homefile, "r"))) {
		printf("Reading %s ...\n", homefile);

		while (fgets(conf_line, 256, config)) {
			if (strlen(conf_line) < 4) {
				continue;
			}
			x = 0;
			while (conf_line[x] == ' ' || conf_line[x] == '\t')
				x++;
			if (conf_line[x] == '#' || conf_line[x] == '\n') {
				continue;
			}

			x = 0;
			while (conf_line[x] == ' ' || conf_line[x] == '\t')
				x++;
			y = 0;
			while (conf_line[x] != ' ' && conf_line[x] != '=' && conf_line[x] != '\t' && conf_line[x] != '\n') {
				conf_param[y++] = conf_line[x++];
			}
			conf_param[y] = '\0';
			while (conf_line[x] == ' ' || conf_line[x] == '=' || conf_line[x] == '\t')
				x++;
			y = 0;
			while (conf_line[x] != '#' && conf_line[x] != '\n') {
				conf_value[y++] = conf_line[x++];
			}
			conf_value[y] = '\0';
			zzz = x;

			if (strlen(conf_param) < 1 || strlen(conf_value) < 1)
				continue;

			for (x = 0; x < paracnt; x++)
				if (!strncmp(param[x], conf_param, strlen(param[x]))) {
					if (param_type[x] == 1) {
						*(int *)param_value[x] = atoi(conf_value);
						printf(" * Read \"%d\" for %s\n", *(int *)param_value[x], param[x]);
					} else if (param_type[x] == 2) {
						while (conf_value[strlen(conf_value) - 1] == ' ' || conf_value[strlen(conf_value) - 1] == '\t')
							conf_value[strlen(conf_value) - 1] = '\0';
						strcpy((char *)param_value[x], conf_value);
						printf(" * Read \"%s\" for %s\n", (char *)param_value[x], param[x]);
					} else if (param_type[x] == 3) {
						*(GLfloat *) param_value[x] = (GLfloat) atof(conf_value);
						printf(" * Read \"%f\" for %s\n", *(GLfloat *) param_value[x], param[x]);
					} else if (param_type[x] == 4) {
						if ((strstr(conf_value, "yes")) || (strstr(conf_value, "on")) || (strstr(conf_value, "1")) || (strstr(conf_value, "true")))
							*(GLint *) param_value[x] = 1;
						else
							*(GLint *) param_value[x] = 0;
						printf(" * Read \"%d\" for %s\n", *(GLint *) param_value[x], param[x]);
					} else if (param_type[x] == 5) {
						if (strlen(conf_value) == 3) {
							if (conf_value[0] == '"' && conf_value[2] == '"' && isgraph(conf_value[1]))
								for (x = 0; x < TDFSB_KEYLIST_NUM; x++)
									if (!strcmp(TDFSB_KEYLIST_NAMES[x], conf_param)) {
										*TDFSB_KEYLIST[x] = conf_value[1];
										printf(" * Read \"%c\" for %s\n", *TDFSB_KEYLIST[x], TDFSB_KEYLIST_NAMES[x]);
									}
						} else if (strlen(conf_value) == 1 && strlen(conf_line) > (zzz + 1)) {
							if (conf_value[0] == '"' && conf_line[zzz] == ' ' && conf_line[zzz + 1] == '"')
								for (x = 0; x < TDFSB_KEYLIST_NUM; x++)
									if (!strcmp(TDFSB_KEYLIST_NAMES[x], conf_param)) {
										*TDFSB_KEYLIST[x] = ' ';
										printf(" * Read \"%c\" for %s\n", *TDFSB_KEYLIST[x], TDFSB_KEYLIST_NAMES[x]);
									}
						}
					}
				}
		}

		fclose(config);

		printf(" * some values may be truncated later if out of range\n");

		if (!strstr(TDFSB_CUSTOM_EXECUTE_STRING, "%s")) {
			TDFSB_CSE_FLAG = 0;
			printf("WARNING: no %%s found in custom execute string. will not insert current path or file.\n");
		} else if (strstr(&(strstr(TDFSB_CUSTOM_EXECUTE_STRING, "%s"))[2], "%s")) {
			printf("WARNING: more than one %%s found in the custom execute string! falling back to default string.\n");
			sprintf(TDFSB_CUSTOM_EXECUTE_STRING, "cd \"%%s\"; x-terminal-emulator&");
			TDFSB_CSE_FLAG = 1;
		} else {
			TDFSB_CSE_FLAG = 1;
		}

		if (TDFSB_BALL_DETAIL < 4)
			TDFSB_BALL_DETAIL = 4;
		if (TDFSB_FPS_CONFIG) {
			if (TDFSB_FPS_CONFIG < 1) {
				TDFSB_FPS_CONFIG = 0;
				TDFSB_US_RUN = 0;
			} else {
				TDFSB_US_RUN = 1000000 / TDFSB_FPS_CONFIG;
			}
		}

		TDFSB_FPS_REAL = TDFSB_FPS_CACHE = TDFSB_FPS_CONFIG;

		mousespeed = (GLfloat) ceilf(mousespeed);
		headspeed = (GLfloat) (ceilf(headspeed * 10) / 10);

		if (mousespeed > 20)
			mousespeed = 20;
		else if (mousespeed < 1)
			mousespeed = 1;
		if (headspeed > 2.0)
			headspeed = 2.0;
		else if (headspeed < 0.1)
			headspeed = 0.1;

		if (TDFSB_BG_R > 1.0) {
			TDFSB_BG_R = 1.0;
		} else if (TDFSB_BG_R < 0.0) {
			TDFSB_BG_R = 0.0;
		}
		if (TDFSB_BG_G > 1.0) {
			TDFSB_BG_G = 1.0;
		} else if (TDFSB_BG_G < 0.0) {
			TDFSB_BG_G = 0.0;
		}
		if (TDFSB_BG_B > 1.0) {
			TDFSB_BG_B = 1.0;
		} else if (TDFSB_BG_B < 0.0) {
			TDFSB_BG_B = 0.0;
		}
		if (TDFSB_GG_R > 1.0) {
			TDFSB_GG_R = 1.0;
		} else if (TDFSB_GG_R < 0.0) {
			TDFSB_GG_R = 0.0;
		}
		if (TDFSB_GG_G > 1.0) {
			TDFSB_GG_G = 1.0;
		} else if (TDFSB_GG_G < 0.0) {
			TDFSB_GG_G = 0.0;
		}
		if (TDFSB_GG_B > 1.0) {
			TDFSB_GG_B = 1.0;
		} else if (TDFSB_GG_B < 0.0) {
			TDFSB_GG_B = 0.0;
		}
		if (TDFSB_FN_R > 1.0) {
			TDFSB_FN_R = 1.0;
		} else if (TDFSB_FN_R < 0.0) {
			TDFSB_FN_R = 0.0;
		}
		if (TDFSB_FN_G > 1.0) {
			TDFSB_FN_G = 1.0;
		} else if (TDFSB_FN_G < 0.0) {
			TDFSB_FN_G = 0.0;
		}
		if (TDFSB_FN_B > 1.0) {
			TDFSB_FN_B = 1.0;
		} else if (TDFSB_FN_B < 0.0) {
			TDFSB_FN_B = 0.0;
		}

		if (TDFSB_MAX_TEX_SIZE) {
			unsigned int max;
			for (max = 1; max <= TDFSB_MAX_TEX_SIZE; max *= 2) ;
			TDFSB_MAX_TEX_SIZE = max / 2;
		}

		if (realpath(TDFSB_CURRENTPATH, &temp_trunc[0]) != &temp_trunc[0])
			if (realpath(getenv("HOME"), &temp_trunc[0]) != &temp_trunc[0])
				strcpy(&temp_trunc[0], "/");
		strcpy(TDFSB_CURRENTPATH, temp_trunc);

		for (x = 0; x < TDFSB_KEYLIST_NUM - 1; x++) {
			for (y = x + 1; y < TDFSB_KEYLIST_NUM; y++)
				if (*TDFSB_KEYLIST[x] == *TDFSB_KEYLIST[y]) {
					TDFSB_ALERT_KC = 1000;
					printf("MALFORMED KEYBOARD MAP: %s conflicts with %s\n", TDFSB_KEYLIST_NAMES[x], TDFSB_KEYLIST_NAMES[y]);
				}
		}

		if (TDFSB_ALERT_KC)
			setup_kc();

		return (0);
	} else {
		printf("couldn't open config file\n");
		printf("creating config file\n");
		if ((config = fopen(homefile, "w"))) {
			fprintf(config, "# 3DFSB Example Config File\n\n");
			for (x = 0; x < paracnt; x++)
				if (param_type[x] != 5)
					fprintf(config, "%-18s = %-6s # %s\n", param[x], pdef[x], tdfsb_comment[x]);

			fprintf(config, "\n# Key bindings\n\n");
			for (x = 0; x < TDFSB_KEYLIST_NUM; x++)
				fprintf(config, "%-18s = \"%c\"\n", TDFSB_KEYLIST_NAMES[x], *TDFSB_KEYLIST[x]);

			fclose(config);
		} else
			printf("couldn't create config file\n");

		return (1);
	}
}

void save_config(void)
{
	FILE *config;
	char homefile[256];
	unsigned int x;

	strcpy(homefile, getenv("HOME"));
	if (homefile[strlen(homefile) - 1] == '/')
		strcat(homefile, ".3dfsb");
	else
		strcat(homefile, "/.3dfsb");

	if ((config = fopen(homefile, "w"))) {
		fprintf(config, "# TDFSB Saved Config File\n\n");
		for (x = 0; x < paracnt; x++) {
			if (param_type[x] == 1)
				fprintf(config, "%-18s = %-6d \t# %s\n", param[x], *(int *)param_value[x], tdfsb_comment[x]);
			else if (param_type[x] == 2)
				fprintf(config, "%-18s = %s \t# %s\n", param[x], (char *)param_value[x], tdfsb_comment[x]);
			else if (param_type[x] == 3)
				fprintf(config, "%-18s = %f \t# %s\n", param[x], *(GLfloat *) param_value[x], tdfsb_comment[x]);
			else if (param_type[x] == 4)
				fprintf(config, "%-18s = %s \t# %s\n", param[x], *(int *)param_value[x] ? "yes" : "no", tdfsb_comment[x]);
		}

		fprintf(config, "\n# Key bindings\n\n");
		for (x = 0; x < TDFSB_KEYLIST_NUM; x++)
			fprintf(config, "%-18s = \"%c\"\n", TDFSB_KEYLIST_NAMES[x], *TDFSB_KEYLIST[x]);

		fclose(config);
	} else
		printf("couldn't create config file\n");

}

void setup_kc(void)
{
	TDFSB_KC_FLY = ' ';
	TDFSB_KC_HELP = 'h';
	TDFSB_KC_HOME = '0';
	TDFSB_KC_FS = 'f';
	TDFSB_KC_DOT = '.';
	TDFSB_KC_RELEASE_MOUSE = 'r';
	TDFSB_KC_RL = 'l';
	TDFSB_KC_CDU = 'u';
	TDFSB_KC_IMBR = 'b';
	TDFSB_KC_INFO = 'i';
	TDFSB_KC_DISP = 'y';
	TDFSB_KC_CRH = 'c';
	TDFSB_KC_FPS = 'p';
	TDFSB_KC_GCR = 'g';
	TDFSB_KC_SHD = 'm';
	TDFSB_KC_NAME = 't';
	TDFSB_KC_SORT = 'n';
	TDFSB_KC_CLASS = 'o';
	TDFSB_KC_UP = '1';
	TDFSB_KC_DOWN = '3';
	TDFSB_KC_LEFT = 'a';	// on azerty keyboards, this should be q... but we set the defaults to querty
	TDFSB_KC_RIGHT = 'd';
	TDFSB_KC_SAVE = 'x';
	TDFSB_KC_FTH = '#';
	TDFSB_KC_FORWARD = 'w';	// on azerty keyboards, this should be z... but we set the defaults to querty
	TDFSB_KC_BACKWARD = 's';
}
