
sudo apt-get install gstreamer0.10-plugins-*

How does video get displayed?

- When you select it by clicking, nothing happens except that it gets colored
- When you play it by pressing enter, speckey(key=13) enter handling is called which does the setup (and calls SMPEG_play())
- Then, display() is called continuously, and does play_mpeg() because TDFSB_MPEG_FILE is set, which sets the correct frame

(gdb) c
Continuing.

Breakpoint 3, speckey (key=13) at tdfsb.c:3825
3825				if (TDFSB_OBJECT_SELECTED) {
(gdb) bt
#0  speckey (key=13) at tdfsb.c:3825
#1  0x0000000000403b38 in main (argc=<optimized out>, argv=0x7fffffffe5a8) at tdfsb.c:4525
(gdb) c
Continuing.
MPEG Start /home/tvb/big.mpg
[New Thread 0x7fffec68e700 (LWP 6226)]
[New Thread 0x7fffebe8d700 (LWP 6227)]
[New Thread 0x7fffeb68c700 (LWP 6228)]

Breakpoint 4, play_mpeg () at tdfsb.c:383
383	{
(gdb) bt
#0  play_mpeg () at tdfsb.c:383
#1  0x0000000000407e28 in display () at tdfsb.c:2707
#2  0x0000000000403a46 in main (argc=<optimized out>, argv=0x7fffffffe5a8) at tdfsb.c:4489
(gdb) l
378		SDL_Quit();
379		exit(code);
380	}
381	
382	void play_mpeg()
383	{
384		SMPEG_getinfo(TDFSB_MPEG_HANDLE, &TDFSB_MPEG_INFO);
385		if (TDFSB_MPEG_FRAMENO != TDFSB_MPEG_INFO.current_frame) {
386			TDFSB_MPEG_FRAMENO = TDFSB_MPEG_INFO.current_frame;
387			SDL_LockSurface(TDFSB_MPEG_SURFACE);
(gdb) s
384		SMPEG_getinfo(TDFSB_MPEG_HANDLE, &TDFSB_MPEG_INFO);
(gdb) display TDFSB_MPEG_INFO
1: TDFSB_MPEG_INFO = {has_audio = 1, has_video = 1, width = 352, height = 240, current_frame = 0, current_fps = 0, 
  audio_string = "MPEG-1 Layer 2 224kbit/s 44100Hz stereo", '\000' <repeats 40 times>, audio_current_frame = 0, current_offset = 6149, 
  total_size = 103606244, current_time = 0, total_time = 597.20327777777777}
(gdb) display TDFSB_MPEG_FRAMENO

