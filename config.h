#ifndef CONFIG_H
#define CONFIG_H

#include <GL/gl.h>

// The global variables below are used by 3dfsb.c

unsigned char TDFSB_KC_FLY, TDFSB_KC_HELP, TDFSB_KC_HOME;
unsigned char TDFSB_KC_FS, TDFSB_KC_DOT, TDFSB_KC_RELM;
unsigned char TDFSB_KC_RL, TDFSB_KC_CDU, TDFSB_KC_IMBR;
unsigned char TDFSB_KC_INFO, TDFSB_KC_DISP, TDFSB_KC_CRH;
unsigned char TDFSB_KC_FPS, TDFSB_KC_GCR, TDFSB_KC_SHD;
unsigned char TDFSB_KC_NAME, TDFSB_KC_SORT, TDFSB_KC_CLASS;
unsigned char TDFSB_KC_UP, TDFSB_KC_DOWN, TDFSB_KC_LEFT;
unsigned char TDFSB_KC_RIGHT, TDFSB_KC_SAVE, TDFSB_KC_FTH;
unsigned char TDFSB_KC_FORWARD, TDFSB_KC_BACKWARD;

char TDFSB_CURRENTPATH[4096];
char TDFSB_CUSTOM_EXECUTE_STRING[4096];

int SWX, SWY;		// Window size X,Y
int PWX, PWY;		// Full screen window size X,Y
int PWD;		// Full screen color depth

GLint TDFSB_BALL_DETAIL;
GLint TDFSB_WAS_NOREAD;
unsigned int TDFSB_MAX_TEX_SIZE;	// This cause Out-Of-Memory at 8192 and beyond if I enter a directory with high resolution images or video. So 1024 is a nice default, this uses around 150MB of RAM when going into a directory with around 300 high resolution images.
GLfloat mousesense;
GLfloat mousespeed;	// 1-20, with 1 being maximum
GLfloat headspeed;	// 1.1-2.0, with 2.0 being maximum

GLfloat TDFSB_GG_R, TDFSB_GG_G, TDFSB_GG_B;
GLfloat TDFSB_BG_R, TDFSB_BG_G, TDFSB_BG_B;
GLfloat TDFSB_FN_R, TDFSB_FN_G, TDFSB_FN_B;

int TDFSB_ICUBE;
int TDFSB_SHOW_DOTFILES;
int TDFSB_DIR_ALPHASORT;
int TDFSB_FULLSCREEN;

int TDFSB_CONFIG_FULLSCREEN;
int TDFSB_SHOW_CROSSHAIR;
int TDFSB_GROUND_CROSS;
int TDFSB_CLASSIC_NAV;
int TDFSB_MODE_FLY;

int TDFSB_FPS_CONFIG;
int TDFSB_MW_STEPS;
int TDFSB_CSE_FLAG;
long TDFSB_US_RUN;
int TDFSB_FPS_CACHE;
int TDFSB_FPS_REAL;
int TDFSB_ALERT_KC;

int read_or_create_config_file(void);
void save_config(void);
void setup_kc(void);

#endif
