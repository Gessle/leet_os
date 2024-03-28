// interrupt handler to catch error messages from stdout and print
// them to the screen so that they are readable

void __far __interrupt (*old_int10h)();

/*unsigned draw_char(unsigned char, unsigned, unsigned, unsigned, unsigned, unsigned char *);
extern unsigned char *monospace_font;
extern unsigned char _FONT_HEIGHT;
void draw_rectangle(int, int, unsigned, unsigned, unsigned);*/

void text_mode(void);
#pragma aux text_mode = \
  "push ax" \
  "mov ax, 0x0083" \
  "int 0x10" \
  "pop ax";

static void reset_int10h(void)
{
  _dos_setvect(0x10, old_int10h);
}

static void __interrupt int10handler(union INTPACK regs)
{
//  static unsigned y = 0;
//  static unsigned x = 0;

  if(regs.h.ah == 0xE)
  {
/*    draw_rectangle(x, y, 8, _FONT_HEIGHT, 0x0F);
    draw_char(regs.h.al, x, y, 0x00, _FONT_HEIGHT, monospace_font);
    if(x+=8 == 640)
    {
      x = 0;
      y += _FONT_HEIGHT+2;
    }*/
    text_mode();
    reset_int10h();
  }
//  else
    _chain_intr(old_int10h);
}

static void set_int10h(void)
{
  old_int10h = _dos_getvect(0x10);
  _dos_setvect(0x10, int10handler);
}
