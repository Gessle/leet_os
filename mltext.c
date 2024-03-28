static void copy_text(struct window_element *element)
{
  char *start = get_str_pos(element->text, element->select_start_row, element->select_start_col);
  char *end = get_str_pos(element->text, element->select_end_row, element->select_end_col);
  void *ptr;
  unsigned len;

  if(clipboard.content) free(clipboard.content);

  if(start > end)
  {
    ptr = start;
    start = end;
    end = ptr;
  }
  len = end-start;

  clipboard.content = malloc(len*sizeof(char));
  if(!clipboard.content) return;
  memcpy(clipboard.content, start, len);
  clipboard.content_type = 0;
  clipboard.len = len;
}

static void paste_text(struct window_element *element)
{
  char *dstpos = get_str_pos(element->text, element->selectc, element->cursor_x);

  if(!clipboard.content) return;

  if(element->c <= strlen(element->text) + clipboard.len)
  {
    show_error_msg(outofmemory);
    return;
  }

  memmove(dstpos+clipboard.len, dstpos, strlen(dstpos)+1);
  memcpy(dstpos, clipboard.content, clipboard.len);
}

static void cut_text(struct window_element *element)
{
  char *dstpos = get_str_pos(element->text, element->select_start_row, element->select_start_col);
  char *srcpos = get_str_pos(element->text, element->select_end_row, element->select_end_col);
  void *ptr;

  if(dstpos > srcpos)
  {
    ptr = srcpos;
    srcpos = dstpos;
    dstpos = ptr;
    element->select_start_row = element->select_end_row;
    element->select_start_col = element->select_end_col;
  }
  element->selectc = element->select_end_row = element->select_start_row;
  element->cursor_x = element->select_end_col = element->select_start_col;

  memmove(dstpos, srcpos, strlen(srcpos)+(srcpos-dstpos));
}

static unsigned row_len(char *str)
{
  char *ptr;
  if(ptr = strchr(str, '\n'))
    return ptr - str;
  return strlen(str); 
}

// update scrollbars that show the scroll status of this element
static void updatescroll(struct prog_window *window, unsigned element)
{
  unsigned n = window->element_count;

  while(n--)
  {
    if(window->window_grid[n]->type == 4 
       && window->window_grid[n]->connect_element == element)
      window_drawelement(window, n);
  }
}

static void draw_end_line(unsigned char *curptr, unsigned max_x, struct window_element *element)
{
  unsigned len = row_len(curptr);
  draw_rectangle(caret_screenx+1, caret_screeny, max_x-caret_screenx-1,
                 _FONT_HEIGHT, _TEXTBOX_ABGCOLOR);
  if(len)
    cprint_text(caret_screenx+1, caret_screeny, curptr, _TEXTBOX_TEXTCOLOR,
      _FONT_HEIGHT, max_x, len, element->monospace);
}

static unsigned char scroll_to_cursor(struct prog_window *window, unsigned textbox)
{
  unsigned char draw = 0;
  struct window_element *element = window->window_grid[textbox];
  unsigned element_wchars = element->width >> 3;

  if(element->cursor_x < element->scroll_x)
  {
    draw++;
    element->scroll_x = element->cursor_x;
  }
  else if(caret_screenx >= window->x+element->x+element->width-MONOSPACE_WIDTH)
  {
    draw++;
    element->scroll_x = element->cursor_x - (element->width>>3) + 1;
  }
  if(element->selectc < element->scroll_y)
  {
    draw++;
    element->scroll_y = element->selectc;
  }
  else if(element->selectc >= element->scroll_y + element->height/(_FONT_HEIGHT+2))
  {
    draw++;
    element->scroll_y = element->selectc - element->height/(_FONT_HEIGHT+2) + 1;
  }

  if(draw)
  {
    updatescroll(window, textbox);
    window_drawelement(window, textbox);
  }
  return draw;
}

// render rows that come after the caret
static int render_new_rows(struct prog_window *window, unsigned textbox)
{
  unsigned r;
  struct window_element *element = window->window_grid[textbox];
  unsigned char *ptr;
  unsigned max_x = window->x+element->x+element->width;
  unsigned element_rows = element->height/(_FONT_HEIGHT+2);
  unsigned text_y;
  unsigned rl;
  unsigned element_x = window->x+element->x;
  unsigned element_y = window->y+element->y+_FONT_HEIGHT+4;

  if(scroll_to_cursor(window, textbox)) return 1;

  for(r=element->selectc;r<element->scroll_y+element_rows;r++)
  {
    ptr = get_str_pos(element->text, r, element->scroll_x);
    text_y = element_y+(r-element->scroll_y)*(_FONT_HEIGHT+2);
    draw_rectangle(element_x, text_y, element->width,_FONT_HEIGHT, _TEXTBOX_ABGCOLOR);
    if(ptr)  
    {
      if(rl = row_len(ptr))
        cprint_text(element_x+2, text_y, ptr, _TEXTBOX_TEXTCOLOR, _FONT_HEIGHT,
          max_x, rl, element->monospace);   
    }
  }
  return 0;
}

static unsigned edit_ml_textbox(struct prog_window *window, unsigned textbox)
{
  struct window_element *element = window->window_grid[textbox];
  int c;
  unsigned char *text = element->text;
  unsigned char *rowptr = get_str_pos(text, element->selectc, 0);
  unsigned char *curptr = rowptr + element->cursor_x;
  unsigned element_rows = element->height / (_FONT_HEIGHT+2);
  unsigned n;
  unsigned char *ptr;
  unsigned element_x = window->x+element->x;
  unsigned rowh = _FONT_HEIGHT+2;
  unsigned max_x = element_x+element->width;
  unsigned rowlen = row_len(rowptr);
  unsigned currowlen = row_len(curptr);

  if(kbhit())
  {
    screensaver_timer = 0;
    draw_textbox_caret(1);
    scroll_to_cursor(window, textbox);
//    draw_textbox_caret(1);

//    if(keyboard_status.shift_pressed)
    {
      mltext_draw_selection(window, textbox, 1);
    }

    if(!(c = getch()))
      switch(c = getch())
      {
        case 115: // ctrl + left arrow
          if(element->cursor_x)
          {
            n = 0;
            do
            {
              n++;
              curptr--;
              element->cursor_x--;
              if(!element->cursor_x) break;
            }
            while(*curptr != ' ');
            caret_screenx -= string_width(curptr, _FONT_HEIGHT, n, element->monospace);
            break;
          }
        case 75: // left arrow          
          element->cursor_x--;
          if(element->cursor_x == -1)
            if(element->selectc)
              goto up;
            else element->cursor_x = 0;
          else
          {
            caret_screenx -= string_width(curptr-1, _FONT_HEIGHT, 1, element->monospace);
          }
          break;
        case 77: // right arrow
          if(element->cursor_x == rowlen)
          {
            right_endline:
            if(strlen(curptr))
            {           
              element->cursor_x = 0;
              goto down;
            }
          }
          else
          {
            element->cursor_x++;
            caret_screenx += string_width(curptr, _FONT_HEIGHT, 1, element->monospace);
          }
          break;
        case 116: // ctrl + right arrow
          if(element->cursor_x == rowlen) goto right_endline;
          ptr = strchr(curptr, ' ');
          if(ptr)
          {
            n = ptr+1 - curptr;
            if(n > currowlen) goto row_end;
            element->cursor_x += n;
            caret_screenx += string_width(curptr, _FONT_HEIGHT, n, element->monospace);
            break;
          }
        case 79: row_end: // end
          element->cursor_x = rowlen;        
          caret_screenx += string_width(curptr, _FONT_HEIGHT, row_len(curptr), element->monospace);
          break;    
        case 71: // home
          element->cursor_x = 0;
          caret_screenx = element_x+1;
          break;
        case 72:up: // up
          if(element->selectc)
          {
            element->selectc--;
            caret_screeny -= rowh;
          }
          goto row_end_chk;
        case 80:down: // down
          element->selectc++;
          caret_screeny += rowh;
          row_end_chk:
          if(!(rowptr = get_str_pos(text, element->selectc, 0)))
            goto up;
          if(element->cursor_x > (rowlen = row_len(rowptr)))
            element->cursor_x = rowlen;        
          caret_screenx = element_x + 1
                 + string_width(rowptr+element->scroll_x, _FONT_HEIGHT, element->cursor_x-element->scroll_x, element->monospace);
          break;
        case 73: // page up          
          if(!element->selectc) break;
          if(element->selectc > element_rows)
            element->selectc -= element_rows;
          else
          {
            element->selectc = 0;
            caret_screeny = window->y+element->y+_FONT_HEIGHT+4;
          }
          goto up;
        case 81: // page down
          for(n=element_rows-1;n--;)
          {
            if(get_str_pos(text, element->selectc+2, 0))
            {
              element->selectc++;
              caret_screeny += _FONT_HEIGHT+2;
            }
            else break;
          }
          goto down;
        case 83: // del
          if(text_is_selected(element))
          {
            cut:
            cut_text(element);
            window_drawelement(window, textbox);
            return ~WM_WINPROGACT;
          }
          if(*curptr)
          {
            del:
            c = *curptr;
            memmove(curptr, curptr+1, strlen(curptr));
            draw_end_line(curptr, max_x, element);
          }
        break;
      }
    else
      switch(c)
      {
        case 27:
          deactivate_element(window);
        break;
        case 8: // backspace          
          if(text_is_selected(element)) goto cut;
          if(curptr != text)
          {
            if(element->cursor_x)
            {
              curptr--;
              if(element->cursor_x-- == element->scroll_x)
              {
                element->scroll_x--;
                window_drawelement(window, textbox);
              }
              else
                caret_screenx -= string_width(curptr, _FONT_HEIGHT, 1, element->monospace);
              goto del;
            }
            else
            {
              element->cursor_x = row_len(get_str_pos(text, element->selectc-1, 0));
              memmove(curptr-1, curptr, strlen(curptr)+1);             
              c = '\n';
              goto up;
            }
          }
          else goto end;
        break;
        case 3: // ctrl + c
        case 24: // ctrl+x
          copy_text(element);
          if(c == 3) break;        
          goto cut;
        case 22: // ctrl + v
          if(!clipboard.content_type)
          {
            paste_text(element);
            window_drawelement(window, textbox);
            return ~WM_WINPROGACT;
          }
          break;
        case 13: // enter
          c = '\n';
        default:
          if(curptr+1 == text + element->c) break;
          memmove(curptr+1, curptr, strlen(curptr));
          *curptr = c;
          element->cursor_x++;

          draw_end_line(curptr, max_x, element);
          caret_screenx += string_width(curptr, _FONT_HEIGHT, 1, element->monospace);

          curptr++;

          if(c == '\n')
          {
            element->cursor_x = 0;
            goto down;
          }
          else c = 0;
          break;
      }
    if(n = ((c == '\n' && !render_new_rows(window, textbox))
       || (c != '\n' && !scroll_to_cursor(window, textbox))))
    {
      end:if(!keyboard_status.shift_pressed)
      {
        element->select_end_row = element->select_start_row = element->selectc;
        element->select_end_col = element->select_start_col = element->cursor_x;
      }
      else if(c)
      {
        element->select_end_row = element->selectc;
        element->select_end_col = element->cursor_x;
        if(n) mltext_draw_selection(window, textbox, 0);
      }
    }
    else // if the textbox wasn't scrolled and/or lines redrawn...
      goto end;
    caret_visible = 1;
  }

  draw_textbox_caret(0);

  return ~WM_WINPROGACT;
}
