static int mouseclick(void);
static unsigned call_mouse_handler(unsigned);

static int user_input(void)
{
  return (diskio_in_progress || kbhit() || mouseclick());
}

unsigned mtask_win(int *c, unsigned a, struct prog_window *prev_run_win)
{
  int n, t;

  if(next_tty < 0)
  {
    if(next_window < 0) next_window = window_count;
    nextwindow:while((n = --next_window) >= 0)
    {
      if(next_tty >= 0) goto nexttty;
      if(windows[n]->resized)
      {
        windows[n]->redraw = (!!windows[n]->on_resize)<<1;
        windows[n]->resized = 0;
        if(windows[n]->on_resize)
          a = (*windows[n]->on_resize)(windows[n]);
        else
          goto window_draw;
        //windows[n]->redraw = (!!windows[n]->on_resize)<<1;
//        windows[n]->resized = 0;
      }
      if(windows[n]->redraw && !diskio_in_progress)
      {
        window_draw:
        if(!screen_redraw)
          draw_window(windows[n], (active_window==n), windows[n]->redraw&3);
//        windows[n]->redraw = 0;
      }
      if(windows[n]->function_pointer && windows[n]->initialized)
      {
        running_window = windows[n];
        if(!diskio_in_progress && n == active_window && !tty)
          a = (*windows[n]->function_pointer)(windows[n], call_mouse_handler(a), c);
        else
          (*windows[n]->function_pointer)(windows[n], 0, 0);
      }
      if(user_input())
        break;
    }
    next_tty = TTY_COUNT;
  }
  nexttty:while((t = --next_tty) >= 0)
  {
    if(next_window >= 0) goto nextwindow;
    if(!ttys[t].initd) continue;
    running_window = &ttys[t].con;
    if(tty-1 == t)
      vpuconsole_program(running_window, 0, c);
    else
      vpuconsole_program(running_window, 0, 0);
    if(user_input()) break;
  }
  running_window = prev_run_win;

  return a;
}
