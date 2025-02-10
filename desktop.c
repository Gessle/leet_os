#define _TASKBAR_HEIGHT 42

/*
This function loads a bitmap for a desktop icon
Arguments: pointer to the pointer where to allocate memory,
string that contains a path to the icon file
*/

static unsigned load_icon_bitmap(unsigned char **icon, char *icon_file)
{
  FILE *fp;
  *icon = malloc(100*sizeof(char));

  if(!icon) return 0;
  
  if(fp=fopen(icon_file, "rb"))
  { 
    fread(*icon, sizeof(char), 100, fp);
    fclose(fp);
  }
  else
  {
    free(icon);
    return 0;
  }
  return 1;
}

/*
Loads icon
arguments: pointer to the icon struct, path to the icon bitmap file,
a string that contains the text of icon
*/
static void load_icon(struct icon *icon, char *icon_file, unsigned char *text)
{
  load_icon_bitmap(&icon->bitmap, icon_file);
  if(text)
  {  
    icon->text = malloc((1+strlen(text))*sizeof(char));
//    while(*tp++ = *text++);
    strcpy(icon->text, text);
  }
}
 
static void init_desktop(void)
{
  char *static_icons[][2] =
  {
    {
      "icons/runprmpt.ico", "Run program"
    },
    {
      "icons/fileman.ico", "File manager"
    },
    {
      "icons/cpanel.ico", "Control panel"
    },
    {
      "icons/clock.ico", "Clock"
    },
    {
      "icons/vpucon.ico", "VPU Console"
    }
  };
  unsigned x = 20;
//  windows = malloc(sizeof(struct prog_window*));
//  window_visibility_order = malloc(sizeof(int));
//  desktop_icons = malloc((sizeof(static_icons)/sizeof(char*))*sizeof(struct icon));
  desktop_icons = malloc(((sizeof(static_icons)>>1)/sizeof(char*))*sizeof(struct icon));
  default_icons = malloc(sizeof(char**));

  default_icons[0] = malloc(sizeof(char*));
  load_icon_bitmap(&default_icons[0], "icons/wndwdflt.ico");

  // load static icons
  for(;desktop_icon_count<(sizeof(static_icons)>>1)/sizeof(char*);desktop_icon_count++,x+=60)
  {
    load_icon(&desktop_icons[desktop_icon_count], static_icons[desktop_icon_count][0], static_icons[desktop_icon_count][1]);
    desktop_icons[desktop_icon_count].x=x;
    desktop_icons[desktop_icon_count].y=20;    
  }

  init_desktop_menu();
}

#define _ICON_SHADOW_COLOR _WINDOW_BGCOLOR

static void draw_icon(unsigned int x, unsigned int y, unsigned char *icon, unsigned char *text, unsigned char color)
{
  register unsigned int n=20;
  register unsigned int m;
  unsigned int xc;
  unsigned shadowcolor;
  unsigned pixel;

  if(color == _DESKTOP_BGCOLOR)
    shadowcolor = _DESKTOP_BGCOLOR;
  else
    shadowcolor = _ICON_SHADOW_COLOR;

  while(n--)
  {
/*    for(m=5,xc=x;m--;)
    {
      if(*icon & 0x80)
        put_pixel_shadow(xc, y, color, shadowcolor);
      xc++;
      if(*icon & 0x40)
        put_pixel_shadow(xc, y, color, shadowcolor);
      xc++;      
      if(*icon & 0x20)
        put_pixel_shadow(xc, y, color, shadowcolor);
      xc++;
      if(*icon & 0x10)
        put_pixel_shadow(xc, y, color, shadowcolor);
      xc++;
      if(*icon & 0x08)
        put_pixel_shadow(xc, y, color, shadowcolor);
      xc++;
      if(*icon & 0x04)
        put_pixel_shadow(xc, y, color, shadowcolor);
      xc++;
      if(*icon & 0x02)
        put_pixel_shadow(xc, y, color, shadowcolor);
      xc++;
      if(*icon & 0x01)
        put_pixel_shadow(xc, y, color, shadowcolor);
      xc++;
      icon++;
    } */
    for(m=5,xc=x;m--;)
    {
      for(pixel=0x80;pixel;xc++, pixel>>=1)
      {
        if(*icon & pixel)
          put_pixel_shadow(xc, y, color, shadowcolor);
      }
      icon++;
    }
    y++;
  }

  gprint_text(x+1, y+17-_FONT_HEIGHT, text, shadowcolor, _FONT_HEIGHT, x+_WINDOW_ICONS_MARGIN, 0);  
  gprint_text(x, y+16-_FONT_HEIGHT, text, color, _FONT_HEIGHT, x+_WINDOW_ICONS_MARGIN, 0);
}

static void save_desktop_shortcut(unsigned shortcut)
{
  char coords[5];
  unsigned n;
  FILE *fp;
  char shortcut_file_path[81];

  sprintf(shortcut_file_path, "DESKTOP\\%s", desktop_shortcuts[shortcut].shortcut_file);  
  
  fp = fopen(shortcut_file_path, "w");
  
  for(n=0;desktop_shortcuts[shortcut].path[n];n++)
    fputc(desktop_shortcuts[shortcut].path[n], fp);
  fputc(';', fp);

  for(n=0;desktop_shortcuts[shortcut].working_dir[n];n++)
    fputc(desktop_shortcuts[shortcut].working_dir[n], fp);
  fputc(';', fp);
  
  for(n=0;desktop_shortcuts[shortcut].text[n];n++)
    fputc(desktop_shortcuts[shortcut].text[n], fp);
  fputc(';', fp);

  sprintf(coords, "%i", desktop_shortcuts[shortcut].x);
  for(n=0;coords[n];n++)
    fputc(coords[n], fp);
  fputc(';', fp);
  
  sprintf(coords, "%i", desktop_shortcuts[shortcut].y);
  for(n=0;coords[n];n++)
    fputc(coords[n], fp);
  fputc(';', fp);
  
  sprintf(coords, "%i", desktop_shortcuts[shortcut].ask_args);
  for(n=0;coords[n];n++)
    fputc(coords[n], fp);
  fputc(';', fp);
  
  if(desktop_shortcuts[shortcut].icon_path)
  {
    for(n=0;desktop_shortcuts[shortcut].icon_path[n];n++)
      fputc(desktop_shortcuts[shortcut].icon_path[n], fp);
  }

  fclose(fp);
}

static unsigned load_desktop_shortcuts(void)
{
  FILE *fp;
  FILE *icon;
  unsigned int m;
  int c;
  char coords[5];
  char icon_path[81];
  struct find_t fileinfo;
  unsigned rc;
  if(!(desktop_shortcuts = malloc(sizeof(struct desktop_shortcut))))
    return 0;

  rc = _dos_findfirst("DESKTOP\\*.lnk", _A_NORMAL, &fileinfo); 
  while(!rc)
  {
    if(shortcut_count)
    {
      if(!(desktop_shortcuts = realloc(desktop_shortcuts, (1+shortcut_count)*sizeof(struct desktop_shortcut))))
        return 0;
    }
      
    sprintf(icon_path, "DESKTOP\\%s", fileinfo.name);
    fp = fopen(icon_path, "r");

    strcpy(desktop_shortcuts[shortcut_count].shortcut_file, fileinfo.name);

    for(m=0;(c = fgetc(fp)) != ';';m++)
      desktop_shortcuts[shortcut_count].path[m] = c;
    desktop_shortcuts[shortcut_count].path[m]=0;

    for(m=0;(c = fgetc(fp)) != ';';m++)
      desktop_shortcuts[shortcut_count].working_dir[m] = c;
    desktop_shortcuts[shortcut_count].working_dir[m] = 0;

    for(m=0;(c = fgetc(fp)) != ';';m++)
      desktop_shortcuts[shortcut_count].text[m] = c;
    desktop_shortcuts[shortcut_count].text[m]=0;      

    for(m=0;(c = fgetc(fp)) != ';';m++)
      coords[m] = c;
    coords[m] = 0;
    desktop_shortcuts[shortcut_count].x = atoi(coords);

    for(m=0;(c = fgetc(fp)) != ';';m++)
      coords[m] = c;
    coords[m] = 0;      
    desktop_shortcuts[shortcut_count].y = atoi(coords);

    for(m=0;(c = fgetc(fp)) != ';';m++)
      coords[m] = c;
    coords[m] = 0;      
    desktop_shortcuts[shortcut_count].ask_args = atoi(coords);    

    for(m=0;(c = fgetc(fp)) != -1;m++)
      icon_path[m] = c;
    icon_path[m] = 0;

    fclose(fp);

    if(icon = fopen(icon_path, "rb"))
    {
      if(!(desktop_shortcuts[shortcut_count].icon = malloc(100*sizeof(char))))
        return 0;
      fread(desktop_shortcuts[shortcut_count].icon, sizeof(char), 100, icon);
      fclose(icon);
      
      if(!(desktop_shortcuts[shortcut_count].icon_path = malloc(81*sizeof(char))))
        return 0;
      sprintf(desktop_shortcuts[shortcut_count].icon_path, "%s", icon_path);
    }
    else
    {
      desktop_shortcuts[shortcut_count].icon = 0;
      desktop_shortcuts[shortcut_count].icon_path = 0;
    }

    shortcut_count++;
    rc = _dos_findnext(&fileinfo);     
  }
  return 1;
}

/*void draw_desktop_shortcuts(void)
{
  unsigned int n = shortcut_count;
  unsigned char color;
  while(n--)
  {
    if(icon_selected == desktop_icon_count + n)
    {
      color = _ICON_ACOLOR;
      draw_borders(desktop_shortcuts[n].x, desktop_shortcuts[n].y, 40, 35, 2);            
    }
    else
    {
      color = _ICON_ICOLOR;
      draw_borders(desktop_shortcuts[n].x, desktop_shortcuts[n].y, 40, 35, 4);                  
    }
      
    if(desktop_shortcuts[n].icon)
      draw_icon(desktop_shortcuts[n].x, desktop_shortcuts[n].y, desktop_shortcuts[n].icon, desktop_shortcuts[n].text, color);
    else
      draw_icon(desktop_shortcuts[n].x, desktop_shortcuts[n].y, default_icons[0], desktop_shortcuts[n].text, color);    
  }
}*/

static void draw_desktop_icon(unsigned n, unsigned undraw)
{
  unsigned x;
  unsigned y;
  unsigned char *bitmap;
  char *text;
  unsigned char color;
  void *ptr = running_window;


  if(n >= desktop_icon_count)
  {
    x = desktop_shortcuts[n-desktop_icon_count].x;
    y = desktop_shortcuts[n-desktop_icon_count].y;
    text = desktop_shortcuts[n-desktop_icon_count].text;
    if(desktop_shortcuts[n-desktop_icon_count].icon)
      bitmap = desktop_shortcuts[n-desktop_icon_count].icon;
    else
      bitmap = default_icons[0];
  }
  else
  {
    x = desktop_icons[n].x;
    y = desktop_icons[n].y;
    bitmap = desktop_icons[n].bitmap;
    text = desktop_icons[n].text;
  }
  running_window = (void*)-1;
//  running_window = 0;
  if(undraw)
  {
    color = _DESKTOP_BGCOLOR;
    draw_borders(x, y, 40, 35, 4);                            
  } 
  else if(icon_selected == n)
  {
    color = _ICON_ACOLOR;
    draw_borders(x, y, 40, 35, 2);                  
  }
  else
  {
    color = _ICON_ICOLOR;
    draw_borders(x, y, 40, 35, 4);                        
  }
  draw_icon(x, y, bitmap, text, color);
  running_window = ptr;
}

static void move_icon(unsigned icon, int dx, int dy)
{
  unsigned oldx, oldy;

  draw_desktop_icon(icon, 1);  
  if(icon < desktop_icon_count)
  {
    oldx = desktop_icons[icon].x;
    oldy = desktop_icons[icon].y;
    desktop_icons[icon].x += dx;
    desktop_icons[icon].y += dy;
  }
  else
  {
    oldx = desktop_shortcuts[icon-desktop_icon_count].x;
    oldy = desktop_shortcuts[icon-desktop_icon_count].y;
    desktop_shortcuts[icon-desktop_icon_count].x += dx;
    desktop_shortcuts[icon-desktop_icon_count].y += dy;
    if(desktop_shortcuts[icon-desktop_icon_count].x < 0 || desktop_shortcuts[icon-desktop_icon_count].x > _RES_X - 30)
      desktop_shortcuts[icon-desktop_icon_count].x = _RES_X - 30;
    if(desktop_shortcuts[icon-desktop_icon_count].y < 0 || desktop_shortcuts[icon-desktop_icon_count].y > _RES_Y - 30)
      desktop_shortcuts[icon-desktop_icon_count].y = _RES_Y - 30;      
  }
  draw_desktop_icon(icon, 0);
}

static void draw_select_icon(unsigned old, unsigned new)
{
  draw_desktop_icon(old, 0);
  draw_desktop_icon(new, 0);
}

static void draw_desktop_icons(void)
{
  unsigned int n = desktop_icon_count + shortcut_count;  

  while(n--)
    draw_desktop_icon(n, 0);

}

static void draw_window_icons(void)
{
  unsigned int n;
  unsigned int x = _WINDOW_ICONS_LEFT;
  for(n=0;n<window_count;n++)
  {
    if(windows[n]->minimized)
    {
      draw_borders(x, _RES_Y-40, _WINDOW_ICONS_MARGIN-2, 39, 0);
      if(windows[n]->icon)
        draw_icon(x, _RES_Y-40, windows[n]->icon, windows[n]->title, _WINDOW_ICON_COLOR);
      else
        draw_icon(x, _RES_Y-40, default_icons[0], windows[n]->title, _WINDOW_ICON_COLOR);
      x+=_WINDOW_ICONS_MARGIN;
    }
  }  
}

static void draw_taskbar(void)
{
  unsigned int n;
  unsigned int x = _WINDOW_ICONS_LEFT;
  void *ptr = draw_window_box;
  void *ptr2 = running_window;
  
  draw_window_box = 0;
  running_window = (void*)-1;
//  running_window = 0;
  
  draw_line(0, _RES_Y-_TASKBAR_HEIGHT-1, _RES_X, _RES_Y-_TASKBAR_HEIGHT-1, _BORDERCOLOR_LIGHT);
  draw_rectangle(0, _RES_Y-_TASKBAR_HEIGHT, _RES_X, _TASKBAR_HEIGHT, _WINDOW_BGCOLOR);
  draw_window_icons();
  
  draw_window_box = ptr;
  running_window = ptr2;

/*  for(n=0;n<window_count;n++)
  {
    if(windows[n]->minimized)
    {
      draw_borders(x, _RES_Y-_FONT_HEIGHT-1, _WINDOW_ICONS_MARGIN-2, _FONT_HEIGHT, 0);
      gprint_text(x, _RES_Y-1, windows[n]->title, _WINDOW_BASICTEXTCOLOR, _FONT_HEIGHT, x+_WINDOW_ICONS_MARGIN-2, 0);
      x+=_WINDOW_ICONS_MARGIN;
    }
  }  */
}

static void draw_desktop(void)
{
  unsigned int x=0;
  unsigned int y=0;
  void *rw = running_window;
  running_window = (void*)-1;

  if(!wallpaper_struct.bitmap)
  {
    if(!draw_window_box)
      fill_screen(_DESKTOP_BGCOLOR);
    else
    {
      draw_rectangle(draw_window_box->x-1, draw_window_box->y-1, draw_window_box->width+2, draw_window_box->height+4+_FONT_HEIGHT, _DESKTOP_BGCOLOR);
    }
  }
  else
    draw_xpm_bitmap(&wallpaper_struct, 0, _FONT_HEIGHT+2, _RES_X, _RES_Y-_TASKBAR_HEIGHT-1);

  draw_desktop_icons();

//  draw_menubar();
  running_window = rw;
}

static void draw_windows(void)
{
  unsigned int n;
//  draw_window_icons();
  draw_taskbar();
  for(n=0;n<window_count;n++)
  {
    running_window=windows[window_visibility_order[n]];
    if(!windows[window_visibility_order[n]]->minimized && !windows[window_visibility_order[n]]->hidden)            
      if(active_window == window_visibility_order[n])
        draw_window(windows[window_visibility_order[n]], 1, 0);
      else
        draw_window(windows[window_visibility_order[n]], 0, 0);
  }
  draw_menubar();
}

void draw_screen(void)
{
  if(_VIDEO_MODE != video)
    restore_videomode();
  draw_desktop();
  draw_windows();
}

static void update_titlebar(struct prog_window *window)
{
  if(titlebar_drag)
  {
    titlebar_drag->x0 = window->x;
    titlebar_drag->y0 = window->y;
    titlebar_drag->x1 = window->x+window->width;
    titlebar_drag->y1 = window->y+_FONT_HEIGHT+2;
  }
}

// sets active window. returns active window
int set_active_window(int window, unsigned draw)
{
  int n = window;
  int prev_active_window = active_window;
  void *rw;

  if(window>=0)
  {
    // if the desired active window is hidden, loop until a non-hidden non-minimized window is found
    while(windows[window]->hidden || windows[window]->minimized)
    {
      draw = 1; // and enable drawing the new active window
      if(!window--)
        window = window_count-1;
      // if no non-hidden windows were found, desktop becomes the active window
      if(window == n)
      {
        active_window = -1;
        goto end;
      }
    }
    // find the new active window from window visibility order table
    for(n=0;n<window_count;n++)
      if(window_visibility_order[n] == window)
        break;
    // move all entries in the table from that point left by one
//    for(;n<window_count-1;n++)
//      window_visibility_order[n] = window_visibility_order[n+1];
    // use memmove:
    memmovel(&window_visibility_order[n], &window_visibility_order[n+1], (window_count-n-1)*sizeof(int));
    // put the new active window to top
    window_visibility_order[window_count-1]=active_window=window;

    update_titlebar(windows[active_window]);

    // redraw the title bar of the previous active window
/*    if(window_count>1 && !windows[window_visibility_order[n-1]]->minimized
    && !windows[window_visibility_order[n-1]]->hidden && draw)
      draw_window(windows[window_visibility_order[n-1]], 0, 2);*/
    if(prev_active_window != -1 && draw)
      draw_window(windows[prev_active_window], 0, 2);
    // draw the new active window
//    if(window_visibility_order[n-1] != active_window && draw)
    if(draw)
    {
      rw = running_window;
      running_window = 0;
      draw_window(windows[active_window], 1, 0);
      running_window = rw;
    }
  }
  else
    active_window=window;

  end:
  running_window = 0;
  if(!windows[active_window]->maximized)
    draw_menubar();

  return active_window;
}
