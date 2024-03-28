#define SCR_ROWS 24
#define SCR_COLS 79

char message[2000];
char hexstr[6];

unsigned cursor_row = 0;
unsigned cursor_col = 0;

void clear_row(unsigned row)
{
  unsigned col;
  for(col=0;col<=SCR_COLS;col++)
    mvaddch(row, col, ' ');
}

void clear_screen(void)
{
  unsigned n = SCR_ROWS;
  do
  {
    clear_row(n);
  }
  while(n--);
  cursor_row=0;
  cursor_col=0;
}

void scroll_up(void)
{
  unsigned row, col;
  chtype cell;

  for(row=0;row++<SCR_ROWS-1;)
    for(col=0;col<=SCR_COLS;col++)
    {
      cell = mvinch(row+1, col);
      mvaddch(row, col, cell);
    }
  clear_row(SCR_ROWS);
}

void inc_row(void)
{
  if(cursor_row == SCR_ROWS)
    scroll_up();
  else
    cursor_row++;

  cursor_col = 0;
}

void print_text(unsigned row, unsigned col, char *text)
{
  mvprintw(row, col, text);
}

void printchar(char c)
{
  mvaddch(cursor_row, cursor_col++, c);
  if(cursor_col == SCR_COLS)
  {
    scroll_up();
    cursor_col=0;
  }
}

void putstr(char *text)
{
//  mvprintw(cursor_row, cursor_col, text);
  print_text(cursor_row, cursor_col, text);
  inc_row();
  refresh();
}
