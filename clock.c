/*struct int21time alarm_time;

void (__interrupt __far *prev_int_1c)();

void unset_alarm(void)
{
  alarm_time.s = 0;
  _dos_setvect(0x1c, prev_int_1c);
}

void clock_alarm(void)
{
  make_sound(2);
  unset_alarm();
}

void __interrupt __far alarm_clock_check() 
{ 
  struct int21time *time_now = dostime();

  if(time_now->h == alarm_time.h && time_now->m == alarm_time.m)
    clock_alarm();

  _chain_intr( prev_int_1c ); 
} 

void set_alarm(unsigned char hour, unsigned char min)
{
  alarm_time.h = hour;
  alarm_time.m = min;
  alarm_time.s = 1;
  
  prev_int_1c = _dos_getvect( 0x1c );
  _dos_setvect(0x1c, alarm_clock_check);
}

int alarmclock_program(struct prog_window *window, int args, int *keypress)
{

  if(!args || args==1)
    edit_text(window, args);

  if(args == 2)
  {
    set_alarm(atoi(window->window_grid[0]->text), atoi(window->window_grid[1]->text));
    del_window(window);
    return -101;
  }

  return args+100;
}

struct prog_window *init_alarmclock_window(void)
{
  struct prog_window *retval = new_window(-1, -1, 250, 100, "Set alarm", 0, 1, 1, "ICONS/CLOCK.ICO", &alarmclock_program);
  if(!retval)
  {
    nomemory:
    if(retval) del_window(retval);
    nomemory(0);
    return 0;    
  }

  if(!(window_textbox(retval, 50, 3, 30, _FONT_HEIGHT+2, 0, 2)))
    goto nomemory;
  if(!(window_textbox(retval, 50, 23, 30, _FONT_HEIGHT+2, 0, 2)))
    goto nomemory;
  
  if(!(window_text(retval, 10, 3, "\021Hour:", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;
  if(!(window_text(retval, 10, 23, "\021Minute:", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;

  if(!(window_button(retval, 10, 43, 60, _FONT_HEIGHT+2, "Set alarm", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;    

  draw_window(retval, 1, 1);      

  return retval;
}
*/
static unsigned clock_program(struct prog_window *window, unsigned args, int *keypress)
{
//  static char secs;
  int c;
  if(keypress)
  {
    if(!*keypress)
    {
      c = getch();
      desktop_iconmov(c);
      desktop_iconnav(c);
//      draw_screen();
      *keypress = -1;
    }
    else if(*keypress == 13)
      return WM_ICONSELECT;
//    else if(*keypress == 97 || *keypress == 65)
//      init_alarmclock_window();
  }
  

  if(_VIDEO_MODE == video && *(int*)window->function_status != dostime()->s)
  {
    running_window = 0;
    *(int*)window->function_status = dostime()->s;
    window->window_grid[0]->text = human_date();
    draw_window(window, 1, 3);
    window->window_grid[0]->text = 0;
  }

  return ~WM_WINPROGACT;

//  window_drawelement(window->window_grid[0], window->x, window->y);
}


static struct prog_window *init_clock_window(void)
{
  struct prog_window *retval = new_window(_RES_X-150, _RES_Y-(2*(_FONT_HEIGHT+2)), 150, _FONT_HEIGHT+2, "Clock", 0, 1, "ICONS/CLOCK.ICO", &clock_program);

  struct window_element *element = window_text(retval, 5, 1, 0, _WINDOW_BASICTEXTCOLOR);
  retval->function_status = malloc(sizeof(int));
  retval->menus.menubar = desktop_menu;
  retval->initialized = 1;
  return retval;
}
