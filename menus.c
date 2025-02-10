#define _MENU_WIDTH 100

struct prog_window *menu_window;

struct dropmenu *new_menu(char *label, struct dropmenu *prev)
{
  struct dropmenu *retval = calloc(1, sizeof(struct dropmenu));

  if(!retval) return 0;

  retval->label = malloc((strlen(label)+1)* sizeof(char));
  if(!retval->label)
  {
    free(retval);
    return 0;
  }
  strcpy(retval->label, label);

  if(prev)
    prev->next = retval;

  return retval;
}

struct dropmenu *new_menuitem(struct dropmenu *menu, char *label, unsigned action)
{
  struct dropmenu *ptr = menu->items;
  
  if(!ptr)
  {
    ptr = new_menu(label, 0);
    menu->items = ptr;
  }
  else
  {
    while(ptr->next)
      ptr = ptr->next;
    ptr = new_menu(label, ptr);
  }

  if(ptr)
    ptr->action = action;
  
  return ptr;    
}

static void init_desktop_menu(void)
{
  void *ptr;
  
  desktop_menu = new_menu("Desktop", 0);
  new_menuitem(desktop_menu, "Show desktop", WM_MINIMIZEALL);
  new_menuitem(desktop_menu, "Exit to DOS", WM_QUIT);
  
  ptr = new_menu("Window", desktop_menu);
  new_menuitem(ptr, "Tile", WM_TILE);
  new_menuitem(ptr, "Cascade", WM_CASCADE);
}

static void print_menuitem(int x, int y, unsigned char *text, unsigned selected)
{
  if(!selected)
  {
    draw_rectangle(x, y, _MENU_WIDTH, _FONT_HEIGHT+2, _MENU_BGCOLOR);
    gprint_text(x+2, y+1, text, _WINDOW_BASICTEXTCOLOR, _FONT_HEIGHT, x+_MENU_WIDTH, 0);    
  }
  else
  {
    draw_rectangle(x, y, _MENU_WIDTH, _FONT_HEIGHT+2, _MENU_SELCOLOR);
    gprint_text(x+2, y+1, text, _MENU_ATEXTCOLOR, _FONT_HEIGHT, x+_MENU_WIDTH, 0);    
  }  

  draw_line(x+2, y+_FONT_HEIGHT, x+_MENU_WIDTH-2, y+_FONT_HEIGHT, _MENU_LINECOLOR);
}

static void draw_menubar_menu(struct dropmenu *menu)
{
  unsigned x = 6;
  while(menu)
  {
    gprint_text(x, 1, menu->label, _WINDOW_BASICTEXTCOLOR, _FONT_HEIGHT, x+40, 0);
    x += 40;
    if(menu = menu->next)
    {
      draw_line(x, 1, x, _FONT_HEIGHT, _MENU_LINECOLOR);
      x += 10;
    }
  }
}

static void draw_menubar(void)
{
  void *rw = running_window;
  running_window = 0;
  draw_rectangle(0, 0,_RES_X, _FONT_HEIGHT+2, _WINDOW_BGCOLOR);
  draw_line(0, _FONT_HEIGHT+2, _RES_X, _FONT_HEIGHT+2, _BORDERCOLOR_SHADOW);

  if(active_window >= 0)
  {
    draw_menubar_menu(windows[active_window]->menus.menubar);
  }
  else
  {
    draw_menubar_menu(desktop_menu);
  }
  running_window = 0;
}

static struct coords print_menu(struct prog_window *window, unsigned int menuid, unsigned int selection)
{
  unsigned n;
  unsigned x = 0;
  struct dropmenu *menu;
  struct dropmenu *ptr;
  struct coords retval;
  int scrrow = retval.y = _FONT_HEIGHT+2;

  menu_window = calloc(1, sizeof(struct prog_window));

  // menuid 0 = upleft corner dropdown menu
  if(!menuid)
  {
//    scrrow = window->y+_FONT_HEIGHT+2;
    retval.y = scrrow += window->y;
    retval.x = window->x;
   
    draw_borders(window->x+2, window->y+2, _FONT_HEIGHT-2, _FONT_HEIGHT-2, 1);
    draw_borders(window->x, scrrow, _MENU_WIDTH, window->menus.upleft_count*(_FONT_HEIGHT+2), 0);
    for(n=0;n < window->menus.upleft_count;n++,scrrow+=_FONT_HEIGHT+2)
    {
      if(n==selection)
        print_menuitem(window->x, scrrow, window->menus.upleft[n], 1);
      else
        print_menuitem(window->x, scrrow, window->menus.upleft[n], 0);
    }
  }
  else
  {
    if(!window)
      menu = desktop_menu;
    else
      menu = window->menus.menubar;
      
    for(;--menuid;menu=menu->next)
      x += 50;

    retval.x = x;
    for(n=0,ptr=menu->items;ptr;ptr=ptr->next,n++)
    {
      if(n == selection)
        print_menuitem(x, scrrow, ptr->label, 1);
      else
        print_menuitem(x, scrrow, ptr->label, 0);
      scrrow += _FONT_HEIGHT+2;
    }
    draw_borders(x, _FONT_HEIGHT+2, _MENU_WIDTH, n*(_FONT_HEIGHT+2), 0);
  }

  if(menu_window)
  {
    menu_window->x = retval.x;
    menu_window->y = retval.y;
    menu_window->width = _MENU_WIDTH;
    menu_window->height = (n-1)*(_FONT_HEIGHT+2);
  }

  return retval;
}

static unsigned menu_count(struct prog_window *window)
{
  unsigned retval = 0;
  struct dropmenu *ptr;
  
  if(!window)
    ptr = desktop_menu;
  else
    ptr = window->menus.menubar;
    
  while(ptr)
  {
    retval++;
    ptr = ptr->next;
  }

  return retval;
}

int menu_selected = -1;

static int dropmenu_action(struct prog_window *window, unsigned selection)
{
  struct dropmenu *ptr;// = desktop_menu;

  if(!window)
    ptr = desktop_menu;
  else
    ptr = window->menus.menubar;
  
  while(--menu_selected)
    ptr = ptr->next;    
    
  ptr = ptr->items;
  while(selection--)
    ptr = ptr->next;
  return ptr->action;
}

static void menu_draw_screen(void)
{
  draw_window_box = menu_window;
  draw_screen();
  free(draw_window_box);
  draw_window_box = 0;
}

static int dropmenu_handler(struct prog_window *window)
{
  int retval = 0;
  static int selection = -2;
  static struct coords coords;
  static int keyboard_select = 0;
  unsigned y;
  unsigned n;
  struct dropmenu *ptr;

  if(menu_selected > menu_count(window))
  {
    menu_selected = -1;
    return 0;
  }

  if(selection == -2)
  {
    selection = -1;
    coords = print_menu(window, menu_selected, selection);
    if(mouse)
      wait_leftbutton_release();
    return 0;
  }

  if(mouse)
  {
    while(mouse->left_pressed)
    {
      get_mouse_movement();
      if(mouse->pointer_x > coords.x && mouse->pointer_y > coords.y
         && mouse->pointer_x < coords.x + _MENU_WIDTH)
      {
        y = mouse->pointer_y - coords.y;
        selection = y/(_FONT_HEIGHT+2);
        print_menu(window, menu_selected, selection);        
        wait_leftbutton_release();
      }
      else
      {
        goto exitmenu;
      }
      keyboard_select = 0;
    }
  }

  if(kbhit())
  {
    zero_screensaver_timer();
    keyboard_select = 1;
    switch(getch())
    {
      case 27: goto exitmenu;
      case 13:
        keyboard_select = 0;
        break;
      case 0:
        switch(getch())
        {
          case 72:
            if(selection >= 0)
              selection--;
            else
              selection = 0;
          break;
          case 80:
            selection++;
          break;
          case 75:
            menu_selected--;
            if(!window && !menu_selected
               || menu_selected < 0)
                 menu_selected = menu_count(window);         
            menu_draw_screen();
            break;
          case 77:
            menu_selected++;
            if(menu_selected > menu_count(window))
            {
              menu_selected = 0;
              if(!window)
                menu_selected++;
            }
            menu_draw_screen();
            break;
        }
    }
    print_menu(window, menu_selected, selection);            
  }

  if(!keyboard_select && selection >= 0)
  {
    if(menu_selected > 0)
      retval = dropmenu_action(window, selection);
    else
      retval = selection+1;
    exitmenu:
    menu_selected = -1;
    selection = -2;
    keyboard_select = 0;
    menu_draw_screen();
    return retval;
  }

  return 0;
}

/*
Keyboard & mouse handler for drop-down menus
Returns: selection id
*/

/*int window_menu(struct prog_window *window, unsigned int menuid)
{
  int a;
  unsigned int selection = 0;

  reset_vpu_interrupt_handlers();

  draw_mouse_cursor(2);  
  print_menu(window, menuid, selection);
  draw_mouse_cursor(1);
  // loop until the left button is released
  if(mouse)
    while(mouse->left_pressed)
      if(get_mouse_movement())
        draw_mouse_cursor(0);                

  while(1)
  {
    call_drv_screenupdate();
    if(!menuid)
    {
      if(mouse && (a = menu_mouse_handler(window, window->x, window->y+_FONT_HEIGHT+2, _MENU_WIDTH, window->menus.upleft_count, menuid)) != -1)
        break;
      if(kbhit() && (a = getch()))
      {
        if(a == 80 && selection < window->menus.upleft_count-1) selection++;
        else if(a == 72 && selection) selection--;
        print_menu(window, menuid, selection);          
        if(a == 13)
        {
          set_vpu_interrupt_handlers();          
          return selection+1;
        }
        if(a == 27)
        {
          set_vpu_interrupt_handlers();          
          return -1;
        }
      }
      else
      {
        
      }
    }
  }
  set_vpu_interrupt_handlers();
  return a;
}*/
