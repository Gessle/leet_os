//#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>

unsigned char debug = 0;

#include "vpu_structs.c"
#include "vpu_step.c"

int kbhit(void)
{
    int ch = getch();

    if (ch != ERR) {
        ungetch(ch);
        return 1;
    } else {
        return 0;
    }
}

int main()
{
  struct vpu *cvpu;
  unsigned n = 0;

  cvpu = spawn_new_vpu();
  init_vpu(cvpu, 512, 2048, 1, 4096, 1, 5);

  // .data
  cvpu->data[cvpu->data_segment][0] = 0x04;
  cvpu->data[cvpu->data_segment][1] = 0x00;
  cvpu->data[cvpu->data_segment][2] = 0x02;
  cvpu->data[cvpu->data_segment][3] = 0x02;

  sprintf(&cvpu->data[cvpu->data_segment][0x04], "Näppäintä painettu");

  // .code

  // MOV WORD 0x0000 TO SEGMENT A

  cvpu->code[cvpu->code_segment][0] = OPCODE_MOV;
  cvpu->code[cvpu->code_segment][1] = 0x98;

  cvpu->code[cvpu->code_segment][2] = 0x00;

  cvpu->code[cvpu->code_segment][3] = 0x00;
  cvpu->code[cvpu->code_segment][4] = 0x00;

  // SET INTERRUPT 0x0001 TO PRINTUI

  cvpu->code[cvpu->code_segment][5] = OPCODE_SETINT;
  cvpu->code[cvpu->code_segment][6] = 0x01;
  cvpu->code[cvpu->code_segment][7] = 0x00;

  cvpu->code[cvpu->code_segment][8] = 0x00;
  cvpu->code[cvpu->code_segment][9] = 0x00;

  cvpu->code[cvpu->code_segment][10] = 16;
  cvpu->code[cvpu->code_segment][11] = 0x00;

  // JUMP OVER PRINTUI

  cvpu->code[cvpu->code_segment][12] = OPCODE_JUMP;
  cvpu->code[cvpu->code_segment][13] = 0x00;
  cvpu->code[cvpu->code_segment][14] = 19;
  cvpu->code[cvpu->code_segment][15] = 0x00;

  // INTERRUPT ROUTINE PRINTUI

  cvpu->code[cvpu->code_segment][16] = OPCODE_PRINTSTR;
  cvpu->code[cvpu->code_segment][17] = 0x00;
  cvpu->code[cvpu->code_segment][18] = OPCODE_IRET;

  // WAIT

  cvpu->code[cvpu->code_segment][19] = OPCODE_WAIT;

  // GOTO 12

  cvpu->code[cvpu->code_segment][20] = OPCODE_JUMP;
  cvpu->code[cvpu->code_segment][21] = 0x00;
  cvpu->code[cvpu->code_segment][22] = 12;
  cvpu->code[cvpu->code_segment][23] = 0x00;

  // CALL PRINTUI USING INT

/*  cvpu->code[cvpu->code_segment][n++] = OPCODE_INT;

  cvpu->code[cvpu->code_segment][n++] = 0x01;
  cvpu->code[cvpu->code_segment][n++] = 0x00;*/

  // JUMP TO 12

/*  cvpu->code[cvpu->code_segment][19] = OPCODE_JUMP;

  cvpu->code[cvpu->code_segment][20] = 0x00;

  cvpu->code[cvpu->code_segment][21] = 12;
  cvpu->code[cvpu->code_segment][22] = 0x00;*/


  initscr();
  cbreak();
  noecho();
  nodelay(stdscr, TRUE);
  scrollok(stdscr, TRUE);

  while(!vpu_step(cvpu))
  {
    if(kbhit())
    {
      cvpu->interrupt_wait=1;
//      cvpu->data[cvpu->data_segment][0x60] = getch();
      cvpu->ports[0] = getch();
    }
  }
}
