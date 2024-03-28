#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <dos.h>

unsigned char __far *VGA=(unsigned char __far *)0xB8000000L;

#include "globals.c"
//#include "graphics.c"
#include "settings.c"
#include "textmode.c"
#include "structs.c"

struct desktop_shortcut shortcut_data;

char icon_path[81];  

void load_shortcut_data(char *path)
{
  unsigned m;
  int c;
  char coords[5];
  FILE *fp = fopen(path, "r");
  if(!fp) return;

  for(m=0;(c = fgetc(fp)) != ';';m++)
    shortcut_data.path[m] = c;
  shortcut_data.path[m]=0;

  for(m=0;(c = fgetc(fp)) != ';';m++)
    shortcut_data.working_dir[m] = c;
  shortcut_data.working_dir[m] = 0;

  for(m=0;(c = fgetc(fp)) != ';';m++)
    shortcut_data.text[m] = c;
  shortcut_data.text[m]=0;      

  for(m=0;(c = fgetc(fp)) != ';';m++)
    coords[m] = c;
  coords[m] = 0;
  shortcut_data.x = atoi(coords);

  for(m=0;(c = fgetc(fp)) != ';';m++)
    coords[m] = c;
  coords[m] = 0;      
  shortcut_data.y = atoi(coords);

  for(m=0;(c = fgetc(fp)) != ';';m++)
    coords[m] = c;
  coords[m] = 0;      
  shortcut_data.ask_args = atoi(coords);    

  for(m=0;(c = fgetc(fp)) != -1;m++)
    icon_path[m] = c;
  icon_path[m] = 0;

  fclose(fp);
}

void print_shortcut_data(void)
{
  char coords[5];
  clear_screen_tm(VGA);
    
  printtxt(0, 5, "Shortcut data", 0x1F, VGA);

  printtxt(3, 0, "[1] Command: ", 0x0F, VGA);
  printtxt_len(3, 30, shortcut_data.path, 0x0F, VGA, 49);  

  printtxt(4, 0, "[2] Working directory: ", 0x0F, VGA);
  printtxt_len(4, 30, shortcut_data.working_dir, 0x0F, VGA, 49);

  printtxt(5, 0, "[3] Title: ", 0x0F, VGA);
  printtxt(5, 30, shortcut_data.text, 0x0F, VGA);    
  
  printtxt(6, 0, "[4] X coord: ", 0x0F, VGA);
  sprintf(coords, "%i", shortcut_data.x);
  printtxt(6, 30, coords, 0x0F, VGA);    
  
  printtxt(7, 0, "[5] Y coord: ", 0x0F, VGA);
  sprintf(coords, "%i", shortcut_data.y);
  printtxt(7, 30, coords, 0x0F, VGA);    
  
  printtxt(8, 0, "[6] Ask for arguments: ", 0x0F, VGA);
  if(shortcut_data.ask_args)
    printtxt(8, 30, "Yes", 0x0F, VGA);
  else
    printtxt(8, 30, "No", 0x0F, VGA);  

  printtxt(9, 0, "[7] Icon file: ", 0x0F, VGA);
  printtxt_len(9, 30, icon_path, 0x0F, VGA, 49);

  printtxt(10, 0, "[S] Save", 0x0F, VGA);
}

void edit_properties(unsigned property, char *value, unsigned length)
{
  unsigned row, col;
  int c;
  unsigned char *editvalue = malloc(length*sizeof(char));
  unsigned char *properties[] =
  {
    "Command",
    "Working directory",
    "Title",
    "X coord",
    "Y coord",
    "Icon file"
  };
  for(col=0;value[col];col++)
    editvalue[col] = value[col];
  editvalue[col]=0;
  
  // print active title bar
  for(col=70;col>10;col--)
    put_pixel_tm(col, 7, _TITLE_COLOR, VGA);
  printtxt(7, 20, "Edit property", (_TITLE_TEXTCOLOR)|(_TITLE_COLOR<<4), VGA);
  // print active window's  body
  for(row=15;row>7;row--)
    for(col=70;col>10;col--)
      put_pixel_tm(col, row, _WINDOW_BGCOLOR, VGA);

  printtxt(9, 12, properties[property], (_WINDOW_BASICTEXTCOLOR)|(_WINDOW_BGCOLOR<<4), VGA);
//  printtxt(9, 30, value, (_TEXTBOX_TEXTCOLOR)|(_TEXTBOX_BGCOLOR<<4), VGA);
  printtxt(13, 12, "[ Save (enter) ]", (_WINDOW_BASICTEXTCOLOR)|(_MENU_SELCOLOR<<4), VGA);
  printtxt(13, 30, "[ Cancel (esc) ]", (_WINDOW_BASICTEXTCOLOR)|(_MENU_SELCOLOR<<4), VGA);  

  while(1)
  { 
    for(col=68;col>=30;col--)
      put_pixel_tm(col, 9, _TEXTBOX_BGCOLOR, VGA);
    edit_line(30, 9, editvalue, length, 68-30);

    if((c = getch()) == 27) break;
    if(c == 13)
    {
      for(col=0;editvalue[col];col++)
        value[col] = editvalue[col];
      value[col] = 0;
      break;
    }
  }

  free(editvalue);
}

void save(char *path)
{
  char coords[5];
  unsigned n;
  FILE *fp = fopen(path, "w");
  
  for(n=0;shortcut_data.path[n];n++)
    fputc(shortcut_data.path[n], fp);
  fputc(';', fp);

  for(n=0;shortcut_data.working_dir[n];n++)
    fputc(shortcut_data.working_dir[n], fp);
  fputc(';', fp);
  
  for(n=0;shortcut_data.text[n];n++)
    fputc(shortcut_data.text[n], fp);
  fputc(';', fp);

  sprintf(coords, "%i", shortcut_data.x);
  for(n=0;coords[n];n++)
    fputc(coords[n], fp);
  fputc(';', fp);
  
  sprintf(coords, "%i", shortcut_data.y);
  for(n=0;coords[n];n++)
    fputc(coords[n], fp);
  fputc(';', fp);
  
  sprintf(coords, "%i", shortcut_data.ask_args);
  for(n=0;coords[n];n++)
    fputc(coords[n], fp);
  fputc(';', fp);
  
  for(n=0;icon_path[n];n++)
    fputc(icon_path[n], fp);     

  fclose(fp);
  printtxt(24, 60, "Changes saved", 0x0F, VGA);  
}

void main(int argc, char *argv[])
{
  int c;
  char coords[5];  
  
  if(argc == 2)
  {
    load_settings();
    disable_blink();      
    load_shortcut_data(argv[1]);
    print_shortcut_data();    

    while((c = getch()) != 27)
    {
      switch(c)
      {
        case 54:
          if(shortcut_data.ask_args)
            shortcut_data.ask_args = 0;
          else
            shortcut_data.ask_args = 1;
          break;
        case 49:
          edit_properties(0, shortcut_data.path, 81);
          break;
        case 50:
          edit_properties(1, shortcut_data.working_dir, 81-12);
          break;
        case 51:
          edit_properties(2, shortcut_data.text, 32);
          break;
        case 52:
          sprintf(coords, "%i", shortcut_data.x);
          edit_properties(3, coords, 4);
          shortcut_data.x = atoi(coords);
          break;
        case 53:
          sprintf(coords, "%i", shortcut_data.y);
          edit_properties(4, coords, 4);                    
          shortcut_data.y = atoi(coords);          
          break;
        case 55:
          edit_properties(5, icon_path, 81);
          break;
      }      
      print_shortcut_data();

      if(c == 115 || c == 83)
        save(argv[1]);      
    }  
  }
    
}
