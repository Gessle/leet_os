#define _WINDOW_ICONS_LEFT 5
#define _WINDOW_ICONS_MARGIN 70
#define _WINDOW_ICON_COLOR 0x00
#define _CURSOR_COLOR 0x00
#define COLS 80
#define PATHSTRLEN 81
#define FILENAMESTRLEN 13
#define MAX_FILENAME 12
#define MAX_PATH 80

#include "vpu\sigdef.c"
#include "xpmparse\xpmstrct.c"
#include "vpu\privdef.c"

unsigned char _TITLE_COLOR = 0x00;
unsigned char _TITLE_COLOR_IA = 0x00;
unsigned char _TITLE_TEXTCOLOR = 0x01;
unsigned char _WINDOW_BGCOLOR = 0x01;
unsigned char _MENU_BGCOLOR = 0x01;
unsigned char _MENU_SELCOLOR = 0x00;
unsigned char _MENU_LINECOLOR = 0x00;
unsigned char _MENU_ATEXTCOLOR = 0x01;
unsigned char _WINDOW_BASICTEXTCOLOR = 0x00;
unsigned char _BORDERCOLOR_LIGHT = 0x00;
unsigned char _BORDERCOLOR_SHADOW = 0x00;
unsigned char _DESKTOP_BGCOLOR = 0xFF;
unsigned char _ICON_ACOLOR = 0x00;
unsigned char _ICON_ICOLOR = 0x00;
unsigned char _TEXTBOX_BGCOLOR = 0x01;
unsigned char _TEXTBOX_ABGCOLOR = 0x01;
unsigned char _TEXTBOX_TEXTCOLOR = 0x00;
unsigned char _TEXTBOX_ATEXTCOLOR = 0x01;
unsigned char _SCROLLBAR_BGCOLOR = 0x00;
unsigned char _ATEXT_BGCOLOR = 0x00;

unsigned char _FONT_HEIGHT;// = 8;

unsigned char *system_font;
unsigned char *monospace_font = 0;

unsigned char video = 0x06;

unsigned int _RES_X, _RES_Y;

// video memory of EGA & VGA
unsigned char __far *VGA;
//unsigned char __far *VGA=(unsigned char __far *)0xA0000000L;
// video memory of CGA
//unsigned char __far *VGA=(unsigned char __far *)0xB8000000L;
//unsigned char __far *VGABUFF;

//struct screen_region *mouse_hitboxes = 0;
//unsigned int mouse_hitbox_count = 0;
struct screen_region * mouse_dragboxes = 0;
struct screen_region * titlebar_drag = 0;

struct prog_window ** windows = 0;
unsigned int window_count = 0;
int * window_visibility_order;
unsigned int desktop_icon_count = 0;
struct icon *desktop_icons;
unsigned int icon_selected = 0;

struct dropmenu *desktop_menu;

// this is the ID of the currently active window. -1 if all windows are minimized or no windows are present.
int active_window = -1;

// this is the window that is DOING THINGS. if this is 0, drawing functions don't do visibility checks.
// if this is (void*)-1, every window is blocking visibility to the drawing point.
struct prog_window *running_window = 0;

// this is the console window that is running bytecode.
struct prog_window *running_console = 0;


extern unsigned char __based(__segname("_TEXT")) diskio_in_progress;
void __interrupt dosidle_handler(void);
extern void __far *old_dosindle_handler;
void set_dosidle_handler(void);
void reset_dosidle_handler(void); 
void save_dosidle_handler(void);
// function prototypes
static void set_window_resized(struct prog_window*);
static void mouse_set_normal_cursor(void);
unsigned get_pixel_raw(unsigned, unsigned);
unsigned put_pixel_raw(unsigned, unsigned, unsigned);
static void wait_leftbutton_release(void);
void hide_mouse_if_in_box(int, int, unsigned, unsigned);
static int menu_mouse_handler(struct prog_window *window, int x, int y, unsigned int width, unsigned int itemcount, unsigned int menuid);
struct mouse_movement *get_mouse_movement(void);
static void draw_mouse_cursor(void);
void hide_mouse_cursor(void);
void draw_window(struct prog_window *window, unsigned active, unsigned update);
void draw_screen(void);
static void unminimize_window(unsigned int window);
int set_active_window(int window, unsigned draw);
void nomemory(void);
static void element_active(struct prog_window *window, int element);
void shortdelay(unsigned long interval);
void draw_borders(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char type);
void window_drawelement(struct prog_window *window, unsigned int element);
static void draw_desktop(void);
static void reload_shortcuts(void);
static void save_desktop_shortcut(unsigned shortcut);
void load_system_font(char *font_file, unsigned char **font_array);
static struct prog_window *init_vpuconsole_window(char *filename);
static unsigned char checksuff(char *filename, char *suf);
unsigned char windowpos_visible(struct prog_window *window, int x, int y);
unsigned vpuconsole_program(struct prog_window *window, unsigned args, int *keypress);
void wait_refresh(void);
int send_vpu_signal(struct vpu *vpu, unsigned short signal);
void draw_xpm_bitmap(struct xpm_bitmap *bitmap, unsigned x0, unsigned y0, unsigned x1, unsigned y1);
int load_xpm_bitmap(char *filename, struct xpm_bitmap *retval);
static unsigned vpuwindow_program(struct prog_window *window, unsigned args, int *keypress);
void set_vpu_turn(struct vpu *vpu);
static void draw_select_icon(unsigned old, unsigned new);
static void move_icon(unsigned icon, int dx, int dy);
void window_radiobutton_select(struct prog_window *window, struct window_element *radiobutton);
void window_list_selectrow(struct prog_window *window, unsigned element, unsigned row);

void append_path(struct vpu *cpu, char*, char*);

void delete_console(struct console_struct *console);

void reset_vpu_interrupt_handlers(void);
void set_vpu_interrupt_handlers(void);

char system_font_file[] = "FONTS/CGA.FON";
char monospace_font_file[] = "FONTS/CGAMON.FON";

unsigned char VPUCON_LINES = 15;
//#define VPUCON_COLS 60

unsigned int window_x = 40;
unsigned int window_y = 40;

extern char *message;

unsigned char **default_icons;
char *path;

char comsuf[] = ".COM";
char exesuf[] = ".EXE";
char batsuf[] = ".BAT";
char appsuf[] = ".APP";
char lnksuf[] = ".LNK";

struct desktop_shortcut *desktop_shortcuts;

unsigned int shortcut_count = 0;

unsigned int screensaver_timer;
unsigned int screensaver_delay = 0;

char screensaver_program[13];
char wallpaper[13];
struct xpm_bitmap wallpaper_struct;

// if this is true, the main loop redraws screen unconditionally and then sets this false.
unsigned char screen_redraw=1;


struct mouse_status *mouse;

extern unsigned vpu_count;
extern struct vpu **vpus;

char cwd[MAX_PATH+1];
unsigned cwdrive;

int videodriver = -1;

//extern unsigned disable_vpu_interrupts;

unsigned char null_byte = 0;


void _far (*update_screen)(void) = 0;
void _far (*old_put_pixel)();
unsigned _far (*old_get_pixel)();
void _far (*old_draw_rectangle)();
void _far (*old_fill_screen)();
unsigned old_xres, old_yres;

extern void _far *fill_screen_color;
extern unsigned char _VIDEO_MODE;
extern int ega4_lastoperation;
void put_pixel(unsigned int x, unsigned int y, unsigned color);
void scrmemcpy(unsigned x, unsigned y, unsigned width, unsigned height, int dx, int dy);
unsigned char get_pixel(unsigned int x, unsigned int y);
void draw_line(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, unsigned char color);
void draw_rectangle(int x0, int y0, unsigned int width, unsigned int height, unsigned color);
void draw_move_borders(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char phase);
void put_pixel_shadow(unsigned int x, unsigned int y, unsigned int color, unsigned int shadow);
void fill_screen(unsigned color);
void restore_videomode(void);
void draw_circle_border(unsigned x, unsigned y, unsigned r, unsigned char type);
void draw_filled_circle(unsigned x, unsigned y, unsigned r, unsigned color);
#ifdef MAIN
void video_mode(unsigned char mode);
#endif
void _far draw_rectangle_px(int x0, int y0, unsigned int width, unsigned int height, unsigned color);
#pragma aux draw_rectangle_px parm [ax] [bx] [cx] [dx] [si];

//void reset_int23(void);
//void set_int23(void);
void reset_int24(void);
void set_int24(void);
void set_keyboard_handler(void);
void reset_keyboard_handler(void);
void unload_drivers(void);

void vpu_push_word(struct vpu *vpu, unsigned short word);

static void call_drv_screenupdate(void);

static unsigned char infotitle[] = "Info";

unsigned char inbyte(unsigned port);
#pragma aux inbyte = \
  "in al, dx" \
  parm [dx] value [al];

void outbyte(unsigned port, unsigned char byte);
#pragma aux outbyte = \
  "out dx, al" \
  parm [dx] [ax];

int all_proc_blocking(void);

extern unsigned DEFAULT_PRIVS;
extern unsigned starttime_high;
extern unsigned starttime_low;

struct unixtime unixtime(long offset);

extern struct keyboard_status keyboard_status;

volatile signed next_window;
volatile signed next_tty;

#ifdef MAIN
struct clipboard clipboard;
#endif

void halt(void);
#ifndef NO_HALT
#pragma aux halt = \
  "hlt";
#else
#pragma aux halt = \
  "nop";
#endif

char *comspec;

#define ERRORLOG_SIZE 512
char *errorlog;

void show_error_console();

volatile unsigned char tty = 0;
struct vt *ttys;
volatile unsigned char new_tty = 0xFF;

int new_console(struct console_struct *, char *filename);

static void switch_tty(void);

unsigned caret_screenx, caret_screeny;
char caret_visible = 0;

#define BUILDSTR86 "8086 build"
#define BUILDSTR186 "i186 build"
#define BUILDSTR286 "i286 build"
#define BUILDSTR386 "i386 build"
#define BUILDSTR486 "i486 build"
#define BUILDSTR586 "i586 build"
#define BUILDSTR686 "i686 build"

#ifdef __SW_0
  #define CPUBUILDSTR BUILDSTR86
#endif
#ifdef __SW_1
  #define CPUBUILDSTR BUILDSTR186
#endif
#ifdef __SW_2
  #define CPUBUILDSTR BUILDSTR286
#endif
#ifdef __SW_3
  #define CPUBUILDSTR BUILDSTR386
#endif
#ifdef __SW_4
  #define CPUBUILDSTR BUILDSTR486
#endif
#ifdef __SW_5
  #define CPUBUILDSTR BUILDSTR586
#endif
#ifdef __SW_6
  #define CPUBUILDSTR BUILDSTR686
#endif

static unsigned char versioninfo[] = "lEEt/OS 0.9.490 "CPUBUILDSTR" - (c) Sami Tikkanen 2024";
