static void desktop_iconmov(int c)
{
  switch (c)
  {
    case 152:
      move_icon(icon_selected, 0, -10);
    goto save_position;
    case 160:
      move_icon(icon_selected, 0, 10);
    goto save_position;
    case 155:
      move_icon(icon_selected, -10, 0);
    goto save_position;    case 157:
      move_icon(icon_selected, 10, 0);
    goto save_position;
  }
  return;
  save_position:
  if(icon_selected >= desktop_icon_count)
    save_desktop_shortcut(icon_selected-desktop_icon_count);  
}

static void desktop_iconnav(int c)
{
  unsigned int n = desktop_icon_count+shortcut_count;
  int *x;
  int *y;
  unsigned old_select = icon_selected;

//  int x = desktop_icons[icon_selected].x;
//  int y = desktop_icons[icon_selected].y;
  int cx, cy;
  int ns = -1;

  struct coords *icon_coords = malloc(n*sizeof(struct coords));

  if(!icon_coords)
    return;
  
  for(;n--;)
  {
    if(n>=desktop_icon_count)
    {
      icon_coords[n].x = desktop_shortcuts[n-desktop_icon_count].x;
      icon_coords[n].y = desktop_shortcuts[n-desktop_icon_count].y;      
    }
    else
    {
      icon_coords[n].x = desktop_icons[n].x;
      icon_coords[n].y = desktop_icons[n].y;      
    }
  }
  x = &icon_coords[icon_selected].x;
  y = &icon_coords[icon_selected].y;
  
  n = desktop_icon_count+shortcut_count;

  switch(c)
  {
    case 77:
      for(cx=_RES_X, cy=_RES_Y;n--;)
        if(icon_coords[n].x > *x && icon_coords[n].x <= cx)
          if(abs(icon_coords[n].y-*y) <= cy)
          {
            cx = icon_coords[n].x;
            cy = abs(*y-icon_coords[n].y);
            ns=n;            
          }
    break;
    case 75:
      for(cx=0, cy=_RES_Y;n--;)
        if(icon_coords[n].x < *x && icon_coords[n].x >= cx)
          if(abs(icon_coords[n].y-*y) <= cy)
          {
            cx = icon_coords[n].x;
            cy = abs(*y-icon_coords[n].y);
            ns=n;            
          }
    break;
    case 80:
      for(cx=_RES_X, cy=_RES_Y;n--;)
        if(icon_coords[n].y > *y && icon_coords[n].y <= cy)
          if(abs(icon_coords[n].x-*x) <= cx)
          {
            cx = abs(*x-icon_coords[n].x);
            cy = icon_coords[n].y;
            ns=n;            
          }
    break;
    case 72:
      for(cx=_RES_X, cy=0;n--;)
        if(icon_coords[n].y < *y && icon_coords[n].y >= cy)
          if(abs(icon_coords[n].x-*x) <= cx)
          {
            cx = abs(*x-icon_coords[n].x);
            cy = icon_coords[n].y;
            ns=n;            
          }
    break;    
  }
  if(ns>=0)
    icon_selected = ns;

  if(icon_selected != old_select)
    draw_select_icon(old_select, icon_selected);

  free(icon_coords);  
}

/*
Resizes a window using keyboard
*/
static void window_kbdresize(struct prog_window *window)
{
  int c;
  reset_vpu_interrupt_handlers();  
  window->hidden=1;
  draw_window_box = window;
  draw_screen();
  draw_window_box = 0;
  draw_move_borders(window->x, window->y, window->width, window->height+2+_FONT_HEIGHT, 0);
  while((c = getch()) != 27 && c != 13)
  {
    call_drv_screenupdate();
    if(!c)
    {
      switch((c=getch()))
      {
        case 75:
          if(titlebar_drag)
            titlebar_drag->x1--;
          window->width--;
        break;
        case 77:
          if(titlebar_drag)
            titlebar_drag->x1++;        
          window->width++;
        break;
        case 72:
          window->height--;
        break;
        case 80:
          window->height++;         
        break;
      }
    }
  draw_move_borders(window->x, window->y, window->width, window->height+2+_FONT_HEIGHT, 3);      
  }
  set_window_resized(window);
  draw_move_borders(window->x, window->y, window->width, window->height+2+_FONT_HEIGHT, 2);
  window->hidden=0;

  draw_window(window, 1, 0);
  set_vpu_interrupt_handlers();  
}

/*
Moves a window using keyboard
*/
static void window_kbdmove(struct prog_window *window)
{
  int c;
  int dx;
  int dy;
  reset_vpu_interrupt_handlers();  
  window->hidden=1;
  draw_window_box = window;
  draw_screen();
  draw_window_box = 0;
  draw_move_borders(window->x, window->y, window->width, window->height+2+_FONT_HEIGHT, 0);  
  while((c = getch()) != 27 && c != 13)
  {
    call_drv_screenupdate();
    dx=0;
    dy=0;
    if(!c)
    {
      switch(getch())
      {
        case 75:
          dx=-1;
        break;
        case 77:
          dx=1;
        break;
        case 72:
          dy=-1;
        break;
        case 80:
          dy=1;         
        break;
      }
    }
    window->x+=dx;
    window->y+=dy;
    if(titlebar_drag)
    {
      titlebar_drag->x0+=dx;
      titlebar_drag->x1+=dx;
      titlebar_drag->y0+=dy;
      titlebar_drag->y1+=dy;      
    }
    draw_move_borders(window->x, window->y, window->width, window->height+2+_FONT_HEIGHT, 1);
  }
  draw_move_borders(window->x, window->y, window->width, window->height+2+_FONT_HEIGHT, 2);
  window->hidden=0;
  draw_window(window, 1, 0);
  set_vpu_interrupt_handlers();  
}
