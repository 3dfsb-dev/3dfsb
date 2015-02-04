// C library includes
#include <math.h>

// Other library includes
#include <xdo.h>
#include <X11/extensions/XTest.h>

// Own includes
#include "3dfsb.h"
#include "config.h"
#include "input.h"
#include "media.h"
#include "str_replace.h"
#include "tools.h"

/*
 * Global variables that are only used in this file.
 * These are global because they are read in one function call, and written to in another, so they go out of function scope.
 * You could say these are part of the "model" that we build here,
 * although it doesn't make much sense to include window size and bpp in the model of the input system.
 */
int aSWX, aSWY, aPWX, aPWY;
SDL_TimerID TDFSB_TIMER_ID;

/*
 * Input handling code
 */
void MouseMove(int x, int y)
{
	if (!TDFSB_ANIM_STATE) {
		centX = centX - (GLdouble) (headspeed * (floor(((double)SWX) / 2) - (double)x));
		centY = centY - (GLdouble) (headspeed * (floor(((double)SWY) / 2) - (double)y));
		if (centY >= SWY / 2)
			centY = SWY / 2;
		if (centY <= ((-SWY) / 2))
			centY = ((-SWY) / 2);
		tposz = ((GLdouble) sin((((double)centX) / (double)(SWX / mousesense / PI)))) * ((GLdouble) cos((((double)centY) / (double)(SWY / PI))));
		tposx = ((GLdouble) cos((((double)centX) / (double)(SWX / mousesense / PI)))) * ((GLdouble) cos((((double)centY) / (double)(SWY / PI))));
		tposy = -((GLdouble) sin((((double)centY) / (double)(SWY / PI))));
	}
}

void MouseLift(int x, int y)
{
	if (!TDFSB_ANIM_STATE) {
		uposy += ((GLfloat) ((GLfloat) (SWY / 2) - (GLfloat) y) / 25);

		centX = centX - (GLdouble) (headspeed * (floor(((double)SWX) / 2) - (double)x));
/*        	centY=centY-(GLdouble)(headspeed*(floor(((double)SWY)/2)-(double)y));*/
		if (centY >= SWY / 2)
			centY = SWY / 2;
		if (centY <= ((-SWY) / 2))
			centY = ((-SWY) / 2);
		tposz = ((GLdouble) sin((((double)centX) / (double)(SWX / mousesense / PI)))) * ((GLdouble) cos((((double)centY) / (double)(SWY / PI))));
		tposx = ((GLdouble) cos((((double)centX) / (double)(SWX / mousesense / PI)))) * ((GLdouble) cos((((double)centY) / (double)(SWY / PI))));
		tposy = -((GLdouble) sin((((double)centY) / (double)(SWY / PI))));
	}
}

void mouse(int button, int state)
{
	if (TDFSB_ANIM_STATE)
		return;		// We don't react to mouse buttons when we are in animation state, such as flying somewhere

	switch (button) {

	case SDL_BUTTON_LEFT:
		if (!TDFSB_CLASSIC_NAV) {
			if (state == SDL_PRESSED) {
				activate_object(TDFSB_OBJECT_SELECTED);
			} else {
				TDFSB_OBJECT_SELECTED = NULL;
				TDFSB_KEY_FINDER = 0;
				TDFSB_FUNC_KEY = keyboard;
				TDFSB_FUNC_UPKEY = keyboardup;
			}
			break;
		} else {	// Classic mode means left mouse button is followed
			if (state == SDL_PRESSED) {
				forwardkeybuf = 1;
				backwardkeybuf = 0;
				TDFSB_FUNC_IDLE = move;
			} else {
				forwardkeybuf = 0;
				check_standstill();
			}
			break;
		}

	case SDL_BUTTON_RIGHT:
		if (!TDFSB_CLASSIC_NAV) {
			if (state == SDL_PRESSED && TDFSB_OBJECT_SELECTED) {
				stop_move();
				TDFSB_OA = TDFSB_OBJECT_SELECTED;
				TDFSB_OA_DX = (TDFSB_OA->posx - vposx) / 100;
				TDFSB_OA_DY = (TDFSB_OA->posy + TDFSB_OA->scaley + 4 - vposy) / 100;
				TDFSB_OA_DZ = (TDFSB_OA->posz - vposz) / 100;
				TDFSB_ANIM_STATE = 1;
				TDFSB_OBJECT_SELECTED = NULL;
				TDFSB_FUNC_KEY = keyboard;
				TDFSB_FUNC_UPKEY = keyboardup;
				TDFSB_KEY_FINDER = 0;
				TDFSB_FUNC_IDLE = approach;
			}
			break;
		} else {
			if (state == SDL_PRESSED) {
				backwardkeybuf = 1;
				forwardkeybuf = 0;
				TDFSB_FUNC_IDLE = move;
			} else {
				backwardkeybuf = 0;
				check_standstill();
			}
			break;
		}

	case SDL_BUTTON_MIDDLE:
		if (state == SDL_PRESSED) {
			TDFSB_FUNC_MOTION = MouseLift;
			TDFSB_FUNC_IDLE = move;
		} else {
			TDFSB_FUNC_MOTION = MouseMove;
			TDFSB_FUNC_IDLE = move;
			check_standstill();
			uposy = vposy;
		}
		break;

	case 4:		// SDL_SCROLLUP?
		if (state == SDL_PRESSED)
			uposy = vposy = vposy + TDFSB_MW_STEPS;
		if (vposy < 0)
			vposy = uposy = 0;
		break;

	case 5:		// SDL_SCROLLDOWN?
		if (state == SDL_PRESSED)
			uposy = vposy = vposy - TDFSB_MW_STEPS;
		if (vposy < 0)
			vposy = uposy = 0;
		break;

	case 6:
		printf("No function for button 6 yet\n");
		break;

	default:
		break;
	}
}

void release_mouse(void)
{
	TDFSB_FUNC_MOUSE = NULL;
	TDFSB_FUNC_MOTION = NULL;
	SDL_ShowCursor(SDL_ENABLE);
	TDFSB_HAVE_MOUSE = 0;
}

void grab_mouse(void)
{
	SDL_WarpMouse(SWX / 2, SWY / 2);
	smoox = tposx;
	smooy = tposy;
	smooz = tposz;
	TDFSB_FUNC_MOUSE = mouse;
	TDFSB_FUNC_MOTION = MouseMove;
	SDL_ShowCursor(SDL_DISABLE);
	TDFSB_HAVE_MOUSE = 1;
}

void toggle_mouse_grab(void)
{
	if (TDFSB_HAVE_MOUSE) {
		release_mouse();
	} else {
		grab_mouse();
	}
}

/* Returns 1 when the key still has to be handled by the keyboard function */
int speckey(int key)
{
	if (!TDFSB_ANIM_STATE) {

		// Update the fullpath variable
		char fullpath[4096];
		strcpy(fullpath, TDFSB_CURRENTPATH);
		if (strlen(fullpath) > 1)
			strcat(fullpath, "/");
		if (TDFSB_OBJECT_SELECTED)
			strcat(fullpath, TDFSB_OBJECT_SELECTED->name);

		switch (key) {

		case SDLK_TAB:
			if (TDFSB_MEDIA_FILE && TDFSB_MEDIA_FILE->regtype == PROCESS) {
				printf("tab pressed while focussed on a PROCESS, sending TAB to the process...\n");
				system("DISPLAY=:0 xdotool key --window 37748743 Tab");
			} else if (TDFSB_MEDIA_FILE && TDFSB_MEDIA_FILE->regtype == TEXTFILE) {
				system("DISPLAY=:1 xdotool key a");
			} else {
				char TDFSB_CES_TEMP[4096];
				if (TDFSB_OBJECT_SELECTED) {
					TDFSB_OBJECT_SELECTED = NULL;
					TDFSB_KEY_FINDER = 0;
					TDFSB_FUNC_KEY = keyboard;
					TDFSB_FUNC_UPKEY = keyboardup;
				}
				if (SDL_GetModState() & 0x0003)
					snprintf(TDFSB_CES_TEMP, 4096, "cd \"%s\"; xterm&", TDFSB_CURRENTPATH);
				else {
					if (TDFSB_CSE_FLAG)
						str_replace(TDFSB_CUSTOM_EXECUTE_STRING, strlen(TDFSB_CUSTOM_EXECUTE_STRING), "%s", fullpath);
					else
						strncpy(TDFSB_CES_TEMP, TDFSB_CUSTOM_EXECUTE_STRING, 4096);
				}
				system(TDFSB_CES_TEMP);
				printf("EXECUTE COMMAND: %s\n", TDFSB_CES_TEMP);
				if (TDFSB_HAVE_MOUSE)
					release_mouse();
			}
			break;
		case SDLK_F1:
			if (mousespeed > 1)
				mousespeed = mousespeed - 1;
			TDFSB_SPEED_DISPLAY = 100;
			break;
		case SDLK_F2:
			if (mousespeed < 20)
				mousespeed = mousespeed + 1;
			TDFSB_SPEED_DISPLAY = 100;
			break;

		case SDLK_F4:
			headspeed = headspeed - .1;
			if (headspeed < 0.1)
				headspeed = 0.1;
			TDFSB_SPEED_DISPLAY = 100;
			break;
		case SDLK_F3:
			headspeed = headspeed + .1;
			if (headspeed > 2.0)
				headspeed = 2.0;
			TDFSB_SPEED_DISPLAY = 100;
			break;

		case SDLK_F5:
			TDFSB_BALL_DETAIL++;
			sprintf(ballbuf, "Ball Detail: %d\n", (int)TDFSB_BALL_DETAIL);
			TDFSB_SHOW_BALL = 100;
			break;
		case SDLK_F6:
			if (TDFSB_BALL_DETAIL > 4) {
				TDFSB_BALL_DETAIL--;
			}
			sprintf(ballbuf, "Ball Detail: %d\n", (int)TDFSB_BALL_DETAIL);
			TDFSB_SHOW_BALL = 100;
			break;

		case SDLK_F7:
			TDFSB_FPS_CONFIG++;
			TDFSB_US_RUN = 1000000 / TDFSB_FPS_CONFIG;
			sprintf(cfpsbuf, "MaxFPS: %d", TDFSB_FPS_CONFIG);
			if (TDFSB_FPS_CONFIG)
				strcpy(throttlebuf, "Throttle: ON");
			else
				strcpy(throttlebuf, "Throttle:OFF");
			TDFSB_SHOW_CONFIG_FPS = 100;
			break;
		case SDLK_F8:
			if (TDFSB_FPS_CONFIG > 0)
				TDFSB_FPS_CONFIG--;
			if (TDFSB_FPS_CONFIG > 0) {
				TDFSB_US_RUN = 1000000 / TDFSB_FPS_CONFIG;
			}
			sprintf(cfpsbuf, "MaxFPS: %d", TDFSB_FPS_CONFIG);
			if (TDFSB_FPS_CONFIG)
				strcpy(throttlebuf, "Throttle: ON");
			else
				strcpy(throttlebuf, "Throttle:OFF");
			TDFSB_SHOW_CONFIG_FPS = 100;
			break;
		case SDLK_F9:
			CURRENT_TOOL++;
			if (CURRENT_TOOL >= NUMBER_OF_TOOLS)
				CURRENT_TOOL = 0;
			break;
		case SDLK_F12:
			if (TDFSB_MEDIA_FILE) {
				INPUT_OBJECT = TDFSB_MEDIA_FILE;
				// Ensure the normal keyboard handlers are enabled,
				// because the mousebuttonup event will be consumed by the INPUT_OBJECT,
				// so we'll stay in some kind of (currently disabled?) "finder" mode otherwise...
				TDFSB_KEY_FINDER = 0;
				TDFSB_FUNC_KEY = keyboard;
				TDFSB_FUNC_UPKEY = keyboardup;
			} else {
				printf("Cannot bind to any object because none is active!\n");
			}
			break;
		case SDLK_HOME:
			vposy = 0;
			lastposx = lastposz = vposx = vposz = -10;
			smooy = tposy = 0;
			smoox = tposx = SQF;
			smooz = tposz = SQF;
			smoou = 0;
			uposy = 0;
			viewm();
			break;

		case SDLK_UP:
			forwardkeybuf = 1;
			backwardkeybuf = 0;
			TDFSB_FUNC_IDLE = move;
			break;
		case SDLK_DOWN:
			backwardkeybuf = 1;
			forwardkeybuf = 0;
			TDFSB_FUNC_IDLE = move;
			break;
		case SDLK_LEFT:
			leftkeybuf = 1;
			rightkeybuf = 0;
			TDFSB_FUNC_IDLE = move;
			break;
		case SDLK_RIGHT:
			rightkeybuf = 1;
			leftkeybuf = 0;
			TDFSB_FUNC_IDLE = move;
			break;

		case SDLK_PAGEUP:
			upkeybuf = 1;
			downkeybuf = 0;
			TDFSB_FUNC_IDLE = move;
			break;
		case SDLK_PAGEDOWN:
			downkeybuf = 1;
			upkeybuf = 0;
			TDFSB_FUNC_IDLE = move;
			break;
		case SDLK_END:
			TDFSB_OA_DY = vposy / 25;
			TDFSB_ANIM_COUNT = 25;
			TDFSB_ANIM_STATE = 1;
			TDFSB_FUNC_IDLE = ground;
			break;
		case SDLK_RCTRL:
			if (TDFSB_OBJECT_SELECTED) {
				stop_move();
				TDFSB_OA = TDFSB_OBJECT_SELECTED;
				TDFSB_OA_DX = (TDFSB_OA->posx - vposx) / 100;
				if (!((TDFSB_OA->mode) & 0x20)) {	// if not a symlink
					TDFSB_OA_DY = (TDFSB_OA->posy + TDFSB_OA->scaley + 4 - vposy) / 100;
				} else {
					TDFSB_OA_DY = (5 - vposy) / 100;
				}
				TDFSB_OA_DZ = (TDFSB_OA->posz - vposz) / 100;

				// Fly straight into directories
				if (TDFSB_OA->regtype == DIRECTORY) {
					TDFSB_OA_DY = ((TDFSB_OA->posy - vposy) / 100) * 2;	// Straight into the center, not above
					// Do it at double speed
					TDFSB_OA_DX = TDFSB_OA_DX * 2;
					TDFSB_OA_DZ = TDFSB_OA_DZ * 2;
				}
				TDFSB_ANIM_STATE = 1;

				TDFSB_OBJECT_SELECTED = NULL;
				TDFSB_FUNC_KEY = keyboard;
				TDFSB_FUNC_UPKEY = keyboardup;
				TDFSB_KEY_FINDER = 0;
				TDFSB_FUNC_IDLE = approach;
			}
			break;
		case SDLK_RETURN:
			activate_object(TDFSB_OBJECT_SELECTED);
			break;

		default:
			return (1);
			break;
		}
	}

	return (0);
}

/*
 * When a keyup event is received, check whether a special key [1] is received, to stop some motion in a direction
 * [1]: special keys are: up, down, left, right, pageup, pagedown
 */
int specupkey(int key)
{
	if (!TDFSB_ANIM_STATE) {
		if (key == SDLK_UP) {
			forwardkeybuf = 0;
			check_standstill();
		} else if (key == SDLK_DOWN) {
			backwardkeybuf = 0;
			check_standstill();
		} else if (key == SDLK_LEFT) {
			leftkeybuf = 0;
			check_standstill();
		} else if (key == SDLK_RIGHT) {
			rightkeybuf = 0;
			check_standstill();
		} else if (key == SDLK_PAGEUP) {
			upkeybuf = 0;
			check_standstill();
		} else if (key == SDLK_PAGEDOWN) {
			downkeybuf = 0;
			check_standstill();
		} else
			return (1);

		return (0);
	} else
		return (0);
}

/*
 * The key finder overlaps with the various setting keys,
 * so either we activate the keyfinder when slash (/) is pressed,
 * or we activate it by default and move the setting keys somewhere...
 * Plan: activate the keyfinder when the user presses slash (/)
 */
/*
int keyfinder(unsigned char key)
{
	TDFSB_KEY_FINDER = key;
	return (0);
}

int keyupfinder(unsigned char key)
{
	UNUSED(key);		// Can't change the function signature because keyfinder() *does* need the key
	TDFSB_KEY_FINDER = 0;
	return (0);
}
*/

int keyboardup(unsigned char key)
{
	if (!TDFSB_ANIM_STATE) {
		if (key == TDFSB_KC_FORWARD) {
			forwardkeybuf = 0;
			check_standstill();
		} else if (key == TDFSB_KC_BACKWARD) {
			backwardkeybuf = 0;
			check_standstill();
		} else if (key == TDFSB_KC_LEFT) {
			leftkeybuf = 0;
			check_standstill();
		} else if (key == TDFSB_KC_RIGHT) {
			rightkeybuf = 0;
			check_standstill();
		} else if (key == TDFSB_KC_UP) {
			upkeybuf = 0;
			check_standstill();
		} else if (key == TDFSB_KC_DOWN) {
			downkeybuf = 0;
			check_standstill();
		} else
			return (1);

		return (0);
	} else
		return (0);
}

/* Handle a keydown event */
int keyboard(unsigned char key)
{
	if (!TDFSB_ANIM_STATE) {
		if (key == 27) {
			printf("\nBye bye...\n\n");
			ende(0);
		} else if (key == TDFSB_KC_FLY) {
			TDFSB_MODE_FLY = 1 - TDFSB_MODE_FLY;
			if (TDFSB_MODE_FLY)
				strcpy(flybuf, "Flying: ON");
			else
				strcpy(flybuf, "Flying:OFF");
			TDFSB_FLY_DISPLAY = 100;
		} else if (key == TDFSB_KC_HELP) {
			TDFSB_SHOW_HELP = 1 - TDFSB_SHOW_HELP;
			if (TDFSB_SHOW_HELP) {
				printf("\n=======================================\n");
				puts(help_str);
				printf("=======================================\n\n");
			}
		} else if (key == TDFSB_KC_FS) {
			if (TDFSB_FULLSCREEN) {
				SDL_Surface *window;
				if ((window = SDL_SetVideoMode(aSWX, aSWY, PWD, SDL_OPENGL | SDL_RESIZABLE)) == 0) {
					printf("SDL ERROR Video mode set failed: %s\n", SDL_GetError());
					ende(1);
				}
				reshape(window->w, window->h);
				TDFSB_CONFIG_FULLSCREEN = 0;

			} else {
				aSWX = SWX;
				aSWY = SWY;
				SDL_Surface *window;
				if ((window = SDL_SetVideoMode(PWX, PWY, PWD, SDL_OPENGL | SDL_FULLSCREEN)) == 0) {
					printf("SDL ERROR Video mode set failed: %s\n", SDL_GetError());
					ende(1);
				}
				reshape(window->w, window->h);
				TDFSB_CONFIG_FULLSCREEN = 1;
			}
			TDFSB_FULLSCREEN = 1 - TDFSB_FULLSCREEN;
		}

		else if (key == TDFSB_KC_DOT) {
			TDFSB_SHOW_DOTFILES = 1 - TDFSB_SHOW_DOTFILES;
			TDFSB_FUNC_IDLE = nullDisplay;
			TDFSB_FUNC_DISP = noDisplay;
		} else if (key == TDFSB_KC_RELEASE_MOUSE) {
			toggle_mouse_grab();
		}

		else if (key == TDFSB_KC_RL) {
			TDFSB_FUNC_IDLE = nullDisplay;
			TDFSB_FUNC_DISP = noDisplay;
		}

		else if (key == TDFSB_KC_CDU) {
			strcat(TDFSB_CURRENTPATH, "/..");
			char temp_trunc[4096];
			temp_trunc[0] = 0;
			if (realpath(TDFSB_CURRENTPATH, &temp_trunc[0]) != &temp_trunc[0]) {
				printf("Cannot resolve path \"%s\".\n", TDFSB_CURRENTPATH);
				ende(1);
			} else {
				strcpy(TDFSB_CURRENTPATH, temp_trunc);
				TDFSB_FUNC_IDLE = nullDisplay;
				TDFSB_FUNC_DISP = noDisplay;
				return 0;
			}
			TDFSB_FUNC_IDLE = nullDisplay;
			TDFSB_FUNC_DISP = noDisplay;
		}

		else if (key == TDFSB_KC_IMBR) {
			if (TDFSB_ICUBE == 1) {
				TDFSB_ICUBE = 0;
			} else {
				TDFSB_ICUBE = 1;
			}
			TDFSB_FUNC_IDLE = nullDisplay;
			TDFSB_FUNC_DISP = noDisplay;
		}

		else if (key == TDFSB_KC_INFO) {
			printf("\n");
			printf("GL_RENDERER   = %s\n", (const char *)glGetString(GL_RENDERER));
			printf("GL_VERSION    = %s\n", (const char *)glGetString(GL_VERSION));
			printf("GL_VENDOR     = %s\n", (const char *)glGetString(GL_VENDOR));
			printf("GL_EXTENSIONS = %s\n", (const char *)glGetString(GL_EXTENSIONS));
			printf("\n");
			printf("Max Texture size %d x %d \n", (int)TDFSB_MAX_TEX_SIZE, (int)TDFSB_MAX_TEX_SIZE);
			printf("\n");
		}

		else if (key == TDFSB_KC_DISP) {
			if (TDFSB_SHOW_DISPLAY == 1) {
				TDFSB_SHOW_DISPLAY = 2;
				TDFSB_XL_DISPLAY = 0;
				TDFSB_SPEED_DISPLAY = 200;
			} else if (TDFSB_SHOW_DISPLAY == 2) {
				TDFSB_SHOW_DISPLAY = 3;
				TDFSB_XL_DISPLAY = TDFSB_XL_DISPLAY_INIT;
				TDFSB_SPEED_DISPLAY = 200;
			} else if (TDFSB_SHOW_DISPLAY == 3) {
				TDFSB_SHOW_DISPLAY = 0;
				TDFSB_XL_DISPLAY = 0;
			} else {
				TDFSB_SHOW_DISPLAY = 1;
				TDFSB_XL_DISPLAY = 0;
				TDFSB_SPEED_DISPLAY = 200;
			}
		}

		else if (key == TDFSB_KC_CRH) {
			TDFSB_SHOW_CROSSHAIR = 1 - TDFSB_SHOW_CROSSHAIR;
		}

		else if (key == TDFSB_KC_FPS) {
			TDFSB_SHOW_FPS = 1 - TDFSB_SHOW_FPS;
			if (TDFSB_SHOW_FPS) {
				sprintf(fpsbuf, "FPS: <wait>");
				TDFSB_FPS_DISP = 0;
				TDFSB_TIMER_ID = SDL_AddTimer(TDFSB_FPS_DT, (SDL_NewTimerCallback) fps_timer, 0);
			} else {
				SDL_RemoveTimer(TDFSB_TIMER_ID);
			}
		}

		else if (key == TDFSB_KC_GCR) {
			TDFSB_GROUND_CROSS = 1 - TDFSB_GROUND_CROSS;
		}

		else if (key == TDFSB_KC_SHD) {
			if (TDFSB_SHADE == 1) {
				glShadeModel(GL_FLAT);
				TDFSB_SHADE = 0;
			} else {
				glShadeModel(GL_SMOOTH);
				TDFSB_SHADE = 1;
			}
		}

		else if (key == TDFSB_KC_NAME) {
			TDFSB_FILENAMES++;
			if (TDFSB_FILENAMES == 3)
				TDFSB_FILENAMES = 0;
		}

		else if (key == TDFSB_KC_SORT) {
			TDFSB_DIR_ALPHASORT = 1 - TDFSB_DIR_ALPHASORT;
			TDFSB_FUNC_IDLE = nullDisplay;
			TDFSB_FUNC_DISP = noDisplay;
		}

		else if (key == TDFSB_KC_HOME) {
			strcpy(TDFSB_CURRENTPATH, getenv("HOME"));
			TDFSB_FUNC_IDLE = nullDisplay;
			TDFSB_FUNC_DISP = noDisplay;
		}

		else if (key == TDFSB_KC_CLASS) {
			TDFSB_CLASSIC_NAV = 1 - TDFSB_CLASSIC_NAV;
			if (TDFSB_CLASSIC_NAV)
				strcpy(classicbuf, "Classic: ON");
			else
				strcpy(classicbuf, "Classic:OFF");
			TDFSB_CLASSIC_DISPLAY = 100;
		} else if (key == TDFSB_KC_SAVE) {
			save_config();
		}

		else if (key == TDFSB_KC_FTH) {
			if (TDFSB_FPS_CONFIG) {
				strcpy(throttlebuf, "Throttle:OFF");
				TDFSB_FPS_CACHE = TDFSB_FPS_CONFIG;
				TDFSB_FPS_CONFIG = 0;
			} else {
				strcpy(throttlebuf, "Throttle: ON");
				TDFSB_FPS_CONFIG = TDFSB_FPS_CACHE;
			}

			if (TDFSB_FPS_CONFIG)
				strcpy(throttlebuf, "Throttle: ON");
			else
				strcpy(throttlebuf, "Throttle:OFF");

			sprintf(cfpsbuf, "MaxFPS: %d", TDFSB_FPS_CONFIG);
			TDFSB_SHOW_CONFIG_FPS = 100;
			TDFSB_SHOW_THROTTLE = 100;
		}

		else if (key == TDFSB_KC_FORWARD) {
			forwardkeybuf = 1;
			backwardkeybuf = 0;
			TDFSB_FUNC_IDLE = move;
		} else if (key == TDFSB_KC_BACKWARD) {
			backwardkeybuf = 1;
			forwardkeybuf = 0;
			TDFSB_FUNC_IDLE = move;
		} else if (key == TDFSB_KC_UP) {
			upkeybuf = 1;
			downkeybuf = 0;
			TDFSB_FUNC_IDLE = move;
		} else if (key == TDFSB_KC_DOWN) {
			downkeybuf = 1;
			upkeybuf = 0;
			TDFSB_FUNC_IDLE = move;
		} else if (key == TDFSB_KC_LEFT) {
			leftkeybuf = 1;
			rightkeybuf = 0;
			TDFSB_FUNC_IDLE = move;
		} else if (key == TDFSB_KC_RIGHT) {
			rightkeybuf = 1;
			leftkeybuf = 0;
			TDFSB_FUNC_IDLE = move;
		}

		else
			return (1);

		return (0);

	} else
		return (0);
}

/* Send an SDL_Event to the other X server */
void send_event_to_object(SDL_Event event)
{
	xdo_t *xdo = xdo_new(":1");
	if (event.type == SDL_MOUSEMOTION) {
		int centerx = SWX / 2;
		int centery = SWY / 2;
		int diffx = event.motion.x - centerx;
		int diffy = event.motion.y - centery;
		xdo_move_mouse_relative(xdo, diffx, diffy);
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		xdo_mouse_down(xdo, CURRENTWINDOW, event.button.button);
	} else if (event.type == SDL_MOUSEBUTTONUP) {
		xdo_mouse_up(xdo, CURRENTWINDOW, event.button.button);
	} else {
		printf("Sending event of type %d with event.key.keysym.sym %d\n", event.type, event.key.keysym.sym);
		char *keysequence = NULL;
		unsigned int ukeycode = 0;
		switch (event.key.keysym.sym) {
			// Here we try to follow the same order as in SDL_keysym.h
		case SDLK_NUMLOCK:
			keysequence = "Num_Lock";
			break;
		case SDLK_CAPSLOCK:
			keysequence = "Caps_Lock";
			break;
			//case SDLK_SCROLLLOCK: keysequence = "Scroll_Lock"; break; // SDL 2.0 has this, 1.2 doesn't ?
		case SDLK_RSHIFT:
			keysequence = "Shift_R";
			break;
		case SDLK_LSHIFT:
			keysequence = "Shift_L";
			break;
		case SDLK_RCTRL:
			keysequence = "Control_R";
			break;
		case SDLK_LCTRL:
			keysequence = "Control_L";
			break;
		case SDLK_RALT:
			keysequence = "Alt_R";
			break;
		case SDLK_LALT:
			keysequence = "Alt_L";
			break;
		case SDLK_RMETA:
			keysequence = "Meta_R";
			break;
		case SDLK_LMETA:
			keysequence = "Meta_L";
			break;
		case SDLK_RSUPER:
			keysequence = "Super_R";
			break;
		case SDLK_LSUPER:
			keysequence = "Super_L";
			break;
		case SDLK_MODE:
			keysequence = "ISO_Level3_Shift";
			break;	// AltGr
		case SDLK_RETURN:
			keysequence = "Return";
			break;
		case SDLK_ESCAPE:
			keysequence = "Escape";
			break;
		case SDLK_UP:
			keysequence = "Up";
			break;
		case SDLK_DOWN:
			keysequence = "Down";
			break;
		case SDLK_LEFT:
			keysequence = "Left";
			break;
		case SDLK_RIGHT:
			keysequence = "Right";
			break;
		case SDLK_PAGEUP:
			keysequence = "Page_Up";
			break;
		case SDLK_PAGEDOWN:
			keysequence = "Page_Down";
			break;
		case SDLK_BACKSPACE:
			keysequence = "BackSpace";
			break;
		case SDLK_DELETE:
			keysequence = "Delete";
			break;
		case SDLK_TAB:
			keysequence = "Tab";
			break;
		case SDLK_HOME:
			keysequence = "Home";
			break;
		case SDLK_END:
			keysequence = "End";
			break;
		default:
			ukeycode = XKeysymToKeycode(xdo->xdpy, event.key.keysym.sym);
			if (ukeycode) {
				if (event.type == SDL_KEYDOWN) {
					XTestFakeKeyEvent(xdo->xdpy, ukeycode, 1, 0);
				} else if (event.type == SDL_KEYUP) {
					XTestFakeKeyEvent(xdo->xdpy, ukeycode, 0, 0);
				}
				XSync(xdo->xdpy, False);
				XFlush(xdo->xdpy);
			} else {
				printf("Not forwarding key %d\n", event.key.keysym.sym);
			}
			keysequence = NULL;	// Ensure the code below does not send any more keys
			break;
		}
		// Only unbind at keydown, because a keyup could come from the previous F12 press
		if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F12) {
			printf("Unbinding mouse and keyboard input from %s\n", INPUT_OBJECT->name);
			INPUT_OBJECT = NULL;
		} else if (keysequence) {	// Gets set by the code above
			// This works fine in a normal Xorg session but with Xvnc it generates strange stuff;
			// xev shows just 2 events for a shiftdown, a press, shiftup
			if (event.type == SDL_KEYDOWN) {
				printf("keydown %s\n", keysequence);
				xdo_send_keysequence_window_down(xdo, CURRENTWINDOW, keysequence, 0);
				//char * todo
			} else if (event.type == SDL_KEYUP) {
				printf("keyup %s\n", keysequence);
				xdo_send_keysequence_window_up(xdo, CURRENTWINDOW, keysequence, 0);
			}
		}
	}
	xdo_free(xdo);
}

/* Main input processing funtion, initiated by SDL when a new event is received */
void input_process_event(SDL_Event event)
{
	SDL_Surface *window;
	switch (event.type) {
	case SDL_QUIT:
		ende(0);
		break;
	case SDL_VIDEORESIZE:
		if ((window = SDL_SetVideoMode(event.resize.w, event.resize.h, PWD, SDL_OPENGL | SDL_RESIZABLE)) == 0) {
			printf("SDL ERROR Video mode set failed: %s\n", SDL_GetError());
			ende(1);
		}
		reshape(event.resize.w, event.resize.h);
		break;
	case SDL_MOUSEMOTION:
		if (INPUT_OBJECT) {
			send_event_to_object(event);
		} else if (TDFSB_FUNC_MOTION) {
			TDFSB_FUNC_MOTION(event.motion.x, event.motion.y);
		}
		break;
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		if (INPUT_OBJECT) {
			send_event_to_object(event);
		} else if (TDFSB_FUNC_MOUSE) {
			TDFSB_FUNC_MOUSE(event.button.button, event.button.state);
		}
		break;
	case SDL_KEYDOWN:
		if (INPUT_OBJECT) {
			send_event_to_object(event);
		} else if (speckey(event.key.keysym.sym)) {
			TDFSB_FUNC_KEY((unsigned char)(event.key.keysym.unicode & 0x7F));
		}
		break;
	case SDL_KEYUP:
		if (INPUT_OBJECT) {
			send_event_to_object(event);
		} else if (specupkey(event.key.keysym.sym)) {
			TDFSB_FUNC_UPKEY((unsigned char)(event.key.keysym.sym & 0x7F));	// no unicode for key up's
		}
		break;
	default:
		break;
	}
}
