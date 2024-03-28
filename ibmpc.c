#include <dos.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void outbyte(unsigned port, unsigned char byte);
#pragma aux outbyte = \
  "out dx, al" \
  parm [dx] [ax];

unsigned char inbyte(unsigned port);
#pragma aux inbyte = \
  "in al, dx" \
  parm [dx] value [al];  

#include "extdlc.c"
#include "constrct.c"
#include "structs.c"

extern char errortitle[];
extern unsigned char __far *VGA;
extern unsigned _RES_X, _RES_Y, old_xres, old_yres;
extern struct prog_window *running_window, *draw_window_box;
extern unsigned char _FONT_HEIGHT, screen_redraw, VPUCON_LINES, video;
extern unsigned char _BORDERCOLOR_SHADOW, _BORDERCOLOR_LIGHT, _DESKTOP_BGCOLOR;
extern struct prog_window **windows;
extern int active_window, videodriver;

extern volatile unsigned char tty;
extern struct vt *ttys;

extern void __far *old_put_pixel, *old_get_pixel, *old_draw_rectangle, *old_fill_screen;

int dialog(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned type, unsigned help_button, unsigned sound, unsigned char *message, unsigned char *title, unsigned char *helptext);  
unsigned char windowpos_visible(struct prog_window *window, int x, int y);
void draw_borders(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char type);

//#include "int23h.c"
#include "int24h.c"
#include "graphics.c"
#include "kbdint.c"
//#include "int10h.c"
