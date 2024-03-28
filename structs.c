struct text_print
{
  unsigned int x;
  unsigned char *text;
};

struct screen_region
{
  int x0;
  int y0;
  int x1;
  int y1;
};

struct hitbox
{
  int x0;
  int y0;
  int x1;
  int y1;
  unsigned int element;
};

struct window_element
{
  /* -- element types --
   0: static text or image
   1: button
   2: textbox
   3: list
   4: scrollbar
   5: icon
   6: checkbox
   7: radio button
   8: multi-line textbox
  */
  unsigned char type:5;
  // content-types: 0: text, 1: image
  unsigned ctype:2;
  unsigned active:1;
  unsigned monospace:1;
  unsigned int x, y;
  unsigned int width, height;
  unsigned char *text;
  unsigned char *bitmap;
  unsigned char **items;
//  unsigned char __far * __far *fitems;
  unsigned int scroll_x, scroll_y;
  unsigned int cursor_x;//, cursor_y;
  unsigned char color;
  unsigned int c;
  unsigned int p;
  int connect_element;
  void *connect_value;
  int selectc;
  unsigned select_start_row;
  unsigned select_start_col;
  unsigned select_end_row;
  unsigned select_end_col;
};

struct dropmenu
{
  unsigned char *label;
//  unsigned char **items;
  struct dropmenu *items;
  struct dropmenu *next;
  unsigned action;
};

struct menu_bar
{
//  unsigned char count;  
  struct dropmenu *menubar;
  unsigned char **upleft;
  unsigned char upleft_count;
};

struct prog_window
{
  unsigned char *title;
  int x, y;
  int x2, y2, w2, h2;
  unsigned char scrmode;
  int width, height;
  unsigned resizable:1;
  unsigned minimizable:1;
  unsigned maximized:1;
  unsigned minimized:1;
  unsigned resized:1;
  unsigned hidden:1;
  unsigned fullscreen:1;
  unsigned error_console:1;
  unsigned vt:1;
  unsigned initialized:1;
  unsigned redraw:3;
  struct window_element **window_grid;
  struct menu_bar menus;
  unsigned int element_count;
  unsigned int hitbox_count;
  struct hitbox *hitbox;
  unsigned char *icon;
  unsigned char type;
  int active_element;
  unsigned (*function_pointer)(struct prog_window*, unsigned, int*);
  void *function_status;
  int (*on_close)(struct prog_window*);
  unsigned (*on_resize)(struct prog_window*);
};

struct dialog_window
{
  /* -- dialog types --
   0: ok
   1: ok - cancel
   2: yes - no
   3: yes - no - cancel
   */
  unsigned char type;
  int x, y;
  unsigned int width, height;
  unsigned help_button:1;
  unsigned sound:3;
  unsigned char *message;
};

struct mouse_status
{
  int pointer_x;
  int pointer_y;
  unsigned left_pressed:1;
  unsigned right_pressed:1;
  unsigned hidden:1;
};

struct icon
{
  unsigned char *bitmap;
  unsigned char *text;
  int x, y;
  unsigned int value;
};

struct desktop_shortcut
{
  char shortcut_file[13];
  char path[81];
  char working_dir[81-12];
  unsigned char text[32];
  int x, y;
  unsigned char *icon;
  char *icon_path;
  unsigned char ask_args;
};

struct coords
{
  int x;
  int y;
};

struct int21time
{
  unsigned char h;
  unsigned char m;
  unsigned char s;
  unsigned char ss;
};

struct int21date
{
  unsigned short int y;
  unsigned char m;
  unsigned char d;
  unsigned char wd;
};

struct keyboard_status
{
  unsigned alt_pressed:1;
  unsigned shift_pressed:1;
  unsigned lctrl_pressed:1;
  unsigned rctrl_pressed:1;
  unsigned sysrq_pressed:1;
  unsigned scroll_lock:1;
  unsigned caps_lock:1;
  unsigned num_lock:1;
  unsigned altgr_pressed:1;
  unsigned prev_escape:1;
  unsigned ctrl_alt_del_pressed:1;
  unsigned dos_program_in_control:1;
};

struct clipboard
{
  // 0: text
  // 1: binary
  // 2: file
  unsigned content_type;
  void *content;
  unsigned len;
};

struct vt
{
  unsigned char *vbuff[25];
//  unsigned char videomode;
  unsigned x, y;
  unsigned initd:1;
  struct prog_window con;
};
