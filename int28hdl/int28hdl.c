#include <dos.h>

#include "structs.c"
#include "constrct.c"

extern void __far * __based(__segname("_TEXT")) old_dosidle_handler;
void __interrupt dosidle_handler();
extern void __far * __based(__segname("_TEXT")) old_int21hdl;
void __interrupt int21wrp();

void int28h_hdl(void);
#pragma aux int28h_hdl modify exact [];

extern char *mouse_cursor;
extern char *normal_cursor;
extern char *busy_cursor;
extern struct prog_window *running_console;

unsigned mtask_win(int *, unsigned, struct prog_window *);

extern signed next_window;
extern signed next_tty;

extern struct prog_window *running_window;

extern int window_count;

void hide_mouse_cursor(void);
void redraw_mouse_cursor(void);

void save_stack_seg(void);

void int28h_hdl(void)
{
  int cur_tty = next_tty;
  int cur_win = next_window;
  struct prog_window *cur_con = running_console;
  static signed int28_tty = -1;
  static signed int28_win = -1;

  if(mouse_cursor == normal_cursor)
  {
    hide_mouse_cursor();
    mouse_cursor = busy_cursor;
  }
  redraw_mouse_cursor();
//  next_tty = next_window = -1;
  if(int28_tty < -1) int28_tty = TTY_COUNT;
  if(int28_win < -1) int28_win = window_count;
  next_tty = int28_tty;
  next_window = int28_win;
  running_console = 0;
  mtask_win(0, 0, running_window);
  int28_win--;
  int28_tty--;

  next_window = cur_win;
  next_tty = cur_tty;
  running_console = cur_con;
}

void set_dosidle_handler(void)
{
  _dos_setvect(0x28, dosidle_handler);
  _dos_setvect(0x21, int21wrp);
}

void save_dosidle_handler(void)
{
  old_dosidle_handler = _dos_getvect(0x28);
  old_int21hdl = _dos_getvect(0x21);
  save_stack_seg();
  set_dosidle_handler();
}

void reset_dosidle_handler(void)
{
  _dos_setvect(0x21, old_int21hdl);
  _dos_setvect(0x28, old_dosidle_handler);
}
