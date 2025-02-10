//#define SCR_ROWS VPUCON_LINES
#define SCR_COLS VPUCON_COLS
#define TERMINAL_WIDTH VPUCON_COLS*8+3

char vpuconsole_window_title[] = "VPU Console";
//char vpu_welcome_msg[] = "Virtual CPU Console v. 0.1 alpha";

extern struct vpu **vpus;

extern unsigned char debug;

extern unsigned char SCR_ROWS;

extern unsigned line_num;

void delete_vpu(struct vpu *vpu);

unsigned *cursor_row;
unsigned *cursor_col;

void vpucon_caret(unsigned row, unsigned col, unsigned char remove)
{
  unsigned char linecolor;
  if(clock_ticks & 0x04 && !remove) linecolor=_TEXTBOX_TEXTCOLOR;
  else linecolor=_TEXTBOX_BGCOLOR;

  draw_line(running_console->x+running_console->window_grid[0]->x+col*MONOSPACE_WIDTH+1,
      running_console->y+running_console->window_grid[0]->y+(2+row)*(_FONT_HEIGHT+2),
      running_console->x+running_console->window_grid[0]->x+col*MONOSPACE_WIDTH+MONOSPACE_WIDTH-2,
      running_console->y+running_console->window_grid[0]->y+(2+row)*(_FONT_HEIGHT+2), linecolor);    
}

inline void vpuconsole_hiderow(unsigned row)
{
  struct window_element *elem = running_console->window_grid[0];
  if(!running_console->vt)
    draw_rectangle(running_console->x+elem->x,
    running_console->y+elem->y+_FONT_HEIGHT+2+row*(_FONT_HEIGHT+2),
    strlen(elem->items[row])*MONOSPACE_WIDTH+MONOSPACE_WIDTH,
    _FONT_HEIGHT+3, _TEXTBOX_BGCOLOR);
}

inline void vpuconsole_clearrow(unsigned row)
{
  vpuconsole_hiderow(row);
  memset(((struct console_struct*)running_console->function_status)->con_lines[row], 0, VPUCON_COLS);
}

void clear_row(unsigned row)
{
  vpuconsole_clearrow(row);
}

static void vpuconsole_printchar(unsigned char chr);

inline void vpuconsole_clearchar(void)
{
  struct console_struct *console_status = running_console->function_status;      
  struct window_element *elem = running_console->window_grid[0];

  if(!running_console->vt)
    draw_rectangle(running_console->x+elem->x+(*cursor_col)*MONOSPACE_WIDTH+2,
    running_console->y+elem->y+_FONT_HEIGHT+2+(*cursor_row)*(_FONT_HEIGHT+2),
      MONOSPACE_WIDTH*2, _FONT_HEIGHT+3, _TEXTBOX_BGCOLOR);
    
  console_status->con_lines[console_status->row][console_status->col] = ' ';
}

static void clear_char()
{
  vpuconsole_clearchar();
}

inline void vpuconsole_clearscr(void)
{
  struct console_struct *console_status = running_console->function_status;      
  unsigned n = console_status->lines;
  while(n--)
    vpuconsole_clearrow(n);
    
  console_status->row=0;
  console_status->col=0;
}

void clear_console(void)
{
  vpuconsole_clearscr();
}

inline unsigned mul_80(unsigned num)
{
  return (num << 6) + (num << 4);
}

inline void fastscroll(void)
{
  struct window_element *elem = running_console->window_grid[0];
  int top = running_console->y+_FONT_HEIGHT+2+elem->y;
  int left = running_console->x+elem->x;
  int scroll_width = VPUCON_COLS*MONOSPACE_WIDTH;
  int scroll_height = elem->height-_FONT_HEIGHT-3;
  register unsigned row, n;
  char __far *dstbuff, *srcbuff;
  int right;
  struct console_struct *console_status = running_console->function_status;        
  unsigned char pixbuff[8];

  if(left<0)
  {
    scroll_width += left;
    left=0;
  }
  if(top<0)
  {
    scroll_height += top;
    top=0;
  }
  if(top+scroll_height+_FONT_HEIGHT+2 >= _RES_Y)
    scroll_height = _RES_Y - (top+_FONT_HEIGHT+2) - 1;
  if(left+scroll_width >= _RES_X)
    scroll_width = _RES_X - left - 1;
  if(scroll_height<0 || scroll_width<0) return;

  right = left+scroll_width;

  hide_mouse_if_in_box(left, top, scroll_width, elem->height);

  if(videodriver != -1)
  {
    scrmemcpy(left, top+_FONT_HEIGHT+2, scroll_width, 
              scroll_height, 0, -(_FONT_HEIGHT+2));
    return;
  }

  scroll_width >>= 3;
  if(right != _RES_X)
    scroll_width--; 

  for(row=0;row<scroll_height;row++)
  {
    for(n=7-(left&7);n;n--)
      pixbuff[n] = get_pixel(left+n, top+row+_FONT_HEIGHT+2);
    for(n=7-(left&7);n;n--)
      put_pixel(left+n, top+row, pixbuff[n]);
    for(n=right&7;n;n--)
      pixbuff[n] = get_pixel(right-n, top+row+_FONT_HEIGHT+2);
    for(n=right&7;n;n--)
      put_pixel(right-n, top+row, pixbuff[n]);
  }

  left >>= 3;    
  left++;

  if(_VIDEO_MODE != 0x06)
  {
    if(_VIDEO_MODE == 0x10 || _VIDEO_MODE == 0x12)
    {
      if(ega4_lastoperation)
      {
        outword(0x3CE, 0x0205);
  
        outbyte(0x3CE, 0x08);
        ega4_lastoperation = 0;
      }
      outbyte(0x3CF, 0x00);
    }    
    for(row=0;row<scroll_height;row++)
    {
      dstbuff = &VGA[mul_80(top+row)+left];
      srcbuff = &VGA[mul_80(top+row+_FONT_HEIGHT+2)+left];
      // In planar display modes movsw isn't safe, so had to implement own memcpy      
      copybytes(dstbuff, srcbuff, scroll_width);
    }
  }
  else
  {
    for(row=0;row<scroll_height;row++)
    {
      dstbuff = &VGA[mul_80((top+row)>>1)+left];
      srcbuff = &VGA[mul_80((top+row+_FONT_HEIGHT+2)>>1)+left];
      if((row&0x01))
      {
        dstbuff += 0x2000;
        srcbuff += 0x2000;
      }
      
      memcpy(&dstbuff[n], &srcbuff[n], scroll_width);      
    }
  }
  
//  ega4_lastoperation=-1;
//  vpuconsole_clearrow(window, console_status->lines-1);

}

inline void vpuconsole_scrollup(void)
{
  unsigned row;
  struct console_struct *console_status = running_console->function_status;      
  unsigned element_x, element_y;
  struct window_element *element;

  if(!tty)
    if(windows[active_window] == running_console && _VIDEO_MODE != 0x13)
      fastscroll();
    else
      if(!windows[active_window]->fullscreen)
      {
        element = running_console->window_grid[0];
        element_x = running_console->x+element->x+2;
        element_y = running_console->y+_FONT_HEIGHT+2+element->y;
        for(row=0;row < console_status->lines-1;row++)
        {
          vpuconsole_hiderow(row);
          gprint_text(element_x, element_y+2+row*(_FONT_HEIGHT+2),
                      element->items[row+1], 0, _FONT_HEIGHT, _RES_X, 1);
        }
      }
//        window_drawelement(window, 0);      

//  vpuconsole_clearrow(window, console_status->lines-1);

  row = 0;
  do
    strcpy(console_status->con_lines[row], console_status->con_lines[row+1]);
  while(++row < console_status->lines-1);

  vpuconsole_clearrow(console_status->lines-1);
}

inline void vpuconsole_incrow(void)
{
  struct console_struct *console_status = running_console->function_status;
  if(console_status->col < VPUCON_COLS)
    vpucon_caret(console_status->row, console_status->col, 1);  
  if(++console_status->row == console_status->lines)
  {
    console_status->row--;
    vpuconsole_scrollup();
  }
  console_status->col=0;
//  window_drawelement(running_console, 0);    
}

void inc_row(void)
{
  vpuconsole_incrow();
}

void move(unsigned row, unsigned col)
{
  struct console_struct *console_status = running_console->function_status;
  vpucon_caret(console_status->row, console_status->col, 1);
  if(col >= VPUCON_COLS) col = VPUCON_COLS-1;
  if(row >= console_status->lines) row = console_status->lines-1;
  console_status->row = row;
  console_status->col = col;
}

static void vpuconsole_printchar(unsigned char chr)
{
  struct console_struct *console_status = running_console->function_status;
//  unsigned line_len = strlen(((struct console_struct*)window->function_status)->con_lines[console_status->row]);
  unsigned line_len;
  struct window_element *elem = running_console->window_grid[0];

  vpucon_caret(console_status->row, console_status->col, 1);

  // if line already continues over the current caret position, draw whitespace over the current character in the position
//  if(console_status->col < line_len)
  if(!running_console->vt)
    if(((struct console_struct*)running_console->function_status)->con_lines[console_status->row][console_status->col])
    draw_rectangle(running_console->x+elem->x+(console_status->col)*MONOSPACE_WIDTH+2,
      running_console->y+elem->y+_FONT_HEIGHT+2+(console_status->row)*(_FONT_HEIGHT+2),
        8, _FONT_HEIGHT+3, _TEXTBOX_BGCOLOR);

  // if the length of the current line does not reach the current caret position, fill the space with whitespaces
    else if(console_status->col && !((struct console_struct*)running_console->function_status)->con_lines[console_status->row][console_status->col-1])
    {
      line_len = strlen(((struct console_struct*)running_console->function_status)->con_lines[console_status->row]);
      memset(&((struct console_struct*)running_console->function_status)->con_lines[console_status->row][line_len], ' ', console_status->col-line_len);
    }

//  else
    draw_char(chr, running_console->x+elem->x+console_status->col*MONOSPACE_WIDTH+2, running_console->y+_FONT_HEIGHT+4+elem->y+console_status->row*(_FONT_HEIGHT+2),
      _TEXTBOX_TEXTCOLOR, _FONT_HEIGHT, monospace_font);

  console_status->con_lines[console_status->row][console_status->col++] = chr;
//  ((struct console_struct*)window->function_status)->con_lines[console_status->row][console_status->col] = 0;
  
  if(console_status->col == VPUCON_COLS)
    vpuconsole_incrow();
}

static void mvaddch(unsigned row, unsigned col, unsigned char chr)
{
  unsigned oldrow = *cursor_row;
  unsigned oldcol = *cursor_col;
  move(row, col);
  vpuconsole_printchar(chr);
  move(oldrow, oldcol);
}

static void vpuconsole_printtxt(unsigned char *string)
{
  while(*string)
      vpuconsole_printchar(*string++);
}

static void save_cursor_position(void)
{
  struct console_struct *console_status = running_console->function_status;

  console_status->srow = console_status->row;
  console_status->scol = console_status->col;
}

static int con_write_stdin(unsigned char chr);

static void restore_cursor_position(void)
{
  struct console_struct *console_status = running_console->function_status;

  vpucon_caret(console_status->row, console_status->col, 1);
  console_status->row = console_status->srow;
  console_status->col = console_status->scol;
}

static void device_status_report(void)
{
  con_write_stdin(0x1B);
  con_write_stdin(0x5B);
  con_write_stdin(*cursor_row);
  con_write_stdin(*cursor_col);
  con_write_stdin('R');  
}

static unsigned char parse_ansi_arg(unsigned char **string, unsigned *length)
{
  unsigned char retval;
  retval = atoi(*string);
  if(**string >= '0' && **string <= '9')
    while(**string >= '0' && **string <= '9')
    {
      ++*string;
      if(!--*length)
        return 0;
    }
  else return 1;
  return retval;
}

static void parse_ansi_code(unsigned char **string, unsigned *length)
{
  unsigned char arg1, arg2;
  struct console_struct *console_status = running_console->function_status;  
  
  if(!*length)
    return;
  arg1 = parse_ansi_arg(string, length);
  if(!*length)
    return;
  --*length;    
  switch(**string) // opcode
  {
    case 'A': // move up
      move(*cursor_row-arg1, *cursor_col);
      break;
    case 'B': // move down
      move(*cursor_row+arg1, *cursor_col);
      break;
    case 'C': // move right
      move(*cursor_row, *cursor_col+arg1);
      break;
    case 'D': // move left
      move(*cursor_row, *cursor_col-arg1);
      break;
    case ';':
      if(!*length)
        return;
      ++*string;
      arg2 = parse_ansi_arg(string, length);
      if(!*length)
        return;
      --*length;
      switch(**string) // opcode
      {
        case 'f': case 'H': // move to arg1, arg2
          move(arg1-1, arg2-1);
        break;
      }
      break;
    case 'J': // clear display
      clear_console();
      draw_window(running_console, 0, 1);
      break;
    case 'K': // clear row
      clear_row(*cursor_row);
      draw_window(running_console, 0, 1);      
      break;
    case 's': // save cursor position
      save_cursor_position();
      break;
    case 'u': // restore cursor position
      restore_cursor_position();
      break;
    case 'n':
      if(arg1 == 6) // device status report
        device_status_report();
      break;
    case 0x11: // local echo off
      console_status->noecho = arg1;
    break;
    case 0x12: // no line buffering
      console_status->nolinebuffer = arg1;
    break;
  }
}

void flush_stdout(void)
{
  unsigned char *string = ((struct console_struct*)running_console->function_status)->con_stdout.buff;
  unsigned *length = &((struct console_struct*)running_console->function_status)->con_stdout.pos;
  
  while((*length)--)
  {
    if(*string == 0x07)
      make_sound(1);
    else if(*string == 0x1B && *length)
    {
      if(string[1] == 0x5B)
      {
        --*length;
        string+=2;
        parse_ansi_code(&string, length);
      }
    }
    else
      vpuconsole_printchar(*string);
    string++;
  }
}

void print_text(unsigned row, unsigned col, unsigned char *str)
{
  move(row, col);
  vpuconsole_printtxt(str);    
}

static void con_printstr(unsigned char *str)
{
  vpuconsole_printtxt(str);
}

inline void vpuconsole_putstr(unsigned char *string)
{
  vpuconsole_printtxt(string);
  vpuconsole_incrow();
}

void putstr(unsigned char *string)
{
  vpuconsole_putstr(string);
}

void refresh(void)
{
  window_drawelement(running_console, 0);    
}

void scroll_up(void)
{
  vpuconsole_scrollup();
}

static void printchar(unsigned char chr)
{
  vpuconsole_printchar(chr);
}

int vpuconsole_getch(void)
{
  int retval;
  struct console_struct *console_status = running_console->function_status;

/*  if(console_status->keybuff[0] != -1)
  {
//    console_status->waiting_for_key=0;
    retval = console_status->keybuff[0];
    if(!retval)
      console_status->keybuff[0] = console_status->keybuff[1];
    else
      console_status->keybuff[0] = -1;
    console_status->keybuff[0] = console_status->keybuff[1];
//    if(console_status->keybuff[0] < 0)
//      console_status->waiting_for_key = 0;
      console_status->keybuff[1] = -1;
    return retval;
  }*/
//  console_status->waiting_for_key=0;
//  return -1;
  retval = console_status->keybuff[0];
  console_status->keybuff[0] = console_status->keybuff[1];
  console_status->keybuff[1] = -1;
  return retval;
}

/*int getchr(void)
{
  int c = getch();
  printchar(c);
  return c;
}*/

static int vpuconsole_keypress(struct prog_window *window, int *keypress)
{
  struct console_struct *console_status = window->function_status;

  console_status->keybuff[0] = *keypress;
  if(!*keypress)
    console_status->keybuff[1] = getch();
  
  return 0;
}

//#include "vpu/console/main.c"
unsigned char commandline(struct console_struct *);

void code_editor(struct console_struct *);

unsigned char vpu_run_program(struct console_struct *vpu);

unsigned char vpu_step(struct vpu *vpu);

extern unsigned symbol_count;
extern unsigned char **data;
extern unsigned datap_segment;
extern unsigned datap_offset;
extern void *symbols;
extern unsigned char pass;

unsigned char vpu_has_active_window(struct vpu *vpu)
{
  if(windows[active_window] == running_console
   || ((struct vpu_progwindow *)windows[active_window]->function_status)->vcpu == ((struct console_struct *)running_console->function_status)->vcpu)
    return 1;
  return 0;
}

void vpu_cwd(struct vpu *vpu)
{
  static unsigned char chddir = 0;
  unsigned char drive;
  char drv_cwd[MAX_PATH];
  
  if(vpu)
  {
    drive = vpu->cwd[0] - 'A' + 1;
    if(drive > 32) drive -= 32;

    _getdcwd(drive, drv_cwd, MAX_PATH);    

    if(strcmp(drv_cwd, vpu->cwd))
    {
      if(chdir(vpu->cwd))
        strcpy(vpu->cwd, cwd);
      else if(drive == cwdrive)
        chddir = 1;
    }
  }
  else if(chddir && !diskio_in_progress)
  {
    chdir(cwd);
    chddir = 0;
  }
}

void con_move_back(void)
{
  if(!*cursor_col)
    move(*cursor_row-1, VPUCON_COLS-1);
  else
    move(*cursor_row, *cursor_col-1);
}

static int con_write_stdin(unsigned char chr)
{
  void *new_pointer;

  if(chr == 3)
  {
    ((struct console_struct*)running_console->function_status)->vcpu->signal = SIGINT;
    return 0;
  }

  if(chr == 19)
  {
    ((struct console_struct*)running_console->function_status)->vcpu->signal = SIGSTOP;
    con_printstr("^S");
    return 0;
  }

  if(chr == 17)
  {
    ((struct console_struct*)running_console->function_status)->vcpu->signal = SIGCONT;
    con_printstr("^Q");
    return 0;
  }

  if(chr == 26)
  {
    ((struct console_struct*)running_console->function_status)->vcpu->signal = SIGTSTP;
    con_printstr("^Z");
    return 0;
  }  

  if(chr == 4) con_printstr("^D");

  if(chr == 8)
  {
    if(!((struct console_struct*)running_console->function_status)->nolinebuffer)
    {
      if(((struct console_struct*)running_console->function_status)->input_col)
      {
        if(!((struct console_struct*)running_console->function_status)->noecho)
        {
          con_move_back();
          clear_char();
        }
        ((struct console_struct*)running_console->function_status)->con_stdin.buff
          [--((struct console_struct*)running_console->function_status)->input_col] = 0;
      }
      return 0;
    }
    else
      if(!((struct console_struct*)running_console->function_status)->noecho)
        con_move_back();
  }

  if(chr == 13) chr = 0xA;
  
  ((struct console_struct*)running_console->function_status)->con_stdin.buff
    [((struct console_struct*)running_console->function_status)->input_col++] = chr;

  if(!((struct console_struct*)running_console->function_status)->noecho)
  {
    if(chr == 0xA)
      inc_row();
    else if(chr != 8)
      printchar(chr);
  }

  if(((struct console_struct*)running_console->function_status)->input_col
    == ((struct console_struct*)running_console->function_status)->con_stdin.len-1)
  {
    new_pointer = realloc(((struct console_struct*)running_console->function_status)->con_stdin.buff,
      (((struct console_struct*)running_console->function_status)->con_stdin.len += 32)*sizeof(char));
    if(!new_pointer)
      return -1;
    ((struct console_struct*)running_console->function_status)->con_stdin.buff = new_pointer;
  }

  ((struct console_struct*)running_console->function_status)->con_stdin.buff
    [((struct console_struct*)running_console->function_status)->input_col] = 0;

  return 0;
}

char *getcommand(struct console_struct *con_struct, char *retval);

int prepare_vpu_steps(struct vpu *vpu);

unsigned char vpu_run(struct vpu *vpu);

unsigned vpuconsole_program(struct prog_window *window, unsigned args, int *keypress)
{
  unsigned n;
  int c;
  struct console_struct *console_status = window->function_status;
  
  data = console_status->vcpu->data;
  SCR_ROWS = console_status->lines;
  running_console = window;
  cursor_row = &console_status->row;
  cursor_col = &console_status->col;

  vpucon_caret(*cursor_row, *cursor_col, 0);

  if(args & WM_WINPROGACT) switch(args & ~WM_WINPROGACT)
  {
    case 1:
      console_status->noecho = ~console_status->noecho;
      break;
    case 2:
      console_status->nolinebuffer = ~console_status->nolinebuffer;
      break;
  }

  if(console_status->runmode == 3)
    goto runmode3;
  else
  {
    symbol_count = console_status->symbol_count;    
    line_num = console_status->asm_line;
    datap_segment = console_status->datap_segment;
    datap_offset = console_status->datap_offset;
    debug = console_status->debug;
    symbols = console_status->symbols;
  }

  if(console_status->vcpu->exiting)
  {
    sprintf(message, "Exited with return code %u", console_status->vcpu->regs[1]);
    putstr(message);
    console_status->vcpu->exiting = 0;
    console_status->runmode = 0;
  }
 
  if(!console_status->editmode)
  {
    if(console_status->runmode == 3)
    {
      runmode3:
        switch(vpu_run(console_status->vcpu))
        {
//          case 3: goto end;            
          case 0: break;
          case 1:
            show_error_msg("Error!");
          case 2:
            goto close_con;
        }
     goto end;
    }
    else if(console_status->runmode == 2)
    {
      if(vpu_run_program(console_status))
      {
        console_status->runmode = 0;
        pass = 0;
      }
    }
    else if(console_status->runmode == 1)
    {
      vpu_run_program(console_status);
      pass = 0;
      console_status->runmode = 0;      
    }
    else
      if(!commandline(console_status))
        goto close_con;
  }
  else
    code_editor(console_status);

  // if another console has the focus for some reason - if, for example, a new VPU process has been spawned by this console -
  // then do not save these
//  if(running_console == window) // not needed anymore
  {
    console_status->asm_line = line_num;
    console_status->symbol_count = symbol_count;
    console_status->datap_segment = datap_segment;
    console_status->datap_offset = datap_offset;
    console_status->debug = debug;
    console_status->symbols = symbols;
  }       
  
  end:

  vpu_cwd(0);
  if(keypress)
  {
    vpuconsole_keypress(window, keypress);
    *keypress = -1;
    if(console_status->runmode)
    {
      while(console_status->keybuff[0] >= 0)
      {
        if(con_write_stdin(vpuconsole_getch()))
          putstr(outofmemory);
      }
    }
  } 

  return ~WM_WINPROGACT;
  close_con:
  vpu_cwd(0);
  for(n=window_count;n--;)
    if(windows[n] == running_console)
    {
      windows[n]->hidden=0;
      set_active_window(n, 0);
      return 2;
    }
  for(n=TTY_COUNT;n--;)
    if(&ttys[n].con == running_console)
    {
      ttys[n].initd = 0;
      if(tty-1 == n) new_tty = 0;
      for(c=25;c--;)
        free(ttys[n].vbuff[c]);
      delete_console(ttys[n].con.function_status);
      return 0;
    }
}

//int new_console(struct console_struct *, char *filename);

//extern char response_ready[];

static struct prog_window *init_vpuconsole_window(char *filename)
{
  struct prog_window *retval;
  struct window_element *element;
  unsigned n = VPUCON_LINES;
  unsigned char **con_lines;// = calloc(VPUCON_LINES, sizeof(char*));
  char *title;
  void *old_running_console = running_console;
  int console_rc;
  char spawn_path[MAX_PATH+1];

  if(!filename)
    title = vpuconsole_window_title;
  else
    title = filename;

  getcwd(spawn_path, MAX_PATH);
  chdir(cwd);

  chdir(spawn_path);

  if(!tty)
  {
    if(!(retval = new_window(0, -1, TERMINAL_WIDTH+2, (2+_FONT_HEIGHT)*VPUCON_LINES+3, title, 0, 1, "ICONS/VPUCON.ICO", &vpuconsole_program)))
    {
      nomemory:
      nomemory();    
      return 0;
    }

    if(!(con_lines = calloc(VPUCON_LINES, sizeof(char*))))
      goto deletewindow;

    while(n--)
    {
      con_lines[n] = calloc(VPUCON_COLS+1, sizeof(char));
      if(!con_lines[n])
      {
        deletelines:
        while(++n < VPUCON_LINES)
          free(con_lines[n]);
        free(con_lines);        
        goto deletewindow;
      }
    }
  
    if(!(element = window_list(retval, 1, 1, TERMINAL_WIDTH, (2+_FONT_HEIGHT)*VPUCON_LINES+1, con_lines, VPUCON_LINES)))
    {
      goto deletelines;
      deletewindow:
      del_window(retval);
      goto nomemory;
    }
    free(con_lines);

    element->active = 0;
    element->monospace = 1;

    if(!(retval->menus.menubar = new_menu("Term", 0)))
      goto deletewindow;
    if(!new_menuitem(retval->menus.menubar, "Local echo", (1 | WM_WINPROGACT)))
      goto deletewindow;
    if(!new_menuitem(retval->menus.menubar, "Line buffer", (2 | WM_WINPROGACT)))
      goto deletewindow;
  

    draw_menubar();

    draw_window(retval, 1, 1);
  }
  else
  {
    retval = &ttys[tty-1].con;
//    retval->function_pointer = vpuconsole_program;
  }

  if(!(retval->function_status = calloc(1, sizeof(struct console_struct))))
    goto deletewindow;
  ((struct console_struct*)retval->function_status)->keybuff[0] = -1;
  ((struct console_struct*)retval->function_status)->keybuff[1] = -1;  
  if(!tty)
    ((struct console_struct*)retval->function_status)->con_lines = retval->window_grid[0]->items;
  else
    ((struct console_struct*)retval->function_status)->con_lines = ttys[tty-1].vbuff;

  running_console = retval;
  console_rc = new_console(retval->function_status, filename);
  running_console = old_running_console;
  
  switch(console_rc)
  {
    case 1:
      delete_console(retval->function_status);
      goto deletewindow;
    case 2:
      show_error_msg("Error while loading application.");
      delete_console(retval->function_status);
      del_window(retval);
      return 0;    
  }

  retval->initialized = 1; 
  return retval;  
}

#include "vpupwin.c"
