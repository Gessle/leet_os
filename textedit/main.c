#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef POSIX
#include <ncurses.h>
#endif

#ifdef DOS
#include <conio.h>
#endif

#include "structs.c"
#include "globals.c"

#ifdef POSIX
#include "linux.c"
#endif
#ifdef DOS
#include "dos.c"
#endif

#include "common.c"

void draw_screen(void)
{
  clear_screen();
  print_file_screen();
  print_status();
  scrollbar();
}

int common_keys(int c)
{
  switch(c)
  {
    case TEXTEDIT_KEY_ESC:
      exit(0);
    case TEXTEDIT_KEY_F4:
      search_word();
      return 1;
    case TEXTEDIT_KEY_F5:
      jump_to_row();
      return 1;
    case TEXTEDIT_KEY_F7:
      open_files[file_view].invisiblechars = ~open_files[file_view].invisiblechars;
      return 1;
    case TEXTEDIT_KEY_F6:
      open_files[file_view].syntaxhighlight = ~open_files[file_view].syntaxhighlight;
      return 1;
    case TEXTEDIT_KEY_F9:
      open_file_name();
      return 1;
    case TEXTEDIT_KEY_F1:
      if(file_view)
        file_view--;
      return 1;
    case TEXTEDIT_KEY_F3:
      if(file_view != file_count-1)
        file_view++;
      return 1;
    case TEXTEDIT_KEY_F8:
      close_file();
      if(file_view)
        file_view--;
      if(!file_count)
        exit(0);
    return 1;
    case TEXTEDIT_KEY_F2:
      if(open_files[file_view].editmode)
        save_file();
    return 1;
  }
  return 0;
}

void edit_file_loop()
{
  int c;
  unsigned n;

  while(1)
  {
    if(!open_files[file_view].editmode)
      return;
    draw_screen();
    scr_cursor(open_files[file_view].cursor_row - open_files[file_view].scr_row, open_files[file_view].cursor_col - open_files[file_view].scr_col);
    c = getkeypress();
    newkey:switch(c)
    {
      case TEXTEDIT_KEY_LEFT:
        if(open_files[file_view].cursor_col)
          open_files[file_view].cursor_col--;
      break;
      case TEXTEDIT_KEY_HOME:
        open_files[file_view].cursor_col = 0;
      break;
      case TEXTEDIT_KEY_RIGHT:
        open_files[file_view].cursor_col++;
      break;
      case TEXTEDIT_KEY_END:
        open_files[file_view].cursor_col = open_files[file_view].screen_lines[open_files[file_view].cursor_row - open_files[file_view].scr_row].length;
      break;
      case TEXTEDIT_KEY_UP:
        if(open_files[file_view].cursor_row)
          open_files[file_view].cursor_row--;
      break;
      case TEXTEDIT_KEY_PGUP:
        for(n=scr_rows-1;n--;)
          if(open_files[file_view].cursor_row)
            open_files[file_view].cursor_row--;
      break;
      case TEXTEDIT_KEY_DOWN:
        if(open_files[file_view].cursor_row < open_files[file_view].lines - 1)
          open_files[file_view].cursor_row++;
      break;
      case TEXTEDIT_KEY_PGDN:
        for(n=scr_rows-1;n--;)
          if(open_files[file_view].cursor_row < open_files[file_view].lines - 1)
            open_files[file_view].cursor_row++;
      break;
      case TEXTEDIT_KEY_F10:
        break;
      case TEXTEDIT_KEY_ENTER:
        goto_endline();
        add_newline();
      break;
      case TEXTEDIT_KEY_BACKSPACE:
        if(open_files[file_view].cursor_col)
        {
          goto_endline();
          if(!open_files[file_view].cursor_col)
            goto removenl;
          c = backspace_chars();
          draw_screen();
          if(c != TEXTEDIT_KEY_BACKSPACE)
            goto newkey;
        }
        else
          removenl:if(open_files[file_view].cursor_row)
            remove_newline();
      break;
      case TEXTEDIT_KEY_DELETE:
        if(open_files[file_view].cursor_col
            >= open_files[file_view].screen_lines[open_files[file_view].cursor_row - open_files[file_view].scr_row].length)
        {
          if(open_files[file_view].cursor_row < open_files[file_view].lines - 1)
          {
            open_files[file_view].cursor_col = 0;
            open_files[file_view].cursor_row++;
            remove_newline();
          }
        }
        else
        {
          c = delete_chars();
          draw_screen();
          goto newkey;
        }
      break;
      default:
        if(!common_keys(c))
        {
          goto_endline();
          c = insert_chars(c);
          goto newkey;
        }
    }
    if(open_files[file_view].cursor_col < open_files[file_view].scr_col)
    {
      open_files[file_view].scr_col = open_files[file_view].cursor_col;
    }
    else if(open_files[file_view].cursor_col > open_files[file_view].scr_col + scr_cols - 2)
    {
      open_files[file_view].scr_col = open_files[file_view].cursor_col - scr_cols + 2;
    }
    if(open_files[file_view].cursor_row < open_files[file_view].scr_row)
    {
      while(open_files[file_view].cursor_row < open_files[file_view].scr_row)
        dec_view_row();
    }
    if(open_files[file_view].cursor_row > open_files[file_view].scr_row + scr_rows - 2)
    {
      while(open_files[file_view].cursor_row > open_files[file_view].scr_row + scr_rows - 2)
        inc_view_row();
    }
    if(open_files[file_view].scr_row > open_files[file_view].lines - scr_rows + 1)
    {
      dec_view_row();
    }
  }
}

void view_file_loop(void)
{
  unsigned n;
  int c;
  while(1)
  {
    if(open_files[file_view].editmode)
      return;
    draw_screen();
    switch(c = getkeypress())
    {
      case TEXTEDIT_KEY_LEFT:
        if(open_files[file_view].scr_col)
          open_files[file_view].scr_col--;
      break;
      case TEXTEDIT_KEY_HOME:
        open_files[file_view].scr_col = 0;
      break;
      case TEXTEDIT_KEY_END:
        open_files[file_view].scr_col += scr_cols;
      break;
      case TEXTEDIT_KEY_RIGHT:
        open_files[file_view].scr_col++;
      break;
      case TEXTEDIT_KEY_UP:
        if(open_files[file_view].scr_row)
          dec_view_row();
      break;
      case TEXTEDIT_KEY_PGUP:
        for(n=scr_rows-1;n--;)
          if(open_files[file_view].scr_row)
            dec_view_row();
      break;
      case TEXTEDIT_KEY_PGDN:
        for(n=scr_rows-1;n--;)
          if(open_files[file_view].scr_row + scr_rows - 1 < open_files[file_view].lines)
            inc_view_row();
      break;
      case TEXTEDIT_KEY_DOWN:
        if(open_files[file_view].scr_row + scr_rows - 1 < open_files[file_view].lines)
          inc_view_row();
      break;
      case TEXTEDIT_KEY_F10:
        toggle_edit_mode();
        return;
      default:
        common_keys(c);
    }
  }
}

void show_help()
{
  puts("$ TEXTEDIT filename.txt        // loads or creates a file named filename.txt");
  puts("\nKeys:\n");
  puts("F10: Go to edit mode");
  puts("F9: Open a file");
  puts("F8: Close the file");
  puts("F7: Invisible chars on/off");
  puts("F6: Syntax highlighting on/off");
  puts("F5: Jump to line");
  puts("F4: Search");
  puts("F3: Next file");
  puts("F2: Save file");
  puts("F1: Previous file");
}

int main(unsigned argc, char **argv)
{
  if(argc != 2)
  {
    puts("Usage: $ TEXTEDIT filename.txt");
    puts("Type TEXTEDIT /? for more detailed help");
    return 0;
  }
  init_screen();
  if(argc == 2)
  {
    if(!strcmp(argv[1], "/?"))
    {
      end_program();
      show_help();
      return 0;
    }
    if(open_file(argv[1]))
      puts("Error while opening file");
    atexit(close_all_files);
    while(1)
    {
      if(open_files[file_view].editmode)
        edit_file_loop();
      else
        view_file_loop();
    }
  }
  getch();
  end_program();
}
