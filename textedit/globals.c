#define TEXTEDIT_KEY_DOWN -2
#define TEXTEDIT_KEY_UP -3
#define TEXTEDIT_KEY_LEFT -4
#define TEXTEDIT_KEY_RIGHT -5
#define TEXTEDIT_KEY_HOME -6
#define TEXTEDIT_KEY_BACKSPACE -7
#define TEXTEDIT_KEY_DELETE -8
#define TEXTEDIT_KEY_ENTER -9
#define TEXTEDIT_KEY_ESC -10
#define TEXTEDIT_KEY_F10 -11
#define TEXTEDIT_KEY_END -12
#define TEXTEDIT_KEY_PGUP -13
#define TEXTEDIT_KEY_PGDN -14
#define TEXTEDIT_KEY_F4 -15
#define TEXTEDIT_KEY_F3 -16
#define TEXTEDIT_KEY_F5 -17
#define TEXTEDIT_KEY_F7 -18
#define TEXTEDIT_KEY_F6 -19
#define TEXTEDIT_KEY_F9 -20
#define TEXTEDIT_KEY_F1 -21
#define TEXTEDIT_KEY_F2 -22
#define TEXTEDIT_KEY_F8 -23

#ifdef POSIX
unsigned scr_rows = 25;
unsigned scr_cols = 80;
#endif

struct file *open_files;
unsigned file_count = 0;

unsigned file_view = 0;

void draw_screen(void);
