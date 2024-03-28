#include <signal.h>
#include <unistd.h>

unsigned color_attr(unsigned char color)
{
  unsigned retval = 0;
  if(color & 0x08)
    retval |= A_BOLD;
  return retval;
}

void print_char(unsigned x, unsigned y, char chr, unsigned char color)
{
  mvaddch(y, x, chr | COLOR_PAIR(color&0x77) | color_attr(color));
}

unsigned char get_char(unsigned x, unsigned y)
{
  return mvinch(y, x);
}

void set_color(unsigned x, unsigned y, unsigned char color, unsigned count)
{
  mvchgat(y, x, count, color_attr(color), color & 0x77, 0);
}

int getkeypress(void)
{
  int ch = getch();

  switch(ch)
  {
    case KEY_PPAGE: return TEXTEDIT_KEY_PGUP;
    case KEY_NPAGE: return TEXTEDIT_KEY_PGDN;
    case KEY_DOWN: return TEXTEDIT_KEY_DOWN;
    case KEY_UP: return TEXTEDIT_KEY_UP;
    case KEY_LEFT: return TEXTEDIT_KEY_LEFT;
    case KEY_RIGHT: return TEXTEDIT_KEY_RIGHT;
    case KEY_HOME: return TEXTEDIT_KEY_HOME;
    case KEY_END: return TEXTEDIT_KEY_END;
    case KEY_BACKSPACE: return TEXTEDIT_KEY_BACKSPACE;
    case KEY_DC: return TEXTEDIT_KEY_DELETE;
    case 10: case KEY_ENTER: return TEXTEDIT_KEY_ENTER;
    case KEY_F(10): return TEXTEDIT_KEY_F10;
    case KEY_F(4): return TEXTEDIT_KEY_F4;
    case KEY_F(3): return TEXTEDIT_KEY_F3;
    case KEY_F(5): return TEXTEDIT_KEY_F5;
    case KEY_F(7): return TEXTEDIT_KEY_F7;
    case KEY_F(6): return TEXTEDIT_KEY_F6;
    case KEY_F(9): return TEXTEDIT_KEY_F9;
    case KEY_F(1): return TEXTEDIT_KEY_F1;
    case KEY_F(2): return TEXTEDIT_KEY_F2;
    case KEY_F(8): return TEXTEDIT_KEY_F8;
    case 27: return TEXTEDIT_KEY_ESC;
    default: return ch;
  }
}

void screen_size_change(int signal)
{
  scr_cols = COLS;
  scr_rows = LINES;
}

void end_program(void)
{
  endwin();
}

void init_screen(void)
{
  initscr();
  signal(SIGWINCH, screen_size_change);
  scr_cols = COLS;
  scr_rows = LINES;
  cbreak();
  noecho();
  keypad(stdscr, true);
  start_color();

  // VGA text mode colors...
  init_pair(0x00, COLOR_BLACK, COLOR_BLACK);
  init_pair(0x01, COLOR_BLUE, COLOR_BLACK);
  init_pair(0x02, COLOR_GREEN, COLOR_BLACK);
  init_pair(0x03, COLOR_CYAN, COLOR_BLACK);
  init_pair(0x04, COLOR_RED, COLOR_BLACK);
  init_pair(0x05, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(0x06, COLOR_YELLOW, COLOR_BLACK);
  init_pair(0x07, COLOR_WHITE, COLOR_BLACK);

  init_pair(0x20, COLOR_BLACK, COLOR_GREEN);

  atexit(end_program);
}

void scr_cursor(unsigned y, unsigned x)
{
  move(y, x);
  refresh();
}
