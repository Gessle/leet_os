#define HISTORY_COUNT 20
#define TTY_COUNT 4

struct con_stdio
{
  char *buff;
  unsigned len;
  unsigned pos;
};

//#define VPUCON_COLS 80
#include "condef.c"

struct console_struct
{
  unsigned row, col;
  unsigned srow, scol;
  unsigned view_row, caret_row, caret_col;
//  unsigned waiting_for_key:1;
  unsigned editmode:2;
  unsigned runmode:2;
  unsigned compiled:1;
  unsigned debug:1;
  unsigned newcommand:1;
  unsigned console_kill:1;
  unsigned noecho:1;
  unsigned nolinebuffer:1;
  unsigned symbol_count;
  int keybuff[2];
  struct vpu *vcpu;
  struct vpu *orig_vpu;    
  struct symbol *symbols;
  struct text_content *asm_content;
  unsigned datap_segment;
  unsigned datap_offset;
  unsigned char command[VPUCON_COLS+1];
  unsigned char history_n;
  unsigned char history[HISTORY_COUNT][VPUCON_COLS];
  unsigned char lines;
  unsigned asm_line;
  struct con_stdio con_stdin;
  struct con_stdio con_stdout;
  struct con_stdio con_stderr;
  unsigned input_col;
  char **con_lines;
};

struct vpu_progwindow
{
  int keybuff[2];
  struct vpu *vcpu;
  unsigned mouse_int;
  unsigned mouse_port;
};
