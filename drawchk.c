// when this is not 0, only pixels inside this window can be drawn.
struct prog_window *draw_window_box = 0;

inline int pixel_is_inside_window(int x, int y, struct prog_window *window)
{
  if(x < window->x-1 || x > window->x+window->width) return 0;
  if(y < window->y-1 || y > window->y+window->height+_FONT_HEIGHT+2) return 0;
  return 1;
}

unsigned char windowpos_visible(struct prog_window *window, int x, int y)
{
  unsigned n;// = window_count;

//  if(!ttys[tty].windowed) return 0;

  if(draw_window_box)
  {
    if(!pixel_is_inside_window(x, y, draw_window_box))
      return 0;
  }

  if(!window) return 1;    

  if(window != (void*)-1)
  {
    if(window->vt) return 0;
    if(windows[active_window] != window && !windows[active_window]->hidden && (windows[active_window]->fullscreen || windows[active_window]->maximized))
      return 0;
    if((window->fullscreen || window->maximized) && windows[active_window] == window)
      return 1;
    else if(window->minimized || window->hidden)
      return 0;
  }
  
//  while(n--)
  for(n=window_count;n--;)
  {
    if(window != (void*)-1 && windows[window_visibility_order[n]] == window) return 1;
    if(!windows[window_visibility_order[n]]->minimized && !windows[window_visibility_order[n]]->hidden
      && pixel_is_inside_window(x, y, windows[window_visibility_order[n]]))
      return 0;
  }
  return 1;
}
