#define SCR_COLS 80
#define SCR_ROWS 25

unsigned char __far *VGA = (unsigned char __far*)0xB0008000L;

void print_char(unsigned col, unsigned row, unsigned char chr, unsigned char color)
{
  unsigned char __far *dst = &VGA[((row)*SCR_COLS+col)<<1];
  if(col >= SCR_COLS || row >= SCR_ROWS)
    return;
  *dst++ = chr;
  *dst = color;
}

unsigned char get_char(unsigned col, unsigned row)
{
  return VGA[((row)*SCR_COLS+col)<<1];
}

void set_color(unsigned col, unsigned row, unsigned char color, unsigned count)
{
  unsigned char __far *dst = (&VGA[((row)*SCR_COLS+col)<<1]) + 1;

  while(count--)
  {
    *dst = color;
    dst += 2;
  }  
}

int getkeypress(void)
{
  int ch = getch();

  retry:if(ch)
  {
    if(ch == 8)
      return TEXTEDIT_KEY_BACKSPACE;
    if(ch == 13)
      return TEXTEDIT_KEY_ENTER;
    if(ch == 27)
      return TEXTEDIT_KEY_ESC;
    return ch;
  }
  switch(getch())
  {
    case 73: return TEXTEDIT_KEY_PGUP;
    case 81: return TEXTEDIT_KEY_PGDN;
    case 80: return TEXTEDIT_KEY_DOWN;
    case 72: return TEXTEDIT_KEY_UP;
    case 75: return TEXTEDIT_KEY_LEFT;
    case 77: return TEXTEDIT_KEY_RIGHT;
    case 71: return TEXTEDIT_KEY_HOME;
    case 79: return TEXTEDIT_KEY_END;
    case 83: return TEXTEDIT_KEY_DELETE;
    case 68: return TEXTEDIT_KEY_F10;
    case 67: return TEXTEDIT_KEY_F9;
    case 66: return TEXTEDIT_KEY_F8;
    case 65: return TEXTEDIT_KEY_F7;
    case 64: return TEXTEDIT_KEY_F6;
    case 63: return TEXTEDIT_KEY_F5;
    case 62: return TEXTEDIT_KEY_F4;
    case 61: return TEXTEDIT_KEY_F3;
    case 60: return TEXTEDIT_KEY_F2;
    case 59: return TEXTEDIT_KEY_F1;
    default: goto retry;
  }
}

void end_program(void)
{

}

void init_screen(void)
{
  __asm
  {
    mov ah, 0x00
    mov al, 0x03
    int 0x10
  }
  
//  scr_cols = SCR_COLS;
//  scr_rows = SCR_ROWS;
#define scr_cols SCR_COLS
#define scr_rows SCR_ROWS
}

void scr_cursor(unsigned char y, unsigned char x)
{
  __asm
  {
    mov ah, 0x02
    mov bh, 0
    mov dh, y
    mov cl, x
    int 0x10
  }
}
