inline void add_char_to_string(char *pos, char chr)
{
  memmove(pos+1, pos, strlen(pos)+1);
  *pos = chr;
}

inline void del_char_from_string(char *pos)
{
  memmove(pos, pos+1, strlen(pos+1)+1);
}

unsigned char *edit_text_ptr = 0;

static void deactivate_element(struct prog_window *window)
{
  unsigned element = window->active_element;
  window->active_element = -1;
  edit_text_ptr = 0;
  window_drawelement(window, element);

//  draw_mouse_cursor(1);           
}

static void activate_next_element(struct prog_window *window)
{
  int element = window->active_element;
  int prev_active_element = element;

  while(1)
  {
    do
      element++;
    while(element < window->element_count
          && window->window_grid[element]->type != 2
          && window->window_grid[element]->type != 8);

    if(element >= window->element_count)
//      return;
      element = -1;
    else break;
    if(element == prev_active_element)
    {
      element = -1;
      break;
    }
  }

  element_active(window, element);

//  draw_mouse_cursor(1);   
}

static unsigned element_args(struct prog_window *window, unsigned element)
{
  unsigned n = window->hitbox_count;

  while(n--)
    if(window->hitbox[n].element == element)
      return n | WM_WINPROGACT;
  
  return 0;
}

static int sltextbox_needs_scrolling(unsigned caretpos, char *ltextp, unsigned element_x, struct window_element *textbox)
{
//  if(caret_screenx <= element_x) return -1;
  if(&edit_text_ptr[caretpos] < ltextp) return -1;
//  if(caretpos > ltextp - textbox->text + (textbox->width/MONOSPACE_WIDTH)) return 1;
//  if(string_width(ltextp, _FONT_HEIGHT, caretpos-(ltextp-textbox->text), textbox->monospace) >= textbox->width-MONOSPACE_WIDTH) return 1;
  if(caret_screenx >= element_x + textbox->width - MONOSPACE_WIDTH) return 1;
  return 0;
}

static unsigned edit_textbox(struct prog_window *window, unsigned textbox)
{
  int c;
  struct text_print texts;
  struct window_element *element = window->window_grid[textbox];  
//  static unsigned char *textp = 0;// = element->text;
  static unsigned char *ltextp;// = textp;
  unsigned textl;
  static int caretpos;
  static unsigned args;
  unsigned element_x = window->x+element->x;
  unsigned element_y = window->y+_FONT_HEIGHT+2+element->y;
  int s;

  if((mouse && mouse->left_pressed) || edit_text_ptr != element->text)
  {
    edit_text_ptr = element->text;
    ltextp = edit_text_ptr;
    args = element_args(window, textbox);

//    draw_mouse_cursor(2);         

    draw_rectangle(element_x, element_y, window->window_grid[textbox]->width, _FONT_HEIGHT+2, _TEXTBOX_ABGCOLOR);    

    if(mouse)
    {
      texts = cprint_text(element_x+2, element_y+2, ltextp, _TEXTBOX_TEXTCOLOR, _FONT_HEIGHT, mouse->pointer_x+8, 0, element->monospace);

      caretpos = texts.text - edit_text_ptr;

      caret_screenx = texts.x-1;
      caret_screeny = element_y+1;

      texts = cprint_text(texts.x, element_y+2, texts.text, _TEXTBOX_TEXTCOLOR, _FONT_HEIGHT, element_x+element->width, 0, element->monospace);
    }
    else
    {
      texts = cprint_text(element_x+2, element_y+2, ltextp, _TEXTBOX_TEXTCOLOR, _FONT_HEIGHT, element_x+element->width, 0, element->monospace);
      caret_screenx = element_x+1;
      caret_screeny = element_y+1;
      caretpos = 0;
    }
    draw_textbox_caret(0);

//    draw_mouse_cursor(1);             

    texts.text += strlen(texts.text);
    textl = texts.text-edit_text_ptr;      
  }

  if(kbhit())
  {
    zero_screensaver_timer();
//    draw_mouse_cursor(2); 
    if(!(c = getch()))
      switch(c = getch())
      {
        case 83: // del
          del_char_from_string(&edit_text_ptr[caretpos]);
          break;
        case 75: // left arrow
          if(caretpos)
          {
            caretpos--;        
            draw_textbox_caret(1);
            caret_screenx -= string_width(&edit_text_ptr[caretpos], _FONT_HEIGHT, 1, element->monospace);
            goto move_caret_1;
          }
          goto end;
        case 77: // right arrow
          if(caretpos < strlen(edit_text_ptr))
          {
            draw_textbox_caret(1);
            caret_screenx += string_width(&edit_text_ptr[caretpos], _FONT_HEIGHT, 1, element->monospace);
            caretpos++;
            move_caret_1:
            caret_visible = 1;
            if(!(s = sltextbox_needs_scrolling(caretpos, ltextp, element_x, element)))
              goto end;
            else goto vscroll;
          }
          goto end;
        case 71: // home
          caretpos=0; ltextp=edit_text_ptr;
          goto redraw;
        case 79:
          caretpos=strlen(edit_text_ptr); if(caretpos>(element->width>>3)) ltextp=&edit_text_ptr[caretpos]-(element->width>>3);
          goto redraw;
      }
    else
    {
      draw_textbox_caret(2);
      switch(c)
      {
        case 13: // enter
          deactivate_element(window);
          return args;        
        case 27: // esc
          deactivate_element(window);        
          return 0;
        case 9: // tab
          activate_next_element(window);
          return 0;
        case 8: // backspace
          if(caretpos)
            del_char_from_string(&edit_text_ptr[--caretpos]);
          break;
        default:
          if(strlen(edit_text_ptr) < element->c)
            add_char_to_string(&edit_text_ptr[caretpos++], c);
          break;
      }
    }

    if(!caretpos) texts.x = 1;
    else
      texts.x = 1 + string_width(ltextp, _FONT_HEIGHT, caretpos-(ltextp-element->text)-1, element->monospace);
    texts.text = element->text + caretpos;

    draw_rectangle(element_x+texts.x, element_y, element->width - texts.x, _FONT_HEIGHT+2, _TEXTBOX_ABGCOLOR);
    texts.x += element_x+1;
    if(caretpos)
      cprint_text(texts.x, element_y+2, &element->text[caretpos-1], _TEXTBOX_TEXTCOLOR, _FONT_HEIGHT, _RES_X, 1, element->monospace);
    texts.x += string_width(element->text+caretpos-1, _FONT_HEIGHT, 1, element->monospace);

    s = sltextbox_needs_scrolling(caretpos, ltextp, element_x, element);
    vscroll:
    if(s > 0)
    {
      ltextp = edit_text_ptr+caretpos-(element->width>>3);
      goto redraw;
    }
    else if(s < 0)
    {
      if(caretpos > 4) ltextp -= 4;
      else ltextp--;
      redraw:
      draw_textbox_caret(2);
      draw_rectangle(element_x, element_y, element->width, _FONT_HEIGHT+2, _TEXTBOX_ABGCOLOR);    
      texts = cprint_text(element_x+2, element_y+2, ltextp, _TEXTBOX_TEXTCOLOR, _FONT_HEIGHT,
        element_x+element->width, caretpos-(ltextp-edit_text_ptr), 0);
    }
      
    if(caretpos)
    {
      caret_screenx = texts.x-1;
      caret_screeny = element_y+1;
      texts = cprint_text(texts.x, element_y+2, texts.text, _TEXTBOX_TEXTCOLOR, _FONT_HEIGHT, element_x+element->width, textl, element->monospace);
    }
    else
    {
      caret_screenx = element_x+1;
      caret_screeny = element_y+1;
      cprint_text(element_x+2, element_y+2, ltextp, _TEXTBOX_TEXTCOLOR, _FONT_HEIGHT,
        element_x+element->width, caretpos-(ltextp-edit_text_ptr), 0);
    }

//    draw_mouse_cursor(1);             
    caret_visible = 1;
  }  
  end:
  draw_textbox_caret(0);

  return ~WM_WINPROGACT;
}

/*
  This function is the handler for editing text in textboxes
  
  Returns: 0 if the user presses ESC,
    1 if ENTER,
    2 if TAB
*/
/*int edit_text(struct prog_window *window, unsigned int textbox)
{
  unsigned char *textp = window->window_grid[textbox]->text;
  unsigned int m;
  struct text_print texts;
  int c;
  unsigned int caretpos;
  unsigned int textl;
  unsigned char *ltextp = textp;

//  disable_vpu_interrupts = 1;
  reset_vpu_interrupt_handlers();

  draw_mouse_cursor(2);                           

  draw_rectangle(window->x+window->window_grid[textbox]->x, window->y+_FONT_HEIGHT+2+window->window_grid[textbox]->y,
          window->window_grid[textbox]->width, _FONT_HEIGHT+2, _TEXTBOX_ABGCOLOR);    

  texts = cprint_text(window->x+window->window_grid[textbox]->x+2,
      window->y+4+_FONT_HEIGHT+window->window_grid[textbox]->y, ltextp, _TEXTBOX_TEXTCOLOR, _FONT_HEIGHT, mouse->pointer_x+8, 0);

  caretpos = texts.text - textp;
  
//  if(!*(texts.text-1))
//  {
//    caretpos--;
//    texts.text--;
//  }

  // draw caret
  draw_line(texts.x-1, window->y+3+_FONT_HEIGHT+window->window_grid[textbox]->y, texts.x-1, window->y+2+(2*_FONT_HEIGHT)+window->window_grid[textbox]->y, _TEXTBOX_TEXTCOLOR);

  texts = cprint_text(texts.x,
    window->y+4+_FONT_HEIGHT+window->window_grid[textbox]->y, texts.text, _TEXTBOX_TEXTCOLOR, _FONT_HEIGHT, window->x+window->window_grid[textbox]->x+window->window_grid[textbox]->width, 0);
  while(*texts.text)
    texts.text++;
  textl = texts.text-textp;

  while(1)
  {
    call_drv_screenupdate();
    if(!(c = getch()))
    {
      switch(getch())
      {
        case 75: if(caretpos) caretpos--; break;
        case 77: if(caretpos < textl) caretpos++; break;
        case 71: caretpos=0; ltextp=textp; break;
        case 79: caretpos=textl; if(textl>10) ltextp=&textp[textl]-10; break;
        case 83:
        // delete
          for(m=caretpos;textp[m];m++)
            textp[m] = textp[m+1];
//          memmove(&textp[caretpos], &textp[caretpos+1], strlen(&textp[caretpos])); // bloated?
        break;
      }
    }
    else
    {
      switch(c)
      {
        case 27: case 13: case 9:
          window_drawelement(window, textbox);
          draw_mouse_cursor(1);
//          disable_vpu_interrupts = 0;
          set_vpu_interrupt_handlers();
          if(c == 27)
            return 0;
          if(c == 13)
            return 1;
          if(c == 9)
            return 2;
        case 8:
        // backspace
          if(caretpos)
          {
//            for(m=--caretpos;m<textl;m++)
            for(m=--caretpos;textp[m];m++)
              textp[m] = textp[m+1];
//            textl--;
          }
        break;
        default:
          if(textl < window->window_grid[textbox]->c)
          {
            for(m=textl;m>caretpos;m--)
              textp[m] = textp[m-1];
            textp[caretpos++] = c;
          }
        break;
      }
    }
    draw_rectangle(window->x+window->window_grid[textbox]->x, window->y+_FONT_HEIGHT+2+window->window_grid[textbox]->y,
          window->window_grid[textbox]->width, _FONT_HEIGHT+2, _TEXTBOX_ABGCOLOR);    

    texts = cprint_text(window->x+window->window_grid[textbox]->x+2,
      window->y+4+_FONT_HEIGHT+window->window_grid[textbox]->y, ltextp, _TEXTBOX_TEXTCOLOR, _FONT_HEIGHT,
      window->x+window->window_grid[textbox]->x+window->window_grid[textbox]->width, caretpos-(ltextp-textp));

    if(caretpos && texts.x > window->x+window->window_grid[textbox]->x+window->window_grid[textbox]->width -30)
    {
      ltextp++;
      if(texts.x > window->x+window->window_grid[textbox]->x+window->window_grid[textbox]->width -10)
        ltextp+=4;
    }
    else if(texts.x < window->x+window->window_grid[textbox]->x+30 && ltextp>textp)
    {
      ltextp--;
      if(texts.x < window->x+window->window_grid[textbox]->x+10 && ltextp>textp)
        ltextp-=4;
    }     
      
    if(caretpos)
    {
      draw_line(texts.x-1, window->y+3+_FONT_HEIGHT+window->window_grid[textbox]->y, texts.x-1, window->y+2+(2*_FONT_HEIGHT)+window->window_grid[textbox]->y, _TEXTBOX_TEXTCOLOR);

      texts = cprint_text(texts.x,
        window->y+4+_FONT_HEIGHT+window->window_grid[textbox]->y, texts.text, _TEXTBOX_TEXTCOLOR, _FONT_HEIGHT, window->x+window->window_grid[textbox]->x+window->window_grid[textbox]->width, textl);
    }
    else
      draw_line(window->x+window->window_grid[textbox]->x+1, window->y+3+_FONT_HEIGHT+window->window_grid[textbox]->y, window->x+window->window_grid[textbox]->x+1, window->y+2+(2*_FONT_HEIGHT)+window->window_grid[textbox]->y, _TEXTBOX_TEXTCOLOR);        
    while(*texts.text)
      texts.text++;
    textl = texts.text-textp;
  }
}*/
