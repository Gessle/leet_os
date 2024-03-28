char *mouse_cursor = 0;
char *normal_cursor = 0;
char *busy_cursor = 0;

#define MOUSE_CURSOR_HEIGHT 12
#define MOUSE_CURSOR_WIDTH sizeof(char)*8

/*
Loads the mouse cursor bitmap
*/
static void load_mouse_cursor(char *filename, char **cursor)
{
  FILE *fp;
  *cursor = malloc(MOUSE_CURSOR_HEIGHT*sizeof(char));

  fp = fopen(filename, "rb");
  fread(*cursor, sizeof(char), MOUSE_CURSOR_HEIGHT, fp);
  fclose(fp);  
}

/*
Detects the mouse
Returs: count of buttons if mouse detected, else returns 0
*/
static int detect_mouse(void)
{
  int mouse;
  int buttons;
  if(_dos_getvect(0x33))
  {
    __asm
    {
      mov ax, 0x00
      int 0x33
      mov mouse, ax
      mov buttons, bx
    }
    if(mouse)
    {
      titlebar_drag = malloc(sizeof(struct screen_region));
      return buttons;
    }
  }
  return 0;
}

#define POINTER_START_X 160
#define POINTER_START_Y 100

/* Initializes the mouse and returns a pointer to the mouse status struct
Returns 0 if no mouse detected */
static struct mouse_status *init_mouse(void)
{
  int buttons;
  struct mouse_status *retval = 0;

  if((buttons = detect_mouse()))
  {
    retval = calloc(1, sizeof(struct mouse_status));
    retval->pointer_x=POINTER_START_X;
    retval->pointer_y=POINTER_START_Y;
    retval->hidden = 1;
    load_mouse_cursor("normal.cur", &normal_cursor);
    load_mouse_cursor("hourglas.cur", &busy_cursor);
    mouse_cursor = normal_cursor;
  }

  return retval;
}

struct mouse_movement
{
  int dx;
  int dy;
};

/*
Detects mouse movement
Returns: 0 if no mouse movement
Else If mouse movement detected, returns a pointer to a signed int array
where the offset 0 is the movement in x axis and offset 1 is the movement in
y axis
*/
struct mouse_movement *get_mouse_movement(void)
{
  static struct mouse_movement retval;
  int d_x;
  int d_y;
  unsigned char bl_reg;

  __asm
  {
    mov ax, 0x0B
    int 0x33
    mov d_x, cx
    mov d_y, dx
    mov ax, 0x03
    int 0x33
    mov bl_reg, bl
  }

  if((bl_reg & 0x01))
    mouse->left_pressed=1;
  else
    mouse->left_pressed=0;
  if((bl_reg & 0x02))
    mouse->right_pressed=1;
  else
    mouse->right_pressed=0;


  if(d_x || d_y)
  {
    hide_mouse_cursor();

    mouse->pointer_x+=d_x;
    mouse->pointer_y+=d_y;

    if(mouse->pointer_x < 0)
    {
      d_x -= mouse->pointer_x;
      mouse->pointer_x = 0;
    }
    if(mouse->pointer_y < 0)
    {
      d_y -= mouse->pointer_y;    
      mouse->pointer_y = 0;
    }
    if(mouse->pointer_x >= _RES_X)
    {
      d_x -= mouse->pointer_x-_RES_X;    
      mouse->pointer_x = _RES_X-1;
    }
    if(mouse->pointer_y >= _RES_Y)
    {
      d_y -= mouse->pointer_y-_RES_Y;    
      mouse->pointer_y = _RES_Y-1;
    }

    retval.dx = d_x;
    retval.dy = d_y;
    return &retval;
  }
  return 0;
}

unsigned char mouse_background[MOUSE_CURSOR_WIDTH][MOUSE_CURSOR_HEIGHT];

static void draw_mouse_cursor(void)
{
  static unsigned int old_x = POINTER_START_X;
  static unsigned int old_y = POINTER_START_Y;
  unsigned char x, y;

  if(mouse && mouse->hidden)
  {
//    if(!mouse->hidden) hide_mouse_cursor();
    for(y=MOUSE_CURSOR_HEIGHT;y--;)
      for(x=MOUSE_CURSOR_WIDTH;x--;)
        if((mouse_cursor[y] << x) & 0x80)
        {
          mouse_background[x][y] = get_pixel(mouse->pointer_x+x, mouse->pointer_y+y);
          if(mouse_background[x][y] == _CURSOR_COLOR)
            put_pixel_raw(mouse->pointer_x+x, mouse->pointer_y+y, _CURSOR_COLOR | 0x08);
          else
            put_pixel_raw(mouse->pointer_x+x, mouse->pointer_y+y, _CURSOR_COLOR);
        }
    mouse->hidden = 0;
  }
  call_drv_screenupdate();
}

void hide_mouse_cursor(void)
{
  unsigned char x, y;

  if(mouse && !mouse->hidden)
  {
    mouse->hidden = 1;
    for(y=MOUSE_CURSOR_HEIGHT;y--;)
      for(x=MOUSE_CURSOR_WIDTH;x--;)
        if((mouse_cursor[y] << x) & 0x80)
          put_pixel_raw(mouse->pointer_x+x, mouse->pointer_y+y, mouse_background[x][y]);
  }
}

/*static*/ void redraw_mouse_cursor(void)
{
  if(mouse)
  {
    get_mouse_movement();
    draw_mouse_cursor();
  }
}

/*
Draws mouse cursor
Argument 1: pointer to the mouse status struct, 2: pointer to the video memory or buffer,
3: unsigned int updatebg

if updatebg == 0, draws mouse cursor normally
if updatebg == 2, hides mouse cursor
if updatebg == 1, re-draws mouse cursor
*/
/*void draw_mouse_cursor(unsigned updatebg)
{
  static unsigned char background[12][8];
  static unsigned int old_x = 160;
  static unsigned int old_y = 100;
  unsigned int n, m;

  if(!mouse || tty) return;

  running_window = 0;

  if(!updatebg || updatebg == 2)
    for(n=12;n--;)
      for(m=8;m--;)
        put_pixel(old_x+m, old_y+n, background[n][m]);
  if(!updatebg || updatebg == 1)
    for(n=12;n--;)
      for(m=8;m--;)      
        background[n][m] = get_pixel(mouse->pointer_x+m, mouse->pointer_y+n);

  old_x = mouse->pointer_x;
  old_y = mouse->pointer_y;

  if(!updatebg)
  {  
    for(n=12;n--;)
      for(m=8;m--;)
        if((mouse_cursor[n]<<m)&0x80)
          if(background[n][m])
            put_pixel(mouse->pointer_x+m, mouse->pointer_y+n, _CURSOR_COLOR);
          else
            put_pixel(mouse->pointer_x+m, mouse->pointer_y+n, _CURSOR_COLOR | 0x7);
  }
}*/

static void wait_leftbutton_release(void)
{
//  draw_mouse_cursor();
  while(mouse->left_pressed)
  {
    redraw_mouse_cursor();
    halt();
  }
//  hide_mouse_cursor();
}

static int mouse_left_pressed(void)
{
  call_drv_screenupdate();
  return mouse->left_pressed;
}

/*
Starts drawing the border when moving or resizing a window
*/
static void dwwindow_initborder(struct prog_window *window)
{
  while(mouse_left_pressed())
  {
    if(get_mouse_movement())
    {  
      window->hidden=1;
      draw_window_box = window;
      draw_screen();
      draw_window_box = 0;
      draw_move_borders(window->x, window->y, window->width, window->height+2+_FONT_HEIGHT, 0);
      break;
    }
  }
}
/*
Ends drawing the border and frees all mallocs
*/
static void dwwindow_endborder(struct prog_window *window)
{
  draw_move_borders(window->x, window->y, window->width, window->height+2+_FONT_HEIGHT, 2);
  window->hidden=0; 
}

/*
Moves the window by mouse
Returns false if window wasn't moved, otherwise returns -1
*/
static int drag_window(struct prog_window *window)
{
  struct mouse_movement *d;
  int retval = 0;
  reset_vpu_interrupt_handlers();  

  dwwindow_initborder(window);
  while(mouse_left_pressed())
  {
    if((d=get_mouse_movement()))
    {
      window->x += d->dx;
      window->y += d->dy;
      titlebar_drag->x0 +=d->dx;
      titlebar_drag->y0 +=d->dy;
      titlebar_drag->x1 +=d->dx;
      titlebar_drag->y1 +=d->dy;
      draw_move_borders(window->x, window->y, window->width, window->height+2+_FONT_HEIGHT, 1);
      retval = -1;
    }
  }
  dwwindow_endborder(window);
  set_vpu_interrupt_handlers();
  return retval;
}

/*
Resizes the window by mouse
*/
static void mouse_resize_window(struct prog_window *window)
{
  struct mouse_movement *d;
  reset_vpu_interrupt_handlers();  
  dwwindow_initborder(window);
  while(mouse_left_pressed())
  {
    if((d=get_mouse_movement()))
    {
      if((window->width += d->dx) < 40) window->width=40;
      if((window->height += d->dy) < 40) window->height=40;
      if((titlebar_drag->x1 += d->dx) < 40) titlebar_drag->x1 = 40;
      draw_move_borders(window->x, window->y, window->width, window->height+2+_FONT_HEIGHT, 3);      
    }
  }
  dwwindow_endborder(window);
  set_vpu_interrupt_handlers();  
}

/*
Moves a desktop icon or shortcut by mouse
*/
static void mouse_drag_icon(unsigned icon)
{
  struct mouse_movement *d;
  reset_vpu_interrupt_handlers();                              
  while(mouse_left_pressed())
  {
    if((d=get_mouse_movement()))
    {
      move_icon(icon, d->dx, d->dy);
    }
  }
//  draw_mouse_cursor(1);      
  set_vpu_interrupt_handlers();
}

/*void mouse_drag_shortcut(struct desktop_shortcut *icon)
{
  int *d;
  while(mouse->left_pressed)
  {
    if((d=get_mouse_movement()))
    {
      icon->x +=d[0];
      icon->y +=d[1];
      draw_desktop();
    }
  }
  draw_screen();
}*/

/*
Handler for the left mouse button (desktop)
*/
static int mleft_handler_desktop(void)
{
  unsigned int n = desktop_icon_count;
  unsigned int m = 0;
  unsigned int l = 0;
  struct mouse_movement *d = 0;
  unsigned int icon_selected_prev = icon_selected;


  // menubar
  if(mouse->pointer_y < _FONT_HEIGHT+2)
  {
    menu_selected = mouse->pointer_x/50+1;
    return 0;
  }
  // if mouse pointer is over a desktop icon
  while(n--)
  {
    if(mouse->pointer_x > desktop_icons[n].x && mouse->pointer_x < desktop_icons[n].x+40
      && mouse->pointer_y > desktop_icons[n].y && mouse->pointer_y < desktop_icons[n].y+35)
    {
      while(!(d=get_mouse_movement()) && mouse->left_pressed);
      if(!d)
      {
        if(icon_selected_prev != n)
        {
          icon_selected = n;
          draw_select_icon(icon_selected_prev, icon_selected);
        }
        else
          return WM_ICONSELECT;
      }
      // break while if mouse movement detected
      else break;
    }
  }

  if(!d)
  {
    // if mouse pointer is over a shortcut icon
    for(m=shortcut_count;m--;)
    {
      if(mouse->pointer_x > desktop_shortcuts[m].x && mouse->pointer_x < desktop_shortcuts[m].x+40
      && mouse->pointer_y > desktop_shortcuts[m].y && mouse->pointer_y < desktop_shortcuts[m].y+35)
      {
        while(!(d=get_mouse_movement()) && mouse->left_pressed);
        if(!d)
        {
          if(icon_selected_prev != desktop_icon_count+m)
          {
            icon_selected = desktop_icon_count+m;
            draw_select_icon(icon_selected_prev, icon_selected);
          }
          else
            return WM_ICONSELECT;
          return 0;
        }
        else break;
      }
    }
    // if mouse pointer is over a minimized window icon (if d!=0 then jump to else)
    if(!d)
    {
      for(m=0;l<window_count;l++)
      {
        if(windows[l]->minimized)
        {
          if(mouse->pointer_x >= _WINDOW_ICONS_LEFT+m*_WINDOW_ICONS_MARGIN && mouse->pointer_x < _WINDOW_ICONS_LEFT+m*_WINDOW_ICONS_MARGIN+_WINDOW_ICONS_MARGIN-2
            && mouse->pointer_y > _RES_Y-40)
          {
            // draw inset borders while mouse leftbutton down
            draw_borders(_WINDOW_ICONS_LEFT+m*_WINDOW_ICONS_MARGIN, _RES_Y-40,
                _WINDOW_ICONS_MARGIN-2, 39, 1);
            wait_leftbutton_release();            
            unminimize_window(l);
            return 0;
/*            else
            {
              draw_borders(_WINDOW_ICONS_LEFT+m*_WINDOW_ICONS_MARGIN, _RES_Y-40,
                _WINDOW_ICONS_MARGIN-2, 39, 0);
              return 0;
            }*/
          }
          m++;
        }
      }
    }
    // mouse motion detected while left button down && mouse over a shortcut
    else
    {
      mouse_drag_icon(desktop_icon_count+m);
      save_desktop_shortcut(m);      
      return 0;
    }
  }
  // if d != 0 (mouse motion detected while left button down && mouse over a desktop icon)
  else
  {
    mouse_drag_icon(n);
  }
  return 0;
}

/*
Handler for scrollbars
*/
inline void mouse_scrollbar_handler(struct prog_window *window, unsigned element)
{
  struct mouse_movement *d;
  struct window_element *scrollbar = window->window_grid[element];
  unsigned addh = _FONT_HEIGHT+2;

  if(!scrollbar->ctype)
  {
    // vertical scrollbar
    // upper arrow button pressed
    if(mouse->pointer_y < window->y+addh+scrollbar->y+13)
    {
      draw_borders(window->x+scrollbar->x, window->y+addh+scrollbar->y,
          12, 12, 1);
        // scroll up...
      if(scrollbar->scroll_x) scrollbar->scroll_x--;
      wait_leftbutton_release();
      draw_borders(window->x+scrollbar->x, window->y+addh+scrollbar->y,
          12, 12, 0);      
    }
      // if mouse pointer is between the upper arrow button and the anchor
    else if(scrollbar->c && mouse->pointer_y < window->y+addh+scrollbar->y+13
    + (unsigned long)scrollbar->scroll_x * (scrollbar->width-12) / scrollbar->c)
    {
      wait_leftbutton_release();      
      // scroll up amount of scrollbar->width...
      if(scrollbar->scroll_x >= scrollbar->width/(_FONT_HEIGHT+2))
        scrollbar->scroll_x -= scrollbar->width/(_FONT_HEIGHT+2);
      else
        scrollbar->scroll_x = 0;
    }
        // if mouse pointer is over the anchor        
    else if(scrollbar->c && mouse->pointer_y < window->y+addh+scrollbar->y+25
     + (unsigned long)scrollbar->scroll_x * (scrollbar->width-12) / scrollbar->c)
    {
      while(mouse_left_pressed()) 
      {
        if((d = get_mouse_movement()))
        {
//          draw_mouse_cursor(0);
          // scroll by d->dy...
          if((int)scrollbar->scroll_x+d->dy >= 0 && scrollbar->scroll_x+d->dy <= scrollbar->c)
              scrollbar->scroll_x+=d->dy;
          if(scrollbar->connect_value)
            *(int*)scrollbar->connect_value = scrollbar->scroll_x;
          if(scrollbar->connect_element != -1)
            window_drawelement(window, scrollbar->connect_element);          
          window_drawelement(window, element);
        }
      }
    }
    // if mouse pointer is between the anchor and the down arrow button
    else if(mouse->pointer_y < window->y+addh+scrollbar->y+13+scrollbar->width)
    {
      wait_leftbutton_release();      
      // scroll down amount of scrollbar->width...
      if(scrollbar->scroll_x+scrollbar->width/(_FONT_HEIGHT+2) < scrollbar->c)
        scrollbar->scroll_x += scrollbar->width/(_FONT_HEIGHT+2);
      else
        scrollbar->scroll_x = scrollbar->c;        
    }
    // down arrow button pressed
    else
    {
      draw_borders(window->x+scrollbar->x, window->y+addh+scrollbar->y+14+scrollbar->width,
          12, 12, 1);
        // scroll down...
      if(scrollbar->c && scrollbar->scroll_x < scrollbar->c)
        scrollbar->scroll_x++;
      wait_leftbutton_release();        
      draw_borders(window->x+scrollbar->x, window->y+addh+scrollbar->y+14+scrollbar->width,
          12, 12, 0);     
    }
    if(scrollbar->connect_value)
      *(int*)scrollbar->connect_value = scrollbar->scroll_x;
    if(scrollbar->connect_element != -1)
      window_drawelement(window, scrollbar->connect_element);                        
  }
  // horizontal scrollbar
  else
  {
    // left arrow button pressed
    if(mouse->pointer_x < window->x+scrollbar->x+12)
    {
      draw_borders(window->x+scrollbar->x, window->y+scrollbar->y+addh,
          12, 12, 1);
        // scroll left...
      if(scrollbar->scroll_x) scrollbar->scroll_x--;
      wait_leftbutton_release();
      draw_borders(window->x+scrollbar->x, window->y+scrollbar->y+addh,
          12, 12, 0);      
    }
      // if mouse pointer is between the left arrow button and the anchor
    else if(scrollbar->c && mouse->pointer_x < window->x+scrollbar->x+13
    + (unsigned long)scrollbar->scroll_x * (scrollbar->width-12) / scrollbar->c)
    {
      wait_leftbutton_release();      
      // scroll left amount of scrollbar->width...
      if(scrollbar->scroll_x >= scrollbar->width/(_FONT_HEIGHT+2))
        scrollbar->scroll_x -= scrollbar->width/(_FONT_HEIGHT+2);
      else
        scrollbar->scroll_x = 0;
    }
        // if mouse pointer is over the anchor        
    else if(scrollbar->c && mouse->pointer_x < window->x+scrollbar->x+25
     + (unsigned long)scrollbar->scroll_x * (scrollbar->width-12) / scrollbar->c)
    {
      while(mouse_left_pressed()) 
      {
        if((d = get_mouse_movement()))
        {
//          draw_mouse_cursor(0);
          // scroll by d->dx...
          if((int)scrollbar->scroll_x+d->dx >= 0 && scrollbar->scroll_x+d->dx <= scrollbar->c)
              scrollbar->scroll_x+=d->dx;
          if(scrollbar->connect_value)
            *(int*)scrollbar->connect_value = scrollbar->scroll_x;
          if(scrollbar->connect_element != -1)
            window_drawelement(window, scrollbar->connect_element);                    
          window_drawelement(window, element);
        }
      }
    }
    // if mouse pointer is between the anchor and the down arrow button
    else if(mouse->pointer_x < window->x+scrollbar->x+scrollbar->width+13)
    {
      wait_leftbutton_release();      
      // scroll right by amount of scrollbar->width...
      if(scrollbar->scroll_x+scrollbar->width/(_FONT_HEIGHT+2) < scrollbar->c)
        scrollbar->scroll_x += scrollbar->width/(_FONT_HEIGHT+2);
      else
        scrollbar->scroll_x = scrollbar->c;        
    }
    // right arrow button pressed
    else
    {
      draw_borders(window->x+scrollbar->x+scrollbar->width+14, window->y+scrollbar->y+addh,
          12, 12, 1);
        // scroll right...
      if(scrollbar->c && scrollbar->scroll_x < scrollbar->c)
        scrollbar->scroll_x++;
      wait_leftbutton_release();        
      draw_borders(window->x+scrollbar->x+scrollbar->width+14, window->y+scrollbar->y+addh,
          12, 12, 0);     
    }
    if(scrollbar->connect_value)
      *(int*)scrollbar->connect_value = scrollbar->scroll_x;
    if(scrollbar->connect_element != -1)
      window_drawelement(window, scrollbar->connect_element);              
  }
  window_drawelement(window, element);
}

/*
Handler for window_list-elements
*/
inline void mouse_list_handler(struct prog_window *window, unsigned element)
{
//  draw_mouse_cursor(2);
  if(window->window_grid[element]->active)
  {
    if(window->window_grid[element]->selectc != window->window_grid[element]->scroll_y
      + (mouse->pointer_y-(window->y+2+_FONT_HEIGHT+window->window_grid[element]->y)) / (2+_FONT_HEIGHT))
    {
      window_list_selectrow(window, element, window->window_grid[element]->scroll_y
      + (mouse->pointer_y-(window->y+2+_FONT_HEIGHT+window->window_grid[element]->y)) / (2+_FONT_HEIGHT));
  
      while(mouse_left_pressed()) 
        get_mouse_movement();
    }
    else
    {
      window->window_grid[element]->connect_element = window->window_grid[element]->scroll_y
      + (mouse->pointer_y-(window->y+2+_FONT_HEIGHT+window->window_grid[element]->y)) / (2+_FONT_HEIGHT);
      while(mouse_left_pressed()) 
        get_mouse_movement();    
    }
  }
//  draw_mouse_cursor(1);    
}

inline void mouse_checkbox_handler(struct prog_window *window, unsigned element)
{
  window->window_grid[element]->selectc = ~window->window_grid[element]->selectc;
  wait_leftbutton_release();
}

inline void mouse_radiobutton_handler(struct prog_window *window, unsigned element)
{
  window_radiobutton_select(window, window->window_grid[element]);
  wait_leftbutton_release();
}

inline void mouse_textbox_handler(struct prog_window *window, unsigned element)
{
  element_active(window, element);
  wait_leftbutton_release();  
}

/*
Handler for left mouse button
*/
static int mleft_handler(struct prog_window *window)
{
  unsigned int n;
//  draw_mouse_cursor(2);      
  if(window)
  {
    if(window->resizable)
    {
      // if mouse pointer is over the windows's rightdown corner
      if(mouse->pointer_x >= window->x+window->width-1 && mouse->pointer_x <= window->x+window->width+1
      && mouse->pointer_y >= window->y+window->height+_FONT_HEIGHT+1 && mouse->pointer_y <= window->y+window->height+_FONT_HEIGHT+3)
        return -8;
    // if mouse pointer is over the maximize/unmaximize button                
      if(mouse->pointer_x > window->x+window->width-_FONT_HEIGHT-2 && mouse->pointer_x < window->x+window->width-1
        && mouse->pointer_y > window->y && mouse->pointer_y < window->y+_FONT_HEIGHT+1)
        return 3;
    }
    // if mouse pointer is over the minimize/unminimize button
    if(window->minimizable)
      if(mouse->pointer_x >= window->x+window->width-2*(_FONT_HEIGHT+1) && mouse->pointer_x < window->x+window->width-_FONT_HEIGHT-2
        && mouse->pointer_y > window->y && mouse->pointer_y < window->y+_FONT_HEIGHT+1)
        return WM_MINIMIZEWINDOW;
    // if mouse pointer is over the top left dropdown menu button
    if(mouse->pointer_x > window->x && mouse->pointer_x < window->x+_FONT_HEIGHT+1
      && mouse->pointer_y > window->y && mouse->pointer_y < window->y+_FONT_HEIGHT+1)
      return -2;        
    // if mouse pointer is over the titlebar
    if(!window->maximized)
      if(mouse->pointer_x >= titlebar_drag->x0 && mouse->pointer_y >= titlebar_drag->y0
        && mouse->pointer_x < titlebar_drag->x1 && mouse->pointer_y < titlebar_drag->y1)
        return -1;
    // if mouse pointer is over a hitbox
    for(n=window->hitbox_count;n--;)
    {
      if(mouse->pointer_x >= window->x + window->hitbox[n].x0 && mouse->pointer_x < window->x + window->hitbox[n].x1
      && mouse->pointer_y >= window->y + window->hitbox[n].y0 && mouse->pointer_y < window->y + window->hitbox[n].y1)
      {
        switch(window->window_grid[window->hitbox[n].element]->type)
        {
          case 8:
            window->window_grid[window->hitbox[n].element]->selectc = window->window_grid[window->hitbox[n].element]->scroll_y;
          // textbox
          case 2:
            mouse_textbox_handler(window, window->hitbox[n].element);
            return 0;
          break;
          // if list
          case 3:
            mouse_list_handler(window, window->hitbox[n].element);
          break;
          // if scrollbar
          case 4:
            mouse_scrollbar_handler(window, window->hitbox[n].element);
            updatemouseel:
//            draw_mouse_cursor(2);
            window_drawelement(window, window->hitbox[n].element);
//            draw_mouse_cursor(1);            
          break;
          // checkbox
          case 6:
            mouse_checkbox_handler(window, window->hitbox[n].element);
            goto updatemouseel;
          // radiobutton
          case 7:
            mouse_radiobutton_handler(window, window->hitbox[n].element);
            goto updatemouseel;
          // button, switch to inset border
          case 1: case 5:
            draw_borders(window->x+window->hitbox[n].x0, window->y+window->hitbox[n].y0,
                  window->hitbox[n].x1-window->hitbox[n].x0, window->hitbox[n].y1-window->hitbox[n].y0, 1);                        
            wait_leftbutton_release();
            draw_borders(window->x+window->hitbox[n].x0, window->y+window->hitbox[n].y0,
              window->hitbox[n].x1-window->hitbox[n].x0, window->hitbox[n].y1-window->hitbox[n].y0, 0);
          break;
        }
        if(mouse->pointer_x >= window->x + window->hitbox[n].x0 && mouse->pointer_x < window->x + window->hitbox[n].x1
        && mouse->pointer_y >= window->y + window->hitbox[n].y0 && mouse->pointer_y < window->y + window->hitbox[n].y1)
        {
          return n | WM_WINPROGACT;
        }
      }
    }
    // if mouse pointer is outside the window
    if(window->type && !(mouse->pointer_x > window->x && mouse->pointer_x < window->x+window->width
      && mouse->pointer_y > window->y && mouse->pointer_y < window->y+window->height+_FONT_HEIGHT+2))
    {
      // if mouse pointer is over another window
      for(n=window_count;n--;)
      {
        if(mouse->pointer_x > windows[window_visibility_order[n]]->x && mouse->pointer_x < windows[window_visibility_order[n]]->x+windows[window_visibility_order[n]]->width
          && mouse->pointer_y > windows[window_visibility_order[n]]->y && mouse->pointer_y < windows[window_visibility_order[n]]->y+windows[window_visibility_order[n]]->height+_FONT_HEIGHT+2)
        {
          if(!windows[window_visibility_order[n]]->minimized && !windows[window_visibility_order[n]]->hidden
             && !(windows[window_visibility_order[n]]->maximized && mouse->pointer_y < _FONT_HEIGHT+3))
          {
//            draw_mouse_cursor(2);            
            set_active_window(window_visibility_order[n], 1);
//            draw_mouse_cursor(1);            
            return ~WM_WINPROGACT;
          }
        }
      }
      return mleft_handler_desktop();
    }
    // if mouse pointer is inside the window, the window is created by a vpu program and the mouse signal is set
    else if(window->function_pointer == vpuwindow_program && ((struct vpu_progwindow*)window->function_status)->mouse_int)
    {
      if(!send_vpu_signal(((struct vpu_progwindow*)window->function_status)->vcpu, ((struct vpu_progwindow*)window->function_status)->mouse_int))
      {
        ((struct vpu_progwindow*)window->function_status)->
          vcpu->ports[((struct vpu_progwindow*)window->function_status)->mouse_port] = mouse->pointer_x - window->x;
        ((struct vpu_progwindow*)window->function_status)->
          vcpu->ports[((struct vpu_progwindow*)window->function_status)->mouse_port + 1] = mouse->pointer_y - window->y - _FONT_HEIGHT - 2;
        set_vpu_turn(((struct vpu_progwindow*)window->function_status)->vcpu);            
      }
    }
  }
  else
    return mleft_handler_desktop();  
  return ~WM_WINPROGACT;
}

/*
Handler for the mouse...
*/
static int mouse_handler(struct prog_window *window)
{
  int a;
  if(get_mouse_movement())
  {
    screensaver_timer = 0;
  }
  // if a window is active and the left button is pressed      
  if(menu_selected < 0 && mouse->left_pressed)
  {
    screensaver_timer = 0;
    if(!(a = mleft_handler(window))) return ~WM_WINPROGACT;
    // a==1 -> mouse over titlebar
    else if(a == -1)
    {
      if(drag_window(window))
        draw_window(window, 1, 0);
      return ~WM_WINPROGACT;
    }
    else if(a == -2)
    {
      // if topleft dropdown menu is used, return selection id (indexing starts from 1)
       menu_selected = 0;
       return ~WM_WINPROGACT;
    }
    else if(a == -8)
    {
      mouse_resize_window(window);
      set_window_resized(window);
      return ~WM_WINPROGACT;
    }
    else if(a == 3 || a == WM_MINIMIZEWINDOW)
    {
      if(a == 3)
        draw_borders(window->x+window->width-(_FONT_HEIGHT), window->y+2, _FONT_HEIGHT-2, _FONT_HEIGHT-2, 1);
      else
        draw_borders(window->x+window->width-2*(_FONT_HEIGHT+1)+1, window->y+2, _FONT_HEIGHT-2, _FONT_HEIGHT-2, 1);      
      wait_leftbutton_release();
    }
    return a;
  } 
  return ~WM_WINPROGACT;
}
/*
Mouse handler for drop-down menus

Returns: if a menu item is selected, returns the selection id
Else returns -1 if nothing has happened, or -2 if the user clicked somewhere
outside the menu
*/
static int menu_mouse_handler(struct prog_window *window, int x, int y, unsigned int width, unsigned int itemcount, unsigned int menuid)
{
  unsigned int selected;
      
  redraw_mouse_cursor();

  if(mouse->left_pressed)
  {
    if(mouse->pointer_x > x && mouse->pointer_x < x+width
      && mouse->pointer_y > y && mouse->pointer_y < y+ (_FONT_HEIGHT+2) * itemcount)
    {
      while(mouse->left_pressed)
      {
        redraw_mouse_cursor();
        if(selected != (mouse->pointer_y - y) / (_FONT_HEIGHT+2))
        {
          if((selected = (mouse->pointer_y - y) / (_FONT_HEIGHT+2)) < itemcount)
            print_menu(window, menuid, selected);
        }
      }
      if(mouse->pointer_x > x && mouse->pointer_x < x+width
        && mouse->pointer_y > y && mouse->pointer_y < y+ (_FONT_HEIGHT+2) * itemcount)
      {
        return 1+(mouse->pointer_y - y) / (_FONT_HEIGHT+2);
      }
    }
    return -2;
  }
  return -1;
}
