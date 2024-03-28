#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

unsigned char debug = 0;

#include "console/output.c"

#include "structs.c"
#include "assembler/structs.c"

#include "opcodes.c"
#include "instr.c"

#include "step.c"
#include "loadprog.c"

int main(int argc, char **argv)
{
  struct vpu *cvpu;

  if(!init_instruction_table())
  {
    puts("Failed initializing the instruction table\n");
    return 1;
  }

  cvpu = spawn_new_vpu();

  if(argc == 2)
    load_program(argv[1], cvpu);
  else
  {
    puts("No program to load!\n");
    return 1;
  }

  initscr();
  noecho();
  curs_set(1);
  cbreak();
  keypad(stdscr, 1);

  while(!vpu_step(cvpu));

  endwin();

  return 0;

}
