#define _TEXTVIEW_MAXROWS 16000

unsigned long getfilelen(char *path)
{
/*  long int n;
  FILE *fp = fopen(path, "rb");
  if(!fp) return -1;
  while(fgetc(fp) != -1);
  n = ftell(fp);
  fclose(fp);
  return n;*/
  struct find_t fileinfo;

  if(_dos_findfirst(path, 0xFF, &fileinfo))
    return -1;
  return fileinfo.size;    
}

unsigned int getfilerows(FILE *fp)
{
  unsigned int n = 0;
  while(!feof(fp))
    if(fgetc(fp) == '\n')
      n++;
  rewind(fp);
  return n;
}

void textview_place_elements(struct prog_window *window)
{
  window->window_grid[0]->width = window->width-25;
  window->window_grid[0]->height = window->height-65;

  window->window_grid[1]->x = window->window_grid[0]->x+window->window_grid[0]->width+1;
  window->window_grid[1]->width = window->window_grid[0]->height-26;
  
  if(window->window_grid[0]->c > window->window_grid[0]->height/(_FONT_HEIGHT+2))
    window->window_grid[1]->c = window->window_grid[0]->c-window->window_grid[0]->height/(_FONT_HEIGHT+2);
  else
    window->window_grid[1]->c = 0;
  
  window->window_grid[2]->y = window->window_grid[0]->y+window->window_grid[0]->height+1;
  window->window_grid[2]->width = window->window_grid[0]->width-26;

  window_recalc_hitboxes(window);
  
//  draw_window(window, 1, 1);
}

int read_file(char *path, struct window_element *textbox, struct window_element *scroll, struct prog_window *window)
{
  FILE *fp = fopen(path, "r");
  unsigned int n = 0;
  unsigned int m=0;
  int c;
  long int filepos;

  if(!fp)
    return 0;

  // get the amount of rows
  if((m = getfilerows(fp)) > _TEXTVIEW_MAXROWS)
  {
    fclose(fp);
    return 0;
  }

  if(!(textbox->fitems = _fcalloc(m+1, sizeof(unsigned char __far*))))
  {
    fclose(fp);
    nomemory();
    return 0;
  }
  textbox->c = m+1;

/*  if(textbox->c > textbox->height/(_FONT_HEIGHT+2))
    scroll->c = textbox->c-textbox->height/(_FONT_HEIGHT+2);
  else
    scroll->c = 0;*/
  textview_place_elements(window);

  while(!feof(fp))
  {
    // get the length of the current row
    filepos = ftell(fp);
    while(!feof(fp) && fgetc(fp) != '\n');
    if(!(textbox->fitems[n] = (unsigned char __far*)_fmalloc((ftell(fp)-filepos+1)*sizeof(unsigned char))))
    {
      while(n--) _ffree(textbox->fitems[n]);
      _ffree(textbox->fitems);
      textbox->fitems=0;
      nomemory();
      fclose(fp);
      return 0;
    }
    fseek(fp, filepos, SEEK_SET);
    // read the row to the array
    m=0;
    while(!feof(fp) && (c = fgetc(fp)) != '\n')
      textbox->fitems[n][m++] = c;
    textbox->fitems[n][m++] = 0;
    n++;
  }    
  fclose(fp);
  return 1;
}

void textview_arrows(struct prog_window *window)
{
  unsigned count=1;
  switch(getch())
  {
    case 81:
      count=window->window_grid[0]->height/(_FONT_HEIGHT+2);
    case 80:
      goto down;
    case 73:
      count=window->window_grid[0]->height/(_FONT_HEIGHT+2);
    case 72:
      goto up;
    case 71:
      count=window->window_grid[0]->width>>3;    
    case 75:
      goto left;
    case 79:
      count=window->window_grid[0]->width>>3;
    case 77:
      goto right;
  }
  down:
  while(window->window_grid[1]->scroll_x < window->window_grid[1]->c && count--)
    window->window_grid[1]->scroll_x = ++window->window_grid[0]->scroll_y;
  goto end;
  up: 
  while(window->window_grid[1]->scroll_x && count--)
    window->window_grid[1]->scroll_x = --window->window_grid[0]->scroll_y;
  goto end;
  left:
  while(window->window_grid[2]->scroll_x && count--)
    window->window_grid[2]->scroll_x = --window->window_grid[0]->scroll_x;
  goto end;
  right:
  while(count--)
    window->window_grid[2]->scroll_x = ++window->window_grid[0]->scroll_x;          
  end:
//  draw_window(window, 1, 1);
  window_drawelement(window, 0);
  window_drawelement(window, 1);
  window_drawelement(window, 2);    
}
unsigned textview_program(struct prog_window *window, unsigned args, int *keypress)
{
  if(keypress)
  {
    if(!*keypress)
      textview_arrows(window);
    else if(*keypress>0)
    {
      switch(*keypress)
      {
        case 109: case 77:      
          args= WM_WINPROGACT | 3;
        break;
        case 78: case 110:      
          args= WM_WINPROGACT | 4;
      }
    }
  }
  if(window->resized)
  {
    textview_place_elements(window);
//    if(window->maximized)
      draw_window(window, 1, 0);
//    else
//      screen_redraw=1;
    draw_mouse_cursor(1);                          
    window->resized=0;
  }  
/*  if(args == 3)
  {
    if(!monospace_font)
      load_system_font(monospace_font_file, &monospace_font);
  }*/
  
  if(args == (WM_WINPROGACT | 3) || args == (WM_WINPROGACT | 4))
  {
    window->window_grid[0]->monospace=args-(WM_WINPROGACT | 4);
    window_drawelement(window, 0);
  }  
    
  return ~WM_WINPROGACT;   
}

struct prog_window *init_textview_window(char *file, char *path)
{
  struct prog_window *retval;
  struct window_element *element;
  struct window_element *element2;
  char title[27];

  sprintf(title, "%s - Text viewer", file);
  if(!(retval = new_window(-1, -1, 300, 190, title, 1, 1, 1, 0, &textview_program)))
  {
    nomemory();
    return 0;
  }
  if(!(element = window_list(retval, 10, 32, 275, 125, 0, 0)))
  {
    nomemory:
    del_window(retval);
    nomemory();
    return 0;
  }
  element->active = 0;
  // vertical scrollbar
  if(!(element2 = window_scrollbar(retval, element->x+element->width+1, element->y,
  0, element->height-26, 0, 0, (int *)&element->scroll_y)))
    goto nomemory;
  // horizontal scrollbar
  if(!(window_scrollbar(retval, element->x, element->y+element->height+1, 1, element->width-26, 64000, 0, (int *)&element->scroll_x)))
    goto nomemory;

  if(!(window_button(retval, 10, 10, 120, 2+_FONT_HEIGHT, "\021Monospace font", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;
  if(!(window_button(retval, 132, 10, 100, 2+_FONT_HEIGHT, "\021Normal font", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;
  
  if(path)
  {
//    if(getfilelen(path) > _TEXTVIEW_MAXMEM)
//    {
    if(!read_file(path, element, element2, retval))
      show_error_msg("Too big file!");
      //dialog(20, 20, 250, 50, 0, 0, 0, "Too big file! Use text editor to open it.", "Info", 0);
//    }
//    else
//      read_file(path, element, element2);
  }
//  window_drawelement(retval, 0);

  draw_window(retval, 1, 1);

  return retval;
}
