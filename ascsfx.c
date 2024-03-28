#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <dos.h>

unsigned char __far *VGA=(unsigned char __far *)0xB8000000L;

#define ASC_ROWS 20

#include "globals.c"
//#include "graphics.c"
#include "settings.c"
#include "textmode.c"

char **file_assoc;
unsigned file_assoc_count;

void read_file_assoc(void)
{
  FILE *fp;
//  static char buffer[86];
  unsigned int n;
  unsigned int i = 0;

  if(!(file_assoc = malloc(sizeof(char*))))
    return;

  if(!(fp = fopen("fileassc.dat", "r")))
    return;
  while(!feof(fp))
  {
    if(!(file_assoc = realloc(file_assoc, ++i*sizeof(char*))))
      return;
    if(!(file_assoc[i-1] = malloc(86*sizeof(char))))
      return;
    
    for(n=0;n<86;n++)
    {
      file_assoc[i-1][n] = fgetc(fp);
      
      if(file_assoc[i-1][n] == ' ' && n<4)
        file_assoc[i-1][n] = 0;
        
      if(file_assoc[i-1][n] == '\n' || feof(fp))
      {
        file_assoc[i-1][n] = 0;
        break;
      }      
    }
  }
  fclose(fp);
  file_assoc_count = i;
  return;
}

void list_file_assoc(unsigned selection, unsigned scroll)
{
  unsigned n = scroll+ASC_ROWS;
  unsigned char color;
  unsigned c;

  if(n>file_assoc_count)
    n = file_assoc_count;

  clear_screen_tm(VGA);

  printtxt(0, 5, "File type associations", 0x1F, VGA);

  printtxt(2, 0, "Suffix   Program", 0x07, VGA);

  for(;n>scroll;n--)
  {
    if(n-1 == selection) color = 0x2F;
    else color = 0x0F;
    
    printtxt(n-scroll+2, 0, file_assoc[n-1], color, VGA);

    for(c=0;file_assoc[n-1][c];c++);
      printtxt_len(n-scroll+2, 9, &file_assoc[n-1][++c], color, VGA, 70);    
  }

  printtxt(24, 0, "[Enter]: Edit entry | [DEL]: Delete entry | [N] New entry | [S] Save", 0x0F, VGA);        
}

void delete(unsigned selection)
{
  unsigned n;
  file_assoc_count--;
  for(;selection<file_assoc_count;selection++)
  {
    free(file_assoc[selection]);
    file_assoc[selection] = file_assoc[selection+1];
  }

  if(!(file_assoc = realloc(file_assoc, file_assoc_count*sizeof(char*))))
    return;
}

void edit_file_assoc(unsigned selection)
{
  unsigned row, col;
  unsigned c;
  unsigned n;
  char *suffix = malloc(4*sizeof(char));
  char *path = malloc(81*sizeof(char));
  // print active title bar
  for(col=70;col>10;col--)
    put_pixel_tm(col, 7, _TITLE_COLOR, VGA);
  printtxt(7, 20, "Edit file type association", (_TITLE_TEXTCOLOR)|(_TITLE_COLOR<<4), VGA);
  // print active window's  body
  for(row=15;row>7;row--)
    for(col=70;col>10;col--)
      put_pixel_tm(col, row, _WINDOW_BGCOLOR, VGA);

  printtxt(9, 12, "[F]ilename suffix:", (_WINDOW_BASICTEXTCOLOR)|(_WINDOW_BGCOLOR<<4), VGA);
  for(col=68;col>=30;col--)
    put_pixel_tm(col, 9, _TEXTBOX_BGCOLOR, VGA);
  printtxt_len(9, 30, file_assoc[selection], (_TEXTBOX_TEXTCOLOR)|(_TEXTBOX_BGCOLOR<<4), VGA, 68-30);    
  
  printtxt(11, 12, "[P]rogram:", (_WINDOW_BASICTEXTCOLOR)|(_WINDOW_BGCOLOR<<4), VGA);
  for(col=68;col>=30;col--)
    put_pixel_tm(col, 11, _TEXTBOX_BGCOLOR, VGA);
    
  for(c=0;file_assoc[selection][c];c++)
    suffix[c] = file_assoc[selection][c];
  suffix[c] = 0;
    
  printtxt_len(11, 30, &file_assoc[selection][++c], (_TEXTBOX_TEXTCOLOR)|(_TEXTBOX_BGCOLOR<<4), VGA, 68-30);

  for(n=0;file_assoc[selection][c];c++,n++)
    path[n] = file_assoc[selection][c];
  path[n] = 0;

  printtxt(13, 12, "[ Save (enter) ]", (_WINDOW_BASICTEXTCOLOR)|(_MENU_SELCOLOR<<4), VGA);
  printtxt(13, 30, "[ Cancel (esc) ]", (_WINDOW_BASICTEXTCOLOR)|(_MENU_SELCOLOR<<4), VGA);

  while((c = getch()) != 27)
  {
    // F
    if(c == 102)
    {
      for(col=68;col>=30;col--)
        put_pixel_tm(col, 9, _TEXTBOX_BGCOLOR, VGA);      
      edit_line(30, 9, suffix, 3, 68-30);
    }
    // P
    if(c == 112)
    {
      for(col=68;col>=30;col--)
        put_pixel_tm(col, 11, _TEXTBOX_BGCOLOR, VGA);      
      edit_line(30, 11, path, 81, 68-30);
    }
    // enter
    if(c == 13)
    {
      for(n=0;suffix[n];n++)
        file_assoc[selection][n] = suffix[n];
      file_assoc[selection][n++] = 0;
      for(c=0;path[c];c++)
        file_assoc[selection][n+c] = path[c];
      file_assoc[selection][n+c] = 0;
      if(!n || !c) delete(selection);              
      break;
    }
  }
  for(n=0;suffix[n];n++);
  for(c=0;path[c];c++);
  if(!n || !c) delete(selection);
  
  free(suffix);
  free(path);

}

void new_file_assoc(void)
{
  if(!(file_assoc = realloc(file_assoc, ++file_assoc_count*sizeof(char*))))
    return;
  if(!(file_assoc[file_assoc_count-1] = calloc(86, sizeof(char))))
    return;
  edit_file_assoc(file_assoc_count-1);
}

void save(void)
{
  FILE *fp;
  unsigned c;
  unsigned n = file_assoc_count;

  if(!(fp = fopen("fileassc.dat", "w")))
    return;

  while(n--)
  {
    for(c=0;file_assoc[n][c];c++)
      fputc(file_assoc[n][c], fp);
    fputc(' ', fp);
    c++;
    for(;file_assoc[n][c];c++)
      fputc(file_assoc[n][c], fp);
    if(n)
      fputc('\n', fp);
  }
  
  fclose(fp);

  printtxt(24, 60, "Changes saved", 0x0F, VGA);  
}

void main()
{
  unsigned selection = 0;
  int c;
  unsigned scroll = 0;

  load_settings();
  disable_blink();  
  read_file_assoc();
  list_file_assoc(selection, scroll);
  
  while((c = getch()) != 27)
  {
    if(!c)
    {
      c = getch();
      if(c == 72 && selection)
      {
        selection--;
        if (selection<scroll)
          scroll = selection;
      }
      else if(c == 80 && selection<file_assoc_count-1)
      {
        selection++;
        if(selection+1>scroll+ASC_ROWS)
          scroll++;
      }
      else if(c == 73)
      {
        selection = 0;
        scroll = 0;
      }
      else if(c == 81)
      {
        selection = file_assoc_count-1;
        scroll = file_assoc_count-ASC_ROWS;
        if(scroll>file_assoc_count)
          scroll=0;
      }
      // del
      else if(c == 83)
      {
        delete(selection);
      }
    }
    if(c == 13)
      edit_file_assoc(selection);
    // N
    if(c == 110 || c == 78)
      new_file_assoc();      
    list_file_assoc(selection, scroll);          
    // S
    if(c == 115 || c == 83)
      save();
      
  }

  clear_screen_tm(VGA);  
}
