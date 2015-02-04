#ifndef INPUT_H
#define INPUT_H

void MouseMove(int x, int y);
void MouseLift(int x, int y);
void mouse(int button, int state);
void release_mouse(void);
void grab_mouse(void);
void toggle_mouse_grab(void);

int speckey(int key);
int specupkey(int key);
int keyboardup(unsigned char key);
int keyboard(unsigned char key);

void send_event_to_object(SDL_Event event);

void input_process_event(SDL_Event event);
#endif
