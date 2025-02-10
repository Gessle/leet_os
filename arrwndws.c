static void tile_windows(void)
{
  unsigned window_count_l = window_count;
  unsigned window_rows = 0;
  unsigned window_cols;
  unsigned n, m = 0;
  unsigned toobig_stack_x = 0;
  unsigned toobig_stack_y = 0;
  unsigned screenview_top = _FONT_HEIGHT+2;
  unsigned screenview_height = _RES_Y - screenview_top;

  for(n=window_count;n--;)
    if(windows[n]->hidden || windows[n]->function_pointer == &clock_program || windows[n]->fullscreen)
      window_count_l--;

  if(!window_count_l) return;

  window_cols = window_count_l;

  while(++window_rows * 3 < window_cols)
    window_cols >>= 1;
  if(window_count_l % window_cols)
    window_rows++;

  for(n=window_count;n-->m;)
  {
    
    while(windows[n-m]->hidden || windows[n-m]->function_pointer == &clock_program || windows[n-m]->fullscreen)
    {
      if(windows[n-m]->fullscreen)
        minimize_window(windows[n-m]);      
      m++;
      if(n<m) return;
    }
//    if(n<m)
//      break;
      
    windows[n-m]->maximized = 0;
    windows[n-m]->minimized = 0;
    set_window_resized(windows[n-m]);
    if(windows[n-m]->resizable)
    {
      if((windows[n-m]->height = (screenview_height) / window_rows - (_FONT_HEIGHT+2)) < 120)
        windows[n-m]->height = 120;
      if((windows[n-m]->width = _RES_X / window_cols) < 120)
        windows[n-m]->width = 120;
    }
    windows[n-m]->y = screenview_top;
    windows[n-m]->x = (n - (window_count - window_count_l)) * (_RES_X / window_cols);
    while(windows[n-m]->x > _RES_X - windows[n-m]->width)
    {
      if((windows[n-m]->x -= _RES_X) < 0)
        windows[n-m]->x = 0;
      windows[n-m]->y += screenview_height / window_rows;
    }
    if(windows[n-m]->x >= _RES_X || windows[n-m]->y >= _RES_Y)
    {
      windows[n-m]->x = toobig_stack_x += _FONT_HEIGHT+2;
      windows[n-m]->y = toobig_stack_y += _FONT_HEIGHT+2;
    }      
    set_active_window(n-m, 0); 
  }
}

static void cascade_windows(void)
{
  unsigned n = 0;
  int x = 40;
  int y = 0;
  do
  {
    if(windows[n]->hidden || windows[n]->function_pointer == &clock_program || windows[n]->fullscreen)
    {
      if(windows[n]->fullscreen)
        minimize_window(windows[n]);      
      continue;
    }
    windows[n]->minimized = 0;
    windows[n]->maximized = 0;
    windows[n]->x = x += _FONT_HEIGHT+2;
    windows[n]->y = y += _FONT_HEIGHT+2;
    if(x > _RES_X - 200)
      x = 0;
    if(y > _RES_Y - 100)
      y = 0;
    set_active_window(n, 0);      
  }  
  while(++n < window_count);   
}
