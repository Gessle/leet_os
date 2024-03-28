//#define TTY_COUNT 4
#define TEXTMODE_VBUFF (char __far*)0xB8000000L

void set_video_mode(int mode);
#pragma aux set_video_mode = \
  "mov ah, 0x00" \
  "int 0x10" \
  parm [ax];

/*unsigned char get_video_mode(void);
#pragma aux get_video_mode = \
  "mov ah, 0x0F" \
  "int 0x10" \
  value [al];*/

void set_cursor_position_pragma(unsigned pos);
#pragma aux set_cursor_position_pragma = \
  "push ax" \
  "push bx" \
  "mov ah, 0x02" \
  "xor bx, bx" \
  "int 0x10" \
  "pop bx" \
  "pop ax" \
  parm [dx];

void set_cursor_position(unsigned row, unsigned col)
{
  set_cursor_position_pragma(row << 8 | col);
}


static void print_line(unsigned row, char *line)
{
  char __far *ptr = (char __far*)0xB8000000L + (row * 160);
  unsigned n;
  for(n=0;n<80 && line[n];n++)
  {
    if(line[n] >= ' ')
      *ptr = line[n];
    else
      *ptr = ' ';
    ptr++;
    *ptr++ = 0x07;
  }
  for(;n<80;n++)
  {
    *ptr++ = ' ';
    *ptr++ = 0x07;    
  }
}

static void draw_tty(void)
{
  unsigned n;
  for(n=0;n<25;n++)
  {
    print_line(n, ttys[tty-1].vbuff[n]);
  }
  set_cursor_position(
    ((struct console_struct*)ttys[tty-1].con.function_status)->row,
    ((struct console_struct*)ttys[tty-1].con.function_status)->col);
}

static void switch_tty(void)
{
  unsigned n;
  unsigned char prev_tty = tty;

  if(new_tty > TTY_COUNT || !ttys) return;
  tty = new_tty;
  if(tty)
  {
    if(!prev_tty)
      video_mode(0x03);
    if(!ttys[tty-1].initd)
    {
      for(n=0;n<25;n++)
        if(!(ttys[tty-1].vbuff[n] = calloc(81, 1)))
        {
          while(n--)
            free(ttys[tty-1].vbuff[n]);
          tty = 0;
          goto windowed;
        }
      run_program(0, "DOSH.APP", 0, 0, DEFAULT_PRIVS);
      ttys[tty-1].initd = 1;
      ttys[tty-1].con.vt = 1;
    }
  }
  else if(prev_tty)
  {
    windowed:
    restore_videomode();
    draw_screen();
  }
  new_tty = 0xFF;
}
