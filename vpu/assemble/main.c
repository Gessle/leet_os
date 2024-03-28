#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <conio.h>
#include <dos.h>
//#include <ncurses.h>
//#include <unistd.h>
//#include <SDL2/SDL.h>
//#include <SDL2/SDL_ttf.h>

unsigned debug = 0;

unsigned console = 0;

//#include "../console/output.c"

unsigned char outofmemory[] = "Out of memory.";

unsigned char message[100];
unsigned char hexstr[16];

#define putstr(str) puts(str)

#include "globals.c"
#include "asbstrct.c"

#include "../vpustrct.c"

#include "../opcodes.c"
#include "../instr.c"
#include "data.c"
#include "parsemov.c"
#include "basic.c"
#include "symbols.c"
#include "parshead.c"
#include "saveprog.c"

#define MOREMEMORY 512

int main(int argc, char **argv)
{
  unsigned n = 0;

  char instruction[LINE_LEN];
  unsigned code_allocated = 1024;
  unsigned char *bcode = calloc(code_allocated, sizeof(char));
//  unsigned char *bcodep = bcode;
//  char header[] = "stack 1166\ncode 2 32000\ndata 32 64000\ninterrupts 28\n";
  struct return_string *retval;
  struct return_string bcheader;
  FILE *fp = 0;
  char hexstr[6];
  void *new_pointer;

  if(!bcode) goto outofmemory;

//  initscr();
//  noecho();
//  curs_set(1);
//  cbreak();
//  keypad(stdscr, 1);

  if(argc == 3)
    fp = fopen(argv[1], "r");
  if(!fp)
  {
    putstr("Cannot open file or no file specified\n");
    return 0;
  }

  bcheader = parse_header(fp);

  if(!init_data_segs(0))
  {
    outofmemory:putstr(outofmemory);
    return 1;
  }

  symbols = calloc(1, sizeof(struct symbol));
  if(!symbols)
    goto outofmemory;
  
  build:while(fgets(instruction, LINE_LEN, fp) && ++line_num)
  {
    switch(start_instr(instruction, &retval))
    {
      case 0:
        if(debug)
        {
          sprintf(message, "Generated %u bytes long instruction: ", retval->length);
          putstr(message);
        }
        message[0] = 0;
        for(n=0;n<retval->length;n++)
        {
          if(codep_offset == code_allocated -1)
          {
            putstr("Out of memory! Allocating more...");
            new_pointer = realloc(bcode, (code_allocated+=MOREMEMORY)*sizeof(unsigned char));
            if(new_pointer) bcode = new_pointer;
            else goto outofmemory;
            memset(&bcode[codep_offset], 0, MOREMEMORY);
          }
          bcode[codep_offset++] = retval->str[n];
          sprintf(hexstr, "0x%.2X ", retval->str[n]);
          strcat(message, hexstr);
        }
        free(retval);
        if(debug)
          putstr(message);
      break;
      case -1:
        return 1;
    }
  }
  if(!pass++)
  {
    putstr("First pass complete!");
    rewind(fp);
    parse_header(fp);
    codep_offset=0;
    codep_segment=0;
    goto build;
  }
  fclose(fp);

  codep_segment++;
  datap_segment++;

  sprintf(message, "\nWriting to file %s ...", argv[2]);
  putstr(message);

  if(save_program(argv[2], &bcheader, bcode))
    putstr("Error while saving!");

//  napms(5000);

//  endwin();
  return 0;

}

