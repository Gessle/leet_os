unsigned char upleft_move[] = "Move window";
unsigned char upleft_close[] = "Close window";
unsigned char upleft_maximize[] = "(Un)maximize";
unsigned char upleft_resize[] = "Resize";
unsigned char upleft_minimize[] = "Minimize";
unsigned char dialog_ok[] = "OK";
unsigned char dialog_cancel[] = "Cancel";
unsigned char dialog_yes[] = "Yes";
unsigned char dialog_no[] = "No";
unsigned char dialog_help[] = "Help [F1]";


static void element_active(struct prog_window *window, int element)
{
  int n;

  if(window->active_element == element)
  {
    goto end;
  }

  n = window->active_element;
  window->active_element = -1;
  if(n >= 0)
    window_drawelement(window, n);  
  window->active_element = element;  
  end:
  if(element >= 0)
    window_drawelement(window, element);
}

inline void window_drawelement_text(struct prog_window *window, unsigned element)
{
  struct xpm_bitmap image;
  struct window_element *elem = window->window_grid[element];
    
  if(!elem->ctype)
    gprint_text(window->x+elem->x, window->y+_FONT_HEIGHT+2+elem->y,
      elem->text, elem->color, _FONT_HEIGHT, window->x+window->width,
                elem->monospace);
  else
  {
    if(load_xpm_bitmap(elem->text, &image))
    {
      gprint_text(window->x+elem->x, window->y+_FONT_HEIGHT+2+elem->y,
        "Error", elem->color, _FONT_HEIGHT, window->x+window->width, 0);
      return;
    }
    if(elem->width && elem->height)
      draw_xpm_bitmap(&image, window->x+elem->x, window->y+_FONT_HEIGHT+2+elem->y,
        window->x+elem->x+elem->width, window->y+_FONT_HEIGHT+2+window->window_grid[element]->y+elem->height);
    else
      draw_xpm_bitmap(&image, window->x+elem->x, window->y+_FONT_HEIGHT+2+elem->y,
        window->x+elem->x+image.width, window->y+_FONT_HEIGHT+2+elem->y+image.height);
    free(image.bitmap);        
  }
}

inline void window_drawelement_button(struct prog_window *window, unsigned element)
{
  struct window_element *elem = window->window_grid[element];
  draw_borders(window->x+elem->x, window->y+_FONT_HEIGHT+2+elem->y,
    elem->width, elem->height, 0);
  gprint_text(window->x+10+elem->x, window->y+1+_FONT_HEIGHT+2+elem->y,
    elem->text, elem->color, _FONT_HEIGHT, window->x+10+elem->x+elem->width,
    elem->monospace);            
}

inline void window_drawelement_textbox(struct prog_window *window, unsigned element)
{
  struct window_element *elem = window->window_grid[element];

  draw_borders(window->x+elem->x, window->y+_FONT_HEIGHT+2+elem->y,
    elem->width, elem->height, 1);
  if(window->active_element != element)      
    draw_rectangle(window->x+elem->x, window->y+_FONT_HEIGHT+2+elem->y,
      elem->width, elem->height, _TEXTBOX_BGCOLOR);
/*  else
  {
    element_active(window, element);
    return;
  }*/
  else
    draw_rectangle(window->x+elem->x, window->y+_FONT_HEIGHT+2+elem->y,
      elem->width, elem->height, _TEXTBOX_ABGCOLOR);
  if(elem->text)
    gprint_text(window->x+elem->x+2, window->y+_FONT_HEIGHT+2+elem->y+2,
      elem->text, _TEXTBOX_TEXTCOLOR, _FONT_HEIGHT, window->x+elem->x+elem->width, elem->monospace);            
}

static void draw_label(unsigned x, unsigned y, unsigned width,
                       unsigned bgcolor, unsigned textcolor, char *text, unsigned monospace)
{
  draw_rectangle(x, y, width, _FONT_HEIGHT+2, bgcolor);
  gprint_text(x+2, y+2, text, textcolor, _FONT_HEIGHT, x+width, monospace);
}

inline void window_drawelement_list(struct prog_window *window, unsigned element)
{
  unsigned n;
  struct window_element *win_element = window->window_grid[element];
  unsigned element_rows = win_element->height / (_FONT_HEIGHT+2);
  unsigned element_x = window->x + win_element->x;
  unsigned element_y = window->y + win_element->y + _FONT_HEIGHT+2;
  unsigned monospace = win_element->monospace;
  unsigned element_width = win_element->width;
  unsigned scroll_x = win_element->scroll_x;
  unsigned scroll_y = win_element->scroll_y;
  unsigned count = win_element->c;
  unsigned bg_color;
  unsigned text_color;
  char *textptr;
  
  draw_borders(element_x, element_y, element_width, win_element->height, 1);
//  draw_rectangle(element_x, element_y, element_width, win_element->height, _TEXTBOX_BGCOLOR);
  if(win_element->items)
  {
    for(n=win_element->scroll_y;
      n < win_element->c
      && element_rows-- ;
        n++)
    {
      if(win_element->selectc == n)
      {
        bg_color = _ATEXT_BGCOLOR;
        text_color = _TEXTBOX_ATEXTCOLOR;
      }
      else
      {
        bg_color = _TEXTBOX_BGCOLOR;
        text_color = _TEXTBOX_TEXTCOLOR;
      }
      if(win_element->items[n])
        if(strlen(win_element->items[n]) >= win_element->scroll_x)
          textptr = &win_element->items[n][scroll_x];
        else
          textptr = &null_byte;
      else textptr = &null_byte;

      draw_label(element_x, element_y+(n-scroll_y)*(_FONT_HEIGHT+2),
         element_width, bg_color, text_color,
         textptr, monospace);
    }
  }
}

static char *get_str_pos(char *str, unsigned row, unsigned col)
{
  char *ptr;
  while(row--)
  {
    str = strchr(str, '\n');
    if(!str) return 0;
    str++;
  }
  if(ptr = strchr(str, '\n'))
  {
    if(col > ptr - str)
      return 0;
  }
  else if(col > strlen(str))
    return 0;
  return str + col;
}

static unsigned row_count(char *str)
{
  unsigned rc;
  for(rc = 1;str = strchr(str, '\n');rc++, str++);
  return rc;
}

static unsigned row_len(char *str);

// 0: draw caret normally, 1: remove caret, 2: reset caret
static void draw_textbox_caret(unsigned remove)
{
  unsigned n;
  static char visible = 0;

  if(remove == 2)
    visible = 0;
  else if(!remove)
  {
    if(caret_visible && !visible)
      goto draw;
    if(clock_ticks & 0x10)
    {
      if(!visible)
      {
        draw:
        for(n=_FONT_HEIGHT;n--;)
          if(get_pixel(caret_screenx, caret_screeny+n) != _TEXTBOX_ABGCOLOR)
            put_pixel(caret_screenx, caret_screeny+n, _TEXTBOX_ABGCOLOR);
          else
            put_pixel(caret_screenx, caret_screeny+n, _TEXTBOX_TEXTCOLOR);
        visible = 1;
      }
    }
    else
    {
      goto hide_caret;
    }
  }
  else hide_caret:if(visible)
  {
    visible = 0;
    for(n=_FONT_HEIGHT;n--;)
      if(get_pixel(caret_screenx, caret_screeny+n) != _TEXTBOX_TEXTCOLOR)
        put_pixel(caret_screenx, caret_screeny+n, _TEXTBOX_TEXTCOLOR);
      else
        put_pixel(caret_screenx, caret_screeny+n, _TEXTBOX_ABGCOLOR);
  }
  caret_visible = 0;
/*
  if(!remove) n = _TEXTBOX_TEXTCOLOR;
  else n = _TEXTBOX_BGCOLOR;
  draw_line(caret_screenx, caret_screeny, caret_screenx, caret_screeny+_FONT_HEIGHT-1, n);
*/
}

static void mltext_draw_selection(struct prog_window *window, unsigned, unsigned);

inline void window_drawelement_mltext(struct prog_window *window, unsigned element)
{
  struct window_element *textbox = window->window_grid[element];
  char *str = textbox->text;
  const unsigned row = textbox->selectc;
  const unsigned col = textbox->cursor_x;
  const unsigned rc = textbox->height / (_FONT_HEIGHT+2);
  const unsigned element_x = window->x+textbox->x;
  const unsigned element_y = window->y+2+_FONT_HEIGHT+textbox->y;
  const unsigned scroll_x = textbox->scroll_x;
  const unsigned scroll_y = textbox->scroll_y;
  const unsigned element_width = textbox->width;
  const unsigned monospace = textbox->monospace;
  unsigned n = 0;
  char *strptr = str;
  unsigned text_x, row_y, text_y;
  struct text_print text_print;
  const unsigned max_x = element_x+element_width;
  unsigned empty_row, empty_c;
  unsigned bgcolor;

  if(window->active_element == element)
  {
    bgcolor = _TEXTBOX_ABGCOLOR;
    draw_textbox_caret(2);
  }
  else bgcolor = _TEXTBOX_BGCOLOR;

  str = get_str_pos(str, scroll_y, 0);

  draw_borders(element_x, element_y, element_width, textbox->height, 1);

  if(row_len(str) < scroll_x)
  {
    draw_rectangle(element_x, element_y, element_width, 2+_FONT_HEIGHT, bgcolor);
    goto skip_short_rows;
  }
  else str += scroll_x;
  for(;n<rc;n++)
  {
    strptr = strchr(str, '\n');
    if(strptr) *strptr = 0;
    text_x = element_x+2;
    row_y = element_y+(n)*(_FONT_HEIGHT+2);
    text_y = row_y+2;
    if(scroll_y+n == row)
    {
      draw_rectangle(element_x, element_y+n*(_FONT_HEIGHT+2), element_width, _FONT_HEIGHT+2, bgcolor);
      if(col-scroll_x)
        text_print = cprint_text(text_x, text_y, str, _TEXTBOX_TEXTCOLOR, _FONT_HEIGHT,
          max_x, col-scroll_x, monospace);      
      else
      {
        text_print.x = text_x;
        text_print.text = str;
      }
      if(window->active_element == element)
      {
        caret_screenx = text_print.x-1;
        caret_screeny = text_y;
      }
      gprint_text(text_print.x, text_y, text_print.text, _TEXTBOX_TEXTCOLOR, _FONT_HEIGHT,
        max_x, monospace);
    }
    else
      draw_label(element_x, row_y, element_width, bgcolor, 
        _TEXTBOX_TEXTCOLOR, str, monospace);
    if(strptr) *strptr = '\n';
    skip_short_rows:if(!(strptr = get_str_pos(str, 1, scroll_x)))
    {
      empty_row = n+1;
      if(empty_row >= rc) break;
      empty_c = 1;
      while(strptr = get_str_pos(str, 1, 0))
      {
        if(row_len(strptr) > scroll_x)
        {
          str = strptr + scroll_x;
          break;
        }
        else
        {
          str = strptr;
          if(++n>=rc-1) break;
          empty_c++;
        }
      }
      draw_rectangle(element_x, element_y+empty_row*(_FONT_HEIGHT+2), element_width, empty_c*(_FONT_HEIGHT+2), bgcolor);
      if(!strptr)
      {
        mltext_draw_selection(window, element, 0);
        n++;
        goto end;
      }
    }
    else str = strptr;
  }
  return;
  end:draw_rectangle(element_x, element_y+n*(_FONT_HEIGHT+2), element_width, (rc-n)*(_FONT_HEIGHT+2), bgcolor);
}

inline void draw_selection_mark(unsigned x, unsigned y, unsigned undraw)
{
  unsigned n = _FONT_HEIGHT+2;

  x--;

  while(n--)
  {
    if(!undraw)
    {
      if(get_pixel(x, y) != _TEXTBOX_ABGCOLOR)
        put_pixel(x, y, _TEXTBOX_ABGCOLOR);
      else
        put_pixel(x, y, _TEXTBOX_TEXTCOLOR);
    }
    else
    {
      if(get_pixel(x, y) == _TEXTBOX_ABGCOLOR)
        put_pixel(x, y, _TEXTBOX_TEXTCOLOR);
      else
        put_pixel(x, y, _TEXTBOX_ABGCOLOR);
    }
    y++;
  }
}

static int text_is_selected(struct window_element *textbox)
{
  if(textbox->select_start_row != textbox->select_end_row
    || textbox->select_start_col != textbox->select_end_col)
    return 1;
  return 0;
}

static void mltext_draw_selection(struct prog_window *window, unsigned element, unsigned undraw)
{    
  struct window_element *textbox = window->window_grid[element];
  const unsigned select_start_row = textbox->select_start_row;
  const unsigned select_start_col = textbox->select_start_col;
  const unsigned select_end_row = textbox->select_end_row;
  const unsigned select_end_col = textbox->select_end_col;
  const unsigned scroll_y = textbox->scroll_y;
  const unsigned scroll_x = textbox->scroll_x;
  const unsigned rc = textbox->height/(_FONT_HEIGHT+2);
  unsigned text_x, text_y;
  const unsigned text_ix = window->x+textbox->x+2;
  const unsigned element_y = textbox->y;
  const unsigned monospace = textbox->monospace;
  const unsigned max_x = text_ix+textbox->width;

  if(text_is_selected(textbox))
  {
    if(select_start_row >= scroll_y && select_start_row < scroll_y+rc
       && select_start_col >= scroll_x)
    {
      text_y = window->y+element_y+2+(1+select_start_row-scroll_y)*(_FONT_HEIGHT+2);
      text_x = text_ix + string_width(get_str_pos(textbox->text, select_start_row, scroll_x),
                _FONT_HEIGHT, select_start_col-scroll_x, monospace);
      // draw beginning of selection mark
      if(text_x < max_x)
        draw_selection_mark(text_x, text_y, undraw);
    }
    if(select_end_row >= scroll_y && select_end_row < scroll_y+rc
       && select_end_col >= scroll_x)
    {
      text_y = window->y+element_y+2+(1+select_end_row-scroll_y)*(_FONT_HEIGHT+2);
      text_x = text_ix+
             +string_width(get_str_pos(textbox->text, select_end_row, scroll_x),
                _FONT_HEIGHT, select_end_col-scroll_x, monospace);
      // draw end of selection mark
      if(text_x < max_x)
        draw_selection_mark(text_x, text_y, undraw);
    }
  }
}

inline void window_drawelement_scrollbar(struct prog_window *window, unsigned element)
{
  struct window_element *scrollbar = window->window_grid[element];
  unsigned n, m;

     // update scrollbar state if it is in a program window
      if(window->function_pointer == vpuwindow_program)
      {        
        if(scrollbar->ctype >= 2)
        {
          scrollbar->scroll_x = *(unsigned int*)scrollbar->connect_value;
          if(scrollbar->scroll_x > scrollbar->c)
            scrollbar->scroll_x = scrollbar->c;          
        }
        else
        {
          if(!scrollbar->ctype)
          {
            if(window->window_grid[scrollbar->connect_element]->type == 8)
            {
              n = row_count(window->window_grid[scrollbar->connect_element]->text);
              m = (window->window_grid[scrollbar->connect_element]->height
                / (_FONT_HEIGHT+2));
              if(n < m) scrollbar->c = 0;
              else scrollbar->c = n-m;
            }
            else
              scrollbar->c = window->window_grid[scrollbar->connect_element]->c;            
          }
          scrollbar->scroll_x = *(unsigned int*)scrollbar->connect_value;
          if(scrollbar->scroll_x > scrollbar->c)
            scrollbar->scroll_x = scrollbar->c;
        }
      }
      // if vertical
      if(!(scrollbar->ctype & 1))
      {
        draw_borders(window->x+scrollbar->x, window->y+_FONT_HEIGHT+2+scrollbar->y,
          12, 12, 0);
        draw_rectangle(window->x+scrollbar->x-1, window->y+_FONT_HEIGHT+2+scrollbar->y+13,
          14, scrollbar->width, _SCROLLBAR_BGCOLOR);
        draw_borders(window->x+scrollbar->x, window->y+_FONT_HEIGHT+2+scrollbar->y+14
        +scrollbar->width,
          12, 12, 0);
        // and draw the anchor
        if(scrollbar->c)
          draw_rectangle(window->x+scrollbar->x,
            window->y+_FONT_HEIGHT+2+scrollbar->y+13
            + (unsigned long)scrollbar->scroll_x * (scrollbar->width-12) / scrollbar->c, 12, 12,
            _WINDOW_BGCOLOR);
      }
      // if horizontal
      else
      {
        draw_rectangle(window->x+scrollbar->x+13, window->y+_FONT_HEIGHT+2+scrollbar->y-1,
          scrollbar->width, 14, _SCROLLBAR_BGCOLOR);                    
        if(scrollbar->c)
          draw_rectangle(window->x+scrollbar->x+13
          + (unsigned long)scrollbar->scroll_x * (scrollbar->width-12) / scrollbar->c,
            window->y+_FONT_HEIGHT+2+scrollbar->y, 12, 12,
            _WINDOW_BGCOLOR);
        draw_borders(window->x+scrollbar->x, window->y+_FONT_HEIGHT+2+scrollbar->y,
          12, 12, 0);
        draw_borders(window->x+scrollbar->x+14+scrollbar->width, window->y+_FONT_HEIGHT+2+scrollbar->y,
          12, 12, 0);
      }
}

#define CHECKBOX_SIZE _FONT_HEIGHT-1

inline void window_drawelement_checkbox(struct prog_window *window, unsigned element)
{
  struct window_element *elem = window->window_grid[element];
  int x = window->x+elem->x;
  int y = window->y+elem->y+_FONT_HEIGHT+2;
  
  draw_borders(x, y,
    CHECKBOX_SIZE, CHECKBOX_SIZE, 1);

  draw_rectangle(x, y,
    CHECKBOX_SIZE, CHECKBOX_SIZE, _TEXTBOX_BGCOLOR);

  if(elem->selectc)
    draw_rectangle(x+1, y+1,
      CHECKBOX_SIZE-2, CHECKBOX_SIZE-2, _TEXTBOX_TEXTCOLOR);  

  gprint_text(x+CHECKBOX_SIZE+3, y,
    elem->text, elem->color, _FONT_HEIGHT, window->x+window->width, elem->monospace);  
}

inline void window_drawelement_radiobutton(struct prog_window *window, unsigned element)
{
  struct window_element *elem = window->window_grid[element];
  int x = (CHECKBOX_SIZE>>1)+window->x+elem->x;
  int y = (CHECKBOX_SIZE>>1)+window->y+elem->y+_FONT_HEIGHT+2;
  
  draw_circle_border(x, y,
    CHECKBOX_SIZE>>1, 1);

  draw_filled_circle(x, y,
    CHECKBOX_SIZE>>1, _TEXTBOX_BGCOLOR);

  if(elem->selectc)
    draw_filled_circle(x, y,
      (CHECKBOX_SIZE-2)>>1, _TEXTBOX_TEXTCOLOR);

  x += CHECKBOX_SIZE;
  y -= CHECKBOX_SIZE>>1;

  gprint_text(x, y,
    elem->text, elem->color, _FONT_HEIGHT, window->x+window->width, elem->monospace);  
}

void window_drawelement(struct prog_window *window, unsigned int element)
{
  struct window_element *elem = window->window_grid[element];
    
  switch(elem->type)
  {
    // only text or image
    case 0:
      window_drawelement_text(window, element);      
    return;
    // button
    case 1:
      window_drawelement_button(window, element);
    return;
    // icon
    case 5:
      draw_borders(window->x+elem->x, window->y+_FONT_HEIGHT+2+elem->y, _WINDOW_ICONS_MARGIN-2, 39, 0);
      draw_icon(window->x+elem->x, window->y+_FONT_HEIGHT+2+elem->y, elem->bitmap, elem->text, _WINDOW_BASICTEXTCOLOR);
    return;
    // textbox
    case 2:
      window_drawelement_textbox(window, element);
    return;
    // list or multi-line textbox
    case 3:
      window_drawelement_list(window, element);
    return;
    // scrollbar
    case 4:
      window_drawelement_scrollbar(window, element);
    return;
    case 6:
      window_drawelement_checkbox(window, element);
    return;
    case 7:
      window_drawelement_radiobutton(window, element);
    return;
    case 8:
      window_drawelement_mltext(window, element);
    return;
  }
}

/*
Draws a window
If active != 0, the title bar is drawn blue, else it is drawn grey
If update == 0, draws whole window, if 1, draws only the contents,
if 2, draws only the titlebar, if 3, draws only the content and doesn't update mouse cursor's background
*/

void draw_window(struct prog_window *window, unsigned active, unsigned update)
{
  int scrrow = 0;
  unsigned int n;
  void *ptr = draw_window_box;

  window->redraw = 0;

  if(window->vt) return;
  if(!window->fullscreen)
  {
    scrrow = window->y+_FONT_HEIGHT+2;
    if(_VIDEO_MODE != video)
      restore_videomode();
  }
  else
  {
    if(_VIDEO_MODE != window->scrmode)
      video_mode(window->scrmode);
    window->x=0;
    window->y=0;
  }

  if(!update || update == 2)
  {
/*    if(window->maximized && windows[active_window] != window)
    {
      scrrow++;
      goto notitlebar;
    }*/
    // draw title bar
    if(active)
    {
      draw_window_box = window;
      draw_rectangle(window->x, window->y, window->width, _FONT_HEIGHT+2, _TITLE_COLOR);
    }
    else
      draw_rectangle(window->x, window->y, window->width, _FONT_HEIGHT+2, _TITLE_COLOR_IA);
    gprint_text(window->x+_FONT_HEIGHT+4, window->y+1, window->title, _TITLE_TEXTCOLOR, _FONT_HEIGHT, window->x+window->width-24, 0);        

    // topright buttons
    if(window->resizable)
    {
      draw_borders(window->x+window->width-(_FONT_HEIGHT), window->y+2, _FONT_HEIGHT-2, _FONT_HEIGHT-2, 0);
      draw_rectangle(window->x+window->width-(_FONT_HEIGHT), window->y+2, _FONT_HEIGHT-2, _FONT_HEIGHT-2, _WINDOW_BGCOLOR);
      draw_line(window->x+window->width-(_FONT_HEIGHT)+1, window->y+3, window->x+window->width-4, window->y+3, 0x00);
    }
    if(window->minimizable)
    {
       draw_borders(window->x+window->width-2*(_FONT_HEIGHT+1)+1, window->y+2, _FONT_HEIGHT-2, _FONT_HEIGHT-2, 0);
       draw_rectangle(window->x+window->width-2*(_FONT_HEIGHT+1)+1, window->y+2, _FONT_HEIGHT-2, _FONT_HEIGHT-2, _WINDOW_BGCOLOR);
       draw_line(window->x+window->width-2*_FONT_HEIGHT, window->y+_FONT_HEIGHT-2, window->x+window->width-_FONT_HEIGHT-5, window->y+_FONT_HEIGHT-2, 0x00);
    }
      
    // topleft corner dropdown menu button
    draw_borders(window->x+2, window->y+2, _FONT_HEIGHT-2, _FONT_HEIGHT-2, 0);
    draw_rectangle_px(window->x+2, window->y+2, _FONT_HEIGHT-2, _FONT_HEIGHT-2, _WINDOW_BGCOLOR);
    draw_line(window->x+3, window->y+(_FONT_HEIGHT>>1), window->x+_FONT_HEIGHT-2, window->y+(_FONT_HEIGHT>>1), 0x00);
    draw_line(window->x+3, window->y+(_FONT_HEIGHT>>1)+1, window->x+_FONT_HEIGHT-2, window->y+(_FONT_HEIGHT>>1)+1, 0x00);

    draw_window_box = ptr;

    //notitlebar:
    if(update)
      return;    
  }

  if(window == windows[active_window])
    draw_window_box = window;

  draw_rectangle(window->x, scrrow, window->width, window->height, _WINDOW_BGCOLOR);

  for(n=window->element_count;n--;)
  {
    window_drawelement(window, n);
  }

  if(!update)
    if(!window->maximized)
      draw_borders(window->x, window->y, window->width, window->height+_FONT_HEIGHT+2, 0);

  draw_window_box = ptr;
}

static void minimize_window(struct prog_window *window)
{
  unsigned int n=0;
  window->minimized=1;
  if(window_count >= 2)
    set_active_window(window_visibility_order[window_count-2], 0);
  else
    set_active_window(-1, 0);
  if(window->maximized || window->fullscreen)
    screen_redraw = 1;
  else
  {
//    draw_mouse_cursor(2);    
    draw_window_box = window;
    draw_screen();
    draw_window_box = 0;
  }
}

static void unminimize_window(unsigned int window)
{
  windows[window]->minimized=0;

  draw_taskbar();  
  window = set_active_window(window, 1);
}

static void maximize_window(struct prog_window *window)
{
  window->maximized=1;
  
  window->x2 = window->x;
  window->y2 = window->y;
  window->w2 = window->width;
  window->h2 = window->height;

  window->x=0;
  window->y=_FONT_HEIGHT+2;
  window->width=_RES_X;
  window->height=_RES_Y-((2+_FONT_HEIGHT)<<1);

  screen_redraw = 1;

//  draw_screen();
//  draw_window(window, 1, 0);
}

static void unmaximize_window(struct prog_window *window)
{
  window->maximized=0;

  window->x = window->x2;
  window->y = window->y2;
  window->width = window->w2;
  window->height = window->h2;

  update_titlebar(window);
}

void window_grid_delelement(struct prog_window *window, unsigned n)
{
  if(window->window_grid[n]->items)
  {
    while(window->window_grid[n]->c--)
      free(window->window_grid[n]->items[window->window_grid[n]->c]);
    free(window->window_grid[n]->items);
  }
/*  else if(window->window_grid[n]->fitems)
  {
    while(window->window_grid[n]->c--)
      _ffree(window->window_grid[n]->fitems[window->window_grid[n]->c]);
    _ffree(window->window_grid[n]->fitems);
  }    */
  if(window->window_grid[n]->type != 8)
    free(window->window_grid[n]->text);
  free(window->window_grid[n]->bitmap);
  free(window->window_grid[n]);

  if(--window->element_count)
  {
    memmove(&window->window_grid[n], &window->window_grid[n+1], (window->element_count-n)*sizeof(struct window_element*));
    window->window_grid = realloc(window->window_grid, window->element_count*sizeof(struct window_element*));
  }
  else
    free(window->window_grid);
}

static void free_window_dropmenu(struct dropmenu *menu)
{
  struct dropmenu *items;
  void *ptr;

  if(menu == desktop_menu)
    return;

  while(menu)
  {
    if(items = menu->items)
      while(items)
      {
        ptr = items->next;
        free(items->label);
        free(items);
        items = ptr;
      }
    ptr = menu;
    free(menu->label);
    menu = menu->next;
    free(ptr);
  }
}

void del_window(struct prog_window *window)
{
  int n,m;
  int t = window->minimized;

  if(window->vt) return;

  if(window->on_close)
    (*window->on_close)(window);

  if(window->fullscreen)
  {
    if(_VIDEO_MODE != video)
      restore_videomode();
  }
  // hide window and draw screen
  else if(!window->fullscreen && !window->maximized)
    draw_window_box = window;
  window->hidden = 1;
//  window->fullscreen = window->maximized = 0;
  draw_screen();
  draw_window_box = 0;
    
  // free all mallocs
  free(window->hitbox);
  free(window->menus.upleft);
  free_window_dropmenu(window->menus.menubar);
  while(window->element_count)
    window_grid_delelement(window, 0);
  
  free(window->icon);
  free(window->function_status);

  free(window->title);

  free(window);


  // clean some memory
  if(--window_count)
  {
    for(n=0;n<=window_count;n++)
      if(windows[n] == window) break; // n == window id
    for(m=0;m<=window_count;m++)
    {
      if(window_visibility_order[m] == n)
        break;
      if(window_visibility_order[m] > n)
        window_visibility_order[m]--;        
    }
    for(;m<window_count;m++)
    {
      window_visibility_order[m] = window_visibility_order[m+1];
      if(window_visibility_order[m] > n)
        window_visibility_order[m]--;            
    }    
    memmovel(&windows[n], &windows[n+1], (window_count - n)*sizeof(struct prog_window **));
    windows = realloc(windows, window_count*sizeof(struct prog_window **));
    window_visibility_order = realloc(window_visibility_order, window_count*sizeof(int));

    active_window = -1;
    n = set_active_window(window_visibility_order[window_count-1], 0);
    if(n >= 0)
      draw_window(running_window = windows[n], 1, 2);
  }
  else
  {
    free(windows);
    free(window_visibility_order);
    set_active_window(-1, 0);      
  }
  if(t) draw_taskbar();
}

/*
Creates a new window
The last argument is a pointer to a function that is called in a loop if the
window is visible
*/

struct prog_window *new_window(int x, int y, unsigned int width, unsigned int height, unsigned char *title, unsigned resizable, unsigned minimizable, unsigned char *icon, unsigned (*function_pointer)(struct prog_window*, unsigned, int*))
{
  struct prog_window *retval = calloc(1, sizeof(struct prog_window));
  FILE *fp;
  void *new_pointer;
  void *new_pointer2;
  unsigned window_id;

  if(!retval)
  {
    nomemory:
    nomemory();
    return 0;
  }

  if(x == -1)
  {
    retval->x = window_x;
    window_x += _FONT_HEIGHT+2;
    if(window_x>_RES_X-width)
      window_x = 40;
  }
  else
    retval->x = x;
  if(y == -1)
  {
    retval->y = window_y;
    if(window_y>_RES_Y-(height+_FONT_HEIGHT+2))
      window_y = 0;
    window_y += _FONT_HEIGHT+2;
  }
  else
    retval->y = y;
  retval->width = width;
  retval->height = height;
  retval->window_grid=0;
  retval->element_count=0;
  retval->hitbox_count=0;
  retval->hitbox=0;
  retval->type=1;
  retval->active_element=-1;
  retval->function_pointer = function_pointer;

  // if icon is null pointer, no icon, otherwise load icon
  if(!icon) retval->icon=0;
  else
  {
    if(!(retval->icon = malloc(100 * sizeof(char))))
    {
      free(retval);
      goto nomemory;
    }
    if(!(fp=fopen(icon, "rb")))
    {
      free(retval->icon);
      retval->icon=0;
    }
    else
    {
      fread(retval->icon, sizeof(char), 100, fp);
      fclose(fp);
    }
  }
  
  if(!(retval->title = malloc((1+strlen(title)) * sizeof(char))))
  {
    free(retval->icon);
    free(retval);
    goto nomemory;
  }
  strcpy(retval->title, title);

  retval->minimizable = minimizable;
  retval->resizable = resizable;
  retval->maximized = 0;
  retval->minimized = 0;
//  retval->menubar = menubar;

  // up-left corner dropdown menu
//  retval->menus.upleft_count=2+(resizable*2)+(minimizable);
//  if(resizable) retval->menus.upleft_count+=2;
//  if(minimizable) retval->menus.upleft_count++;
  if(!(retval->menus.upleft = malloc((2+(resizable*2)+(minimizable))* sizeof(char *))))
  {
    free(retval->title);
    free(retval->icon);
    free(retval);
    goto nomemory;
  }
//  retval->menus.upleft_count=0;  
  retval->menus.upleft[retval->menus.upleft_count++] = upleft_move;
  retval->menus.upleft[retval->menus.upleft_count++] = upleft_close;
  if(resizable)
  {
    retval->menus.upleft[retval->menus.upleft_count++] = upleft_maximize;
    retval->menus.upleft[retval->menus.upleft_count++] = upleft_resize;
  }
  if(minimizable)
    retval->menus.upleft[retval->menus.upleft_count++] = upleft_minimize;
  
  // add this window to the window arrays
  if(!(window_id = window_count++))
  {
    new_pointer = calloc(window_count, sizeof(struct prog_window *));
    new_pointer2 = calloc(window_count, sizeof(int));
  }
  else
  {
    new_pointer = realloc(windows, window_count*sizeof(struct prog_window *));
    new_pointer2 = realloc(window_visibility_order, window_count*sizeof(int));
  }
  if(new_pointer && new_pointer2)
  {
    windows = new_pointer;
    window_visibility_order = new_pointer2;
  }
  else
  {
    free(retval->title);
    free(retval->icon);
    free(retval);
    window_count--;
    goto nomemory;
  }
  windows[window_id] = retval;
  window_visibility_order[window_id] = window_id;

  running_window = retval;
  set_active_window(window_id, 1);

  return retval;
}



void window_recalc_hitboxes(struct prog_window *window)
{
  unsigned int n;
  unsigned int m = 0;
  for(n=0;n<window->element_count;n++)
  {
    if(window->window_grid[n]->type)
    {
      window->hitbox[m].x0 = window->window_grid[n]->x;
      window->hitbox[m].y0 = window->window_grid[n]->y+_FONT_HEIGHT+2;
      if(window->window_grid[n]->type != 4)
      {
        window->hitbox[m].x1 = window->window_grid[n]->x+window->window_grid[n]->width;
        window->hitbox[m].y1 = window->window_grid[n]->y+window->window_grid[n]->height+_FONT_HEIGHT+2;        
      }
      else
      {
        if(!window->window_grid[n]->ctype)
        {
          window->hitbox[m].x1 = window->window_grid[n]->x+12;
          window->hitbox[m].y1 = window->window_grid[n]->y+_FONT_HEIGHT+28+window->window_grid[n]->width;
        }
        else
        {
          window->hitbox[m].x1 = window->window_grid[n]->x+26+window->window_grid[n]->width;
          window->hitbox[m].y1 = window->window_grid[n]->y+_FONT_HEIGHT+14;
        }
      }

      m++;
    }
  }
}

static int window_add_hitbox(struct prog_window *window, unsigned int x, unsigned int y, unsigned int width, unsigned int height)
{
  void *new_pointer;

  window->hitbox_count++;
  if(!window->hitbox)
  {
    if(!(window->hitbox = malloc(sizeof(struct hitbox))))
    {
      window->hitbox_count--;            
      return 0;
    }
  }
  else
    if(!(new_pointer = realloc(window->hitbox, window->hitbox_count*sizeof(struct hitbox))))
    {
      window->hitbox_count--;      
      return 0;
    }
    else window->hitbox = (struct hitbox*)new_pointer;

  // create a hitbox for the mouse
  window->hitbox[window->hitbox_count-1].x0 = x;
  window->hitbox[window->hitbox_count-1].y0 = y+_FONT_HEIGHT+2;
  window->hitbox[window->hitbox_count-1].x1 = x+width;
  window->hitbox[window->hitbox_count-1].y1 = y+_FONT_HEIGHT+2+height;

  window->hitbox[window->hitbox_count-1].element = window->element_count-1;

  return 1;
}

static int window_grid_addelement(struct prog_window *window, struct window_element *element)
{
  void *new_pointer;
  window->element_count++;

  if(!window->window_grid)
  {
    if(!(window->window_grid = malloc(sizeof(struct window_element*))))
    {
      window->element_count--;
      return 0;
    }
  }
  else
    if(!(new_pointer = realloc(window->window_grid, window->element_count*sizeof(struct window_element*))))
    {
      window->element_count--;      
      return 0;
    }
    else window->window_grid = (struct window_element**)new_pointer;

  window->window_grid[window->element_count-1] = element;    
  return 1;
}

/*
Adds a scrollbar to window

*/

struct window_element *window_scrollbar(struct prog_window *window, unsigned int x, unsigned int y, unsigned type, unsigned int length, unsigned int count, int element, int *value)
{
  struct window_element *retval = calloc(1, sizeof(struct window_element));
//  void *new_pointer;
  if(!retval)
    return 0;

  retval->type=4;
  retval->ctype=type;
  retval->x = x;
  retval->y = y;
  retval->width = length;
  retval->c = count;
  retval->connect_element = element;
  retval->connect_value = value;

  if(!window_grid_addelement(window, retval) || 
    (!type && !window_add_hitbox(window, x, y, 12, 26+length)) ||
    (type && !window_add_hitbox(window, x, y, 26+length, 14)))
  {
    free(retval);
    return 0;
  }
  
  return retval;
}

struct window_element *window_icon(struct prog_window *window, unsigned int x, unsigned int y, unsigned char *text, char *icon_file)
{
  struct window_element *retval = calloc(1, sizeof(struct window_element));
  if(!retval)
    return 0;

  retval->type=5;
  retval->width = _WINDOW_ICONS_MARGIN-2;
  retval->height = 39;
  retval->x = x;
  retval->y = y;

  if(!(retval->text = malloc((1+strlen(text))*sizeof(char))))
  {
    free(retval);
    return 0;
  }
  strcpy(retval->text, text);

  if(!load_icon_bitmap(&retval->bitmap, icon_file))
  {
    free(retval->text);
    free(retval);
    return 0;
  }

  if(!window_grid_addelement(window, retval) ||
     !window_add_hitbox(window, x, y, _WINDOW_ICONS_MARGIN-2, 39))
  {
    free(retval->text);
    free(retval->bitmap);    
    free(retval);
    return 0;
  }


  return retval;  
}

struct window_element *window_list(struct prog_window *window, unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char **items, unsigned int len)
{
  struct window_element *retval = calloc(1, sizeof(struct window_element));
  unsigned char **itemp;
  if(!retval)
    return 0;

  retval->type=3;
  retval->x = x;
  retval->y = y;
  retval->width = width;
  retval->height = height;
  retval->c = len;
  retval->connect_element = -1;
  retval->selectc = -1;
  retval->active = 1;
  if(items)
  {
    if(!(itemp = retval->items = calloc(len,sizeof(char*))))
    {
      free(retval);
      return 0;
    }
    while(len--)
      *itemp++ = *items++;
//      itemp[len] = items[len];
  }
  else retval->items=0;

  if(!window_grid_addelement(window, retval) ||
     !window_add_hitbox(window, x, y, width, height))
  {
    free(retval->items);
    free(retval);
    return 0;
  }

  return retval;
}

void window_list_selectrow(struct prog_window *window, unsigned element, unsigned row)
{
  struct window_element *win_element = window->window_grid[element];
  unsigned scroll_y = win_element->scroll_y;
  unsigned scroll_x = win_element->scroll_x;
  unsigned height = win_element->height;
  unsigned oldrow = win_element->selectc;
  unsigned element_x = window->x + win_element->x;
  unsigned element_y = window->y + _FONT_HEIGHT + 2 + win_element->y;
  char *text_ptr;
  win_element->selectc = row;

  if(oldrow >= scroll_y && oldrow < scroll_y + (height / (_FONT_HEIGHT+2)))
  {
    if(scroll_x < strlen(win_element->items[oldrow]))
      text_ptr = &win_element->items[oldrow][scroll_x];
    else
      text_ptr = &null_byte;
    draw_label(element_x, element_y+(oldrow-scroll_y)*(_FONT_HEIGHT+2),
      win_element->width, _TEXTBOX_BGCOLOR, _TEXTBOX_TEXTCOLOR,
      text_ptr, win_element->monospace);
  }
  if(row >= scroll_y && row < scroll_y + (height / (_FONT_HEIGHT+2)))
  {
    if(scroll_x < strlen(win_element->items[row]))
      text_ptr = &win_element->items[row][scroll_x];
    else
      text_ptr = &null_byte;
    draw_label(element_x, element_y+(row-scroll_y)*(_FONT_HEIGHT+2),
      win_element->width, _ATEXT_BGCOLOR, _TEXTBOX_ATEXTCOLOR,
      text_ptr, win_element->monospace);
  }
}

struct window_element *window_ml_textbox(struct prog_window *window, unsigned x, unsigned y, unsigned width, unsigned height, unsigned char *content, unsigned len)
{
  struct window_element *retval = calloc(1, sizeof(struct window_element));
  if(!retval) return 0;

  retval->type = 8;
  retval->x = x;
  retval->y = y;
  retval->width = width;
  retval->height = height;
  retval->c = len;
  retval->connect_element = -1;
/*  retval->selectc = 0;
  retval->cursor_x = 0;*/
  retval->text = content;

  if(!window_grid_addelement(window, retval) ||
     !window_add_hitbox(window, x, y, width, height))
  {
    free(retval);
    return 0;
  }

  return retval;
}

struct window_element *window_textbox(struct prog_window *window, unsigned int x, unsigned int y, unsigned int width, unsigned char *text, unsigned int len)
{
  struct window_element *retval = calloc(1, sizeof(struct window_element));
  unsigned char *textp;
  if(!retval)
    return 0;

  retval->type=2;
  retval->ctype=0;
  retval->x = x;
  retval->y = y;
  retval->width = width;
  retval->height=_FONT_HEIGHT+2;
  retval->c=len;
  retval->selectc = -1;
  if(len)
  {
    if(!(textp = retval->text = calloc(len+1,sizeof(char))))
    {
      free(retval);
      return 0;
    }
  }
  if(text)
  {
    if(!retval->text)
      if(!(textp = retval->text = calloc(len+1,sizeof(char))))
      {
        free(retval);
        return 0;
      }
    while(len-- && *text)
      *textp++ = *text++;
  }

  if(!window_grid_addelement(window, retval) ||
     !window_add_hitbox(window, x, y, width, _FONT_HEIGHT+2))
  {
    free(retval->text);      
    free(retval);
    return 0;
  }

  return retval;
}

struct window_element *window_button(struct prog_window *window, unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char *text, unsigned color)
{
  struct window_element *retval = calloc(1, sizeof(struct window_element));
  if(!retval)
    return 0;

  retval->type=1;
  retval->x = x;
  retval->y = y;
  retval->color = color;
  retval->width = width;
  retval->height = height;
  if(!(retval->text = malloc((1+strlen(text))*sizeof(char))))
  {
    free(retval);
    return 0;
  }

  strcpy(retval->text, text);

  if(!window_grid_addelement(window, retval) ||
     !window_add_hitbox(window, x, y, width, height))
  {
    free(retval->text);      
    free(retval);
    return 0;
  }
  
  return retval;
}

struct window_element *window_checkbox(struct prog_window *window, unsigned x, unsigned y, char *text, unsigned color)
{
  struct window_element *retval = calloc(1, sizeof(struct window_element));
  if(!retval)
  return 0;

  retval->type = 6;
  retval->x = x;
  retval->y = y;
  retval->color = color;

  if(!(retval->text = malloc((1+strlen(text))*sizeof(char))))
  {
    free(retval);
    return 0;
  }

  strcpy(retval->text, text);

  if(!window_grid_addelement(window, retval) ||
     !window_add_hitbox(window, x, y, CHECKBOX_SIZE, CHECKBOX_SIZE))
  {
    free(retval->text);
    free(retval);
    return 0;
  }

  return retval;
}

void window_radiobutton_select(struct prog_window *window, struct window_element *radiobutton)
{
  struct window_element *ptr = radiobutton->connect_value;
  int n;
  
  radiobutton->selectc = 1;

  while(ptr)
  {
    if(ptr->selectc)
    {
      ptr->selectc = 0;
      for(n=window->element_count;n--;)
        if(window->window_grid[n] == ptr)
        {
          window_drawelement(window, n);
          break;
        }
    }
    ptr = ptr->connect_value;
  }

  while(n >= 0)
  {
    for(n=window->element_count;n--;)
      if(window->window_grid[n]->type == 7 && window->window_grid[n]->connect_value == radiobutton)
      {
        radiobutton = window->window_grid[n];
        if(radiobutton->selectc)
        {
          radiobutton->selectc = 0;
          window_drawelement(window, n);
        }
        break;
      }
  }
}

struct window_element *window_radiobutton(struct prog_window *window, unsigned x, unsigned y, char *text, unsigned color, struct window_element *prev_element)
{
  struct window_element *retval = calloc(1, sizeof(struct window_element));
  if(!retval) return 0;

  retval->type = 7;
  retval->x = x;
  retval->y = y;
  retval->color = color;

  if(!(retval->text = malloc((1+strlen(text))*sizeof(char))))
  {
    free(retval);
    return 0;
  }

  strcpy(retval->text, text);

  if(prev_element && prev_element->type == 7 && !prev_element->connect_value)
    prev_element->connect_value = retval;
  else retval->selectc = 1;

  if(!window_grid_addelement(window, retval) ||
     !window_add_hitbox(window, x, y, CHECKBOX_SIZE, CHECKBOX_SIZE))
  {
    free(retval->text);
    free(retval);
    return 0;
  }

  return retval;
}

struct window_element *window_text(struct prog_window *window, unsigned int x, unsigned int y, unsigned char *text, unsigned char color)
{
  struct window_element *retval = calloc(1, sizeof(struct window_element));
  if(!retval) return 0;

  retval->x = x;
  retval->y = y;
  retval->color = color;
  if(text)
  {
    if(!(retval->text = malloc(1+strlen(text)*sizeof(char))))
    {
      free(retval);
      return 0;
    }

  // copy the string to the struct
    strcpy(retval->text, text);    
  }
  
  if(!window_grid_addelement(window, retval))
  {
    free(retval->text);      
    free(retval);
    return 0;
  }


  return retval;
}

struct window_element *window_image(struct prog_window *window, unsigned int x, unsigned int y, unsigned int width, unsigned int height, char *filename)
{
  struct window_element *retval = calloc(1, sizeof(struct window_element));
  if(!retval) return 0;

  retval->x = x;
  retval->y = y;
  retval->ctype = 1;
  retval->width = height;
  retval->width = width;
  if(!(retval->text = malloc(strlen(filename)+1)))
  {
    free(retval);
    return 0;
  }
  strcpy(retval->text, filename);

  if(!window_grid_addelement(window, retval))
  {
    free(retval->text);      
    free(retval);
    return 0;
  }
  return retval;
}

static void set_window_resized(struct prog_window *window)
{
  window->resized = 1;
  window->redraw = 4;
}

/*
This function is used to make a dialog window
The return value if 0 if user presses enter or ok
*/
//struct prog_window *new_window(int x, int y, unsigned int width, unsigned int height, unsigned char *title, unsigned resizable, unsigned minimizable, unsigned menubar, unsigned char *icon, int (*function_pointer)(struct prog_window*, int, int*))
int dialog(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned type, unsigned help_button, unsigned sound, unsigned char *message, unsigned char *title, unsigned char *helptext)
{
  struct prog_window *window;
  unsigned a;
  unsigned int n;

  reset_vpu_interrupt_handlers();  

//  draw_mouse_cursor(2);

  window = new_window(x, y, width, height, title, 0, 0, 0, 0);

  a = 0;

  window->type = 0;

  // show message  
  if(!(window_text(window, 10, 10, message, _WINDOW_BASICTEXTCOLOR)))
  {
    nomemory2:
    del_window(window);
    set_vpu_interrupt_handlers();    
    return -200;
  }
  // create buttons
  n = 0;
  switch(type)
  {
    // type 0: only OK    
    case 0:case 1:
      if(!(window_button(window, 10, 30, 50, _FONT_HEIGHT+2, dialog_ok, _WINDOW_BASICTEXTCOLOR)))
        goto nomemory2;
    if(!type)break;
    // type 1: ok - cancel    
      if(!(window_button(window, 10+(++n)*60, 30, 50, _FONT_HEIGHT+2, dialog_cancel, _WINDOW_BASICTEXTCOLOR)))
        goto nomemory2;
    // type 2: yes - no
    break;
    case 2: case 3:
      if(!(window_button(window, 10, 30, 50, _FONT_HEIGHT+2, dialog_yes, _WINDOW_BASICTEXTCOLOR)))
        goto nomemory2;
      if(!(window_button(window, 10+(++n)*60, 30, 50, _FONT_HEIGHT+2, dialog_no, _WINDOW_BASICTEXTCOLOR)))
        goto nomemory2;
    if(type==2) break;
    // type 3: yes - no - cancel
      if(!(window_button(window, 10+(++n)*60, 30, 50, _FONT_HEIGHT+2, dialog_cancel, _WINDOW_BASICTEXTCOLOR)))
        goto nomemory2;
    break;        
  }
  
  if(help_button)
    if(!(window_button(window, 10+(++n)*60, 30, 60, _FONT_HEIGHT+2, dialog_help, _WINDOW_BASICTEXTCOLOR)))
      goto nomemory2;

  if(!tty)
    draw_window(window, 1, 0);
  else
  {
    new_tty = 0;
    switch_tty();
  }

  if(sound)
    make_sound(sound);

  while(1)
  {

    call_drv_screenupdate();
    a = ~WM_WINPROGACT;
    if(mouse)
    {
      mouse_set_normal_cursor();
      a = mouse_handler(window);
    }
    if(kbhit())
    {
      // user presses enter      
      if((a = getch()) == 13)
        a = WM_WINPROGACT;
      // user presses esc
      else if(a == 27)
      {
        a = WM_WINPROGACT+1;
        break;
      }
      else if(!a)
      {
        // user presses F1
        if((a = getch()) == 59 && help_button)
          goto helpdialog;
        // alt+m
        else if(a==50)
//          a = window_menu(window, 0);
          menu_selected = 0;
      }
    }
//    while(menu_selected >= 0)
    if(menu_selected >= 0)
    {
      a = dropmenu_handler(window);
    }
    
    if(help_button && a == WM_WINPROGACT+window->hitbox_count-1)
    {
      helpdialog:
      dialog(50, 50, 250, 50, 0, 0, 0, helptext, infotitle, 0);
      a = ~WM_WINPROGACT;
    }
    else
      switch(a)
      {
        case 1: window_kbdmove(window); /*draw_mouse_cursor(1);*/ break;
        case 2: case WM_WINPROGACT: case WM_WINPROGACT+1: case WM_WINPROGACT+2: goto endwhile;
      }
    draw_mouse_cursor();
    halt();
//    hide_mouse_cursor();
  }
  endwhile:
  del_window(window);

//  draw_mouse_cursor(1);

  set_vpu_interrupt_handlers();    

  return a & ~WM_WINPROGACT;
}
