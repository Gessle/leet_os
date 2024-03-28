void syntax_highlight(int line);

/*
  This file contains common functions for all CPU architechtures and operating systems.
*/

void clear_line(unsigned row)
{
  unsigned n = scr_cols;
  while(n--)
    print_char(n, row, ' ', 0x7);
}

void print_line(unsigned x, unsigned y, char *str, unsigned char color)
{
  while(*str)
    print_char(x++, y, *str++, color);
}

void status_line(char *str)
{
  clear_line(scr_rows-1);
  set_color(0, scr_rows-1, 0x20, scr_cols);
  print_line(0, scr_rows-1, str, 0x20);
}

int new_file_struct(void)
{
  void *new_pointer;
  if(!file_count)
  {
    open_files = calloc(++file_count, sizeof(struct file));
    if(!open_files)
    {
      nomemory:
      --file_count;
      return -1;
    }
  }
  else
  {
    new_pointer = realloc(open_files, ++file_count * sizeof(struct file));
    if(!new_pointer) goto nomemory;
    open_files = new_pointer;
    memset(&open_files[file_count-1], 0, sizeof(struct file));
  }
  return 0;
}

void close_file_struct(unsigned index)
{
  free(open_files[index].screen_lines);

  if(open_files[index].editfp)
    remove(open_files[index].editfilename);


  while(index < file_count-1)
  {
    open_files[index] = open_files[index+1];
    index++;
  }

  if(--file_count)
    open_files = realloc(open_files, file_count * sizeof(struct file));
}

void get_line_count(unsigned index)
{
  unsigned long pos;

  open_files[index].lines = 1;
  while(!feof(open_files[index].fp))
  {
    pos = ftell(open_files[index].fp);
    if(fgetc(open_files[index].fp) == '\n')
    {
      open_files[index].lines++;
      if(ftell(open_files[index].fp) == pos+2)
        open_files[index].longnewlines = 1;
    }
//    ++open_files[index].length;
  }
  open_files[index].length = ftell(open_files[index].fp) + 1;
}

int open_file(char *filename)
{
  if(new_file_struct())
    return -1;

  reopen: if(!(open_files[file_count-1].fp = fopen(filename, "r")))
  {
    if(!(open_files[file_count-1].fp = fopen(filename, "w")))
    {
      close_file_struct(file_count-1);
      return -1;
    }
    fclose(open_files[file_count-1].fp);
    goto reopen;
  }

  get_line_count(file_count-1);
  strcpy(open_files[file_count-1].filename, filename);

  open_files[file_count-1].screen_lines = calloc(scr_rows, sizeof(struct line));

  return 0;
}

void close_file(void)
{
  fclose(open_files[file_view].fp);
  close_file_struct(file_view);
}

void close_all_files(void)
{
  file_view = 0;
  while(file_count)
  {
    close_file();
  }
}

void save_file(void)
{
  int c;
  open_files[file_view].fp = fopen(open_files[file_view].filename, "w");
  rewind(open_files[file_view].editfp);
  while(ftell(open_files[file_view].fp) < open_files[file_view].length - 1)
  {
    fputc(fgetc(open_files[file_view].editfp), open_files[file_view].fp);
  }
  fclose(open_files[file_view].fp);
  open_files[file_view].fp = open_files[file_view].editfp;
  status_line("File saved.");
  getkeypress();
}

void clear_screen(void)
{
  unsigned n = scr_rows;
  unsigned i;
  while(n--)
  {
    for(i=scr_cols;i--;)
    {
      print_char(i, n, ' ', 0x7);
    }
  }
}

int print_file_line(unsigned line)
{
  int c;
  unsigned n = 0;

//  while(n < scr_cols-1 && (c = fgetc(open_files[file_view].fp)) != '\n' && !feof(open_files[file_view].fp))
  while(ftell(open_files[file_view].fp) < open_files[file_view].length - 1 && n < scr_cols-1 && (c = fgetc(open_files[file_view].fp)) != '\n')
  {
    print_char(n++, line, c, 0x7);
    open_files[file_view].screen_lines[line].length++;
  }
  if(open_files[file_view].invisiblechars)
  {
    print_char(n, line, 188, 0x7);
  }

  return c;
}

void print_file_screen(void)
{
  unsigned row = 0;
  unsigned long n;
//  unsigned long pos;

  fseek(open_files[file_view].fp, open_files[file_view].file_pos, SEEK_SET);

  do
  {
    open_files[file_view].screen_lines[row].file_offset = ftell(open_files[file_view].fp);
    open_files[file_view].screen_lines[row].length = 0;

    for(n=open_files[file_view].scr_col;n--;)
    {
      if(ftell(open_files[file_view].fp) < open_files[file_view].length - 1 && fgetc(open_files[file_view].fp) == '\n')
        goto nextrow;
      open_files[file_view].screen_lines[row].length++;
    }

    if(print_file_line(row) != '\n')
      while(ftell(open_files[file_view].fp) < open_files[file_view].length - 1 && fgetc(open_files[file_view].fp) != '\n')
        open_files[file_view].screen_lines[row].length++;

    nextrow:;
//    open_files[file_view].screen_lines[row].length = ftell(open_files[file_view].fp) - open_files[file_view].screen_lines[row].file_offset;
//    if(ftell(open_files[file_view].fp) < open_files[file_view].length - 1)
//      open_files[file_view].screen_lines[row].length--;
  }
  while(row++ < scr_rows-2 && row < open_files[file_view].lines);

  if(open_files[file_view].syntaxhighlight)
    syntax_highlight(-1);
}

void print_status(void)
{
  char status[200];
  unsigned long row, col;
  if(!open_files[file_view].editmode)
  {
    row = open_files[file_view].scr_row;
    col = open_files[file_view].scr_col;
  }
  else
  {
    row = open_files[file_view].cursor_row;
    col = open_files[file_view].cursor_col;
  }
  sprintf(status, "File: %s | Row: %lu/%lu Col: %lu Length: %lu", open_files[file_view].filename, row, open_files[file_view].lines, col, open_files[file_view].length);
  if(open_files[file_view].longnewlines)
    strcat(status, " | LONG LINEFEEDS");
  if(open_files[file_view].editmode)
    strcat(status, " | EDIT MODE");

  status_line(status);
}

void dec_view_row(void)
{
  open_files[file_view].scr_row--;
  fseek(open_files[file_view].fp, open_files[file_view].file_pos, SEEK_SET);

  fseek(open_files[file_view].fp, -2, SEEK_CUR);
  if((fgetc(open_files[file_view].fp)) == 10)
    if(ftell(open_files[file_view].fp) == open_files[file_view].file_pos)
    {
      fseek(open_files[file_view].fp, -2, SEEK_CUR);
      open_files[file_view].longnewlines = 1;
    }
    else goto end;

  while(ftell(open_files[file_view].fp) >= 2)
  {
    fseek(open_files[file_view].fp, -2, SEEK_CUR);
    if((fgetc(open_files[file_view].fp)) == 10)
      break;
  }

  if(ftell(open_files[file_view].fp) < 2)
    if(fgetc(open_files[file_view].fp) != 10)
    {
      rewind(open_files[file_view].fp);
      if(fgetc(open_files[file_view].fp) != 10)
        rewind(open_files[file_view].fp);
    }

  end:open_files[file_view].file_pos = ftell(open_files[file_view].fp);
}

void inc_view_row(void)
{
  fseek(open_files[file_view].fp, open_files[file_view].file_pos, SEEK_SET);

  while(fgetc(open_files[file_view].fp) != '\n')
  {
    if(feof(open_files[file_view].fp))
      return;
  }
  open_files[file_view].scr_row++;
  open_files[file_view].file_pos = ftell(open_files[file_view].fp);
}

void scrollbar(void)
{
  unsigned anchor_pos = 0;
  unsigned n = scr_rows;

  if(open_files[file_view].lines > scr_rows - 2)
    anchor_pos = (open_files[file_view].scr_row * scr_rows) / (open_files[file_view].lines - (scr_rows - 2));

  while(n--)
  {
    if(n == anchor_pos)
      print_char(scr_cols-1, n, 219, 2);
    else
      print_char(scr_cols-1, n, '|', 2);
  }
}

char *temp_filename(void)
{
  static char retval[] = "_TXTEDIT.000";
  FILE *fp;
  unsigned n = 0x1000;

  if(fp = fopen(retval, "r"))
  {
    fclose(fp);
    while(n--)
    {
      sprintf(retval, "_TXTEDIT.%X", n);
      if(!(fp = fopen(retval, "r")))
        return retval;
      else
        fclose(fp);
    }
    abort();
  }
  return retval;
}

void toggle_edit_mode(void)
{
  int c;

  open_files[file_view].editfp = fopen(strcpy(open_files[file_view].editfilename, temp_filename()), "w+");
  rewind(open_files[file_view].fp);
  while((c = fgetc(open_files[file_view].fp)) != EOF)
    fputc(c, open_files[file_view].editfp);
  fclose(open_files[file_view].fp);
  open_files[file_view].editmode = 1;
  open_files[file_view].fp = open_files[file_view].editfp;
  open_files[file_view].cursor_row = open_files[file_view].scr_row;
  open_files[file_view].cursor_col = open_files[file_view].scr_col;
}

#define INSERT_BUFF 100

FILE *tr_file(char **tempfilename, char *buff)
{
  FILE *fp;
  unsigned n = 0;
  int c;

  if(buff)
  {
    buff[0] = ' ';
    buff[1] = 0;
  }
  // open a new temporary file to store the rest of the file
  fp = fopen(*tempfilename = temp_filename(), "w+");
//  while((c = fgetc(open_files[file_view].editfp)) != EOF)
  while(ftell(open_files[file_view].editfp) < open_files[file_view].length - 1 && (c = fgetc(open_files[file_view].editfp)))
  {
    fputc(c, fp);
    // and store the rest of the line
    if(buff)
    {
      buff[n+1] = ' ';
      buff[n+2] = 0;
      if(n < INSERT_BUFF-2)
      {
        if(c == '\n')
        {
          buff[n] = ' ';
          n = INSERT_BUFF-2;
          continue;
        }
        buff[n++] = c;
      }
    }
//    buff[n] = ' ';
//    buff[n+1] = 0;
  }
  return fp;
}

void goto_endline(void)
{
  if(open_files[file_view].screen_lines[open_files[file_view].cursor_row - open_files[file_view].scr_row].length
    < open_files[file_view].cursor_col)
    open_files[file_view].cursor_col = open_files[file_view].screen_lines[open_files[file_view].cursor_row - open_files[file_view].scr_row].length;
}

unsigned long seek_cursor_pos(void)
{
  // seek to the position where new characters are to be inserted
  fseek(open_files[file_view].editfp,
     open_files[file_view].screen_lines[open_files[file_view].cursor_row - open_files[file_view].scr_row].file_offset
     + open_files[file_view].cursor_col, SEEK_SET);
  // save the position
  return ftell(open_files[file_view].editfp);
}

void append_tempfile(FILE *fp, unsigned offset, char *filename)
{
  int c;
  // append the temporary file
  fseek(fp, offset, SEEK_SET);
  while((c = fgetc(fp)) != EOF)
    fputc(c, open_files[file_view].editfp);
  fclose(fp);
  open_files[file_view].length = ftell(open_files[file_view].fp) + 1;
  remove(filename);
}

int insert_chars(int c)
{
  char buff[INSERT_BUFF];
  char buff2[INSERT_BUFF];
  unsigned n = 0;
  unsigned m = 0;
  FILE *fp;
  unsigned long pos;
  char *tempfilename;

  pos = seek_cursor_pos();

  fp = tr_file(&tempfilename, buff2);

  // get user input string
  do
  {
    if(open_files[file_view].cursor_col > open_files[file_view].scr_col + scr_cols - 2)
    {
      open_files[file_view].scr_col+=40;
      break;
    }
    else
      print_status();

    buff[n++] = c;
    print_char(open_files[file_view].cursor_col++ - open_files[file_view].scr_col,
      open_files[file_view].cursor_row - open_files[file_view].scr_row,
      c, 0x07);

    print_line(open_files[file_view].cursor_col - open_files[file_view].scr_col,
      open_files[file_view].cursor_row - open_files[file_view].scr_row,
      buff2, 0x07);


    if(open_files[file_view].syntaxhighlight)
      syntax_highlight(open_files[file_view].cursor_row - open_files[file_view].scr_row);

    scr_cursor(open_files[file_view].cursor_row - open_files[file_view].scr_row, 
      open_files[file_view].cursor_col - open_files[file_view].scr_col);
  }
  while((c = getkeypress()) >= 0 && n < INSERT_BUFF);

  // insert the string
  fseek(open_files[file_view].editfp, pos, SEEK_SET);
  while(m < n)
    fputc(buff[m++], open_files[file_view].editfp);

  append_tempfile(fp, 0, tempfilename);

  draw_screen();

  return c;
}

int backspace_chars(void)
{
  char *tempfilename;
  char buff[INSERT_BUFF];
  unsigned long pos = seek_cursor_pos();
  FILE *fp = tr_file(&tempfilename, buff);
  int c;
  unsigned n = 0;

  do
  {
    n++;

    print_line(--open_files[file_view].cursor_col - open_files[file_view].scr_col,
      open_files[file_view].cursor_row - open_files[file_view].scr_row,
      buff, 0x7);

    if(open_files[file_view].cursor_col <= open_files[file_view].scr_col)
    {
      if(open_files[file_view].scr_col >= 40)
        open_files[file_view].scr_col -= 40;
      else
        open_files[file_view].scr_col = 0;
      break;
    }

    print_status();

    if(open_files[file_view].syntaxhighlight)
      syntax_highlight(open_files[file_view].cursor_row - open_files[file_view].scr_row);

    scr_cursor(open_files[file_view].cursor_row - open_files[file_view].scr_row, 
      open_files[file_view].cursor_col - open_files[file_view].scr_col);
  }
  while((c = getkeypress()) == TEXTEDIT_KEY_BACKSPACE && open_files[file_view].cursor_col);

  fseek(open_files[file_view].fp, pos-n, SEEK_SET);

  append_tempfile(fp, 0, tempfilename);
  draw_screen();

  return c;
}

int delete_chars(void)
{
  char *tempfilename;
  char buff[INSERT_BUFF];
  unsigned long pos = seek_cursor_pos();
  FILE *fp = tr_file(&tempfilename, buff);
  int c;
  unsigned n = 0;

  do
  {
    if(n == INSERT_BUFF - 10)
//      return TEXTEDIT_KEY_DELETE;
      break;
    else
    {
      n++;
      print_status();
    }

    print_line(open_files[file_view].cursor_col - open_files[file_view].scr_col,
      open_files[file_view].cursor_row - open_files[file_view].scr_row,
      &buff[n], 0x7);

    if(open_files[file_view].syntaxhighlight)
      syntax_highlight(open_files[file_view].cursor_row - open_files[file_view].scr_row);

    scr_cursor(open_files[file_view].cursor_row - open_files[file_view].scr_row,
      open_files[file_view].cursor_col - open_files[file_view].scr_col);
  }
  while((c = getkeypress()) == TEXTEDIT_KEY_DELETE && open_files[file_view].cursor_col + n
    < open_files[file_view].screen_lines[open_files[file_view].cursor_row - open_files[file_view].scr_row].length);

  fseek(open_files[file_view].fp, pos, SEEK_SET);

  append_tempfile(fp, n, tempfilename);

  draw_screen();
  return c;
}

void add_newline(void)
{
  unsigned long pos = seek_cursor_pos();
  char *tempfilename;
  FILE *fp = tr_file(&tempfilename, 0);

  fseek(open_files[file_view].editfp, pos, SEEK_SET);
  fputc('\n', open_files[file_view].editfp);
  if(ftell(open_files[file_view].editfp) == pos+2)
    open_files[file_view].longnewlines = 1;

  append_tempfile(fp, 0, tempfilename);

  open_files[file_view].cursor_col = 0;
  open_files[file_view].cursor_row++;

  open_files[file_view].lines++;

  scr_cursor(open_files[file_view].cursor_row - open_files[file_view].scr_row, 
    open_files[file_view].cursor_col - open_files[file_view].scr_col);

  draw_screen();
}

void remove_newline(void)
{
  unsigned long pos;
  char *tempfilename;
  FILE *fp;

  if(open_files[file_view].cursor_row == open_files[file_view].scr_row)
  {
    dec_view_row();
    print_file_screen();
  }

  else if(open_files[file_view].cursor_row == open_files[file_view].scr_row + scr_rows - 1)
  {
    inc_view_row();
    print_file_screen();
  }

  pos = seek_cursor_pos();
  fp = tr_file(&tempfilename, 0);

  if(open_files[file_view].longnewlines)
    fseek(open_files[file_view].editfp, pos-2, SEEK_SET);
  else
    fseek(open_files[file_view].editfp, pos-1, SEEK_SET);

  append_tempfile(fp, 0, tempfilename);

  open_files[file_view].cursor_row--;
  open_files[file_view].cursor_col = open_files[file_view].screen_lines[open_files[file_view].cursor_row - open_files[file_view].scr_row].length;

  open_files[file_view].lines--;

  draw_screen();

  scr_cursor(open_files[file_view].cursor_row - open_files[file_view].scr_row, 
    open_files[file_view].cursor_col - open_files[file_view].scr_col);
}

int show_search_result(unsigned long pos, unsigned long row)
{
  char foundmsg[100];

  if(row < open_files[file_view].lines - scr_rows + 1)
    open_files[file_view].scr_row = row;
  else
    open_files[file_view].scr_row = open_files[file_view].lines - scr_rows + 1;
  open_files[file_view].cursor_row = row;
  draw_screen();

  sprintf(foundmsg, "Found on line %lu - press F3 to search again", row);
  status_line(foundmsg);

  if(getkeypress() == TEXTEDIT_KEY_F3)
    return 1;
  return 0;
}

void search_ndl(char *ndl)
{
  unsigned n = 0;
  int c;
  unsigned long row = 0;
  unsigned long old_pos = open_files[file_view].file_pos;
  unsigned long old_row = open_files[file_view].scr_row;
  unsigned long mem_pos;

  rewind(open_files[file_view].fp);
  while(!feof(open_files[file_view].fp))
  {
    if((c = fgetc(open_files[file_view].fp)) == ndl[n])
    {
      if(!ndl[++n])
      {
        mem_pos = ftell(open_files[file_view].fp);
        if(!show_search_result(mem_pos, row))
          return;
        fseek(open_files[file_view].fp, mem_pos, SEEK_SET);
      }
    }
    else n = 0;

    if(c == '\n')
    {
      row++;
      if(row < open_files[file_view].lines - scr_rows + 1)
        open_files[file_view].file_pos = ftell(open_files[file_view].fp);
    }
  }

  open_files[file_view].file_pos = old_pos;
  open_files[file_view].scr_row = old_row;

  status_line("Not found - press any key");
  getkeypress();

  return;
}

void get_input(char *str, unsigned len)
{
  unsigned n = 0;
  int c;

  str[0] = 0;

  scr_cursor(scr_rows-1, 0);

  getinput:while((c = getkeypress()) >= 0 && n < len)
  {
    print_char(n, scr_rows-1, c, 0x7);
    str[n++] = c;
    str[n] = 0;
    scr_cursor(scr_rows-1, n);
  }

  if(c == TEXTEDIT_KEY_BACKSPACE)
  {
    if(!n) goto getinput;
    str[--n] = 0;
    print_char(n, scr_rows-1, ' ', 0x7);
    scr_cursor(scr_rows-1, n);
    goto getinput;
  }
}

#define SEARCH_LEN 100
void search_word(void)
{
  char ndl[SEARCH_LEN+1];

  clear_screen();
  print_line(0, scr_rows-2, "Enter a search string:", 0x7);
  get_input(ndl, SEARCH_LEN);
  search_ndl(ndl);
}

#define ROW_LEN 20
void jump_to_row(void)
{
  char row[ROW_LEN+1];
  unsigned n = 0;

  clear_screen();
  print_line(0, scr_rows-2, "Jump to row:", 0x7);
  get_input(row, ROW_LEN);

  if((open_files[file_view].scr_row = atoi(row)) > open_files[file_view].lines - scr_rows + 1)
  {
    open_files[file_view].scr_row = open_files[file_view].lines - scr_rows + 1;
  }

  rewind(open_files[file_view].fp);
  while(n < open_files[file_view].scr_row)
  {
    if(fgetc(open_files[file_view].fp) == '\n')
      n++;
  }
  open_files[file_view].file_pos = ftell(open_files[file_view].fp);
}

void open_file_name(void)
{
  char filename[FILENAME_LEN+1];
  unsigned n = 0;

  clear_screen();
  print_line(0, scr_rows-2, "Enter file name:", 0x7);
  get_input(filename, FILENAME_LEN);

  if(!open_file(filename))
  {
    status_line("File opened successfully");
    file_view = file_count - 1;
  }
  else
  {
    status_line("Error while opening file - press any key");
    getkeypress();
  }
}

char *declwords[] =
{
  "char",
  "int",
  "short",
  "long",
  "extern",
  "static",
  "volatile",
  "inline",
  "float",
  "struct",
  "unsigned",
  "signed",
  "void",
  "union",
  "register",
  "enum",
  "double",
  "auto",
  "const",
  "sizeof",
  "typedef",
  0
};

char *condwords[] =
{
  "if",
  "switch",
  "do",
  "while",
  "for",
  "else",
  0
};

char *jumpwords[] =
{
  "goto",
  "case",
  "return",
  "default",
  "continue",
  "break",
  0
};

char spaces[] =
{
  ' ',
  ',',
  '.',
  '(',
  ')',
  '[',
  ']',
  '{',
  '}',
  '<',
  '>',
  '!',
  ';',
  ':',
  '\'',
  '"',
  '+',
  '-',
  '*',
  '/',
  '&',
  '^',
  '|',
  '=',
  '~',
  0
};

unsigned is_number(char *word)
{
  if(*word >= 0x30 && *word <= 0x39)
    return strlen(word);
  return 0;
}

unsigned word_in_array(char **array, char *word)
{
  unsigned n = 0;

  do
    if(!strcmp(array[n], word))
      return strlen(word);
  while(array[++n]);

  return 0;
}

#define WORD_LEN 100
char *get_word(unsigned x, unsigned y)
{
  static char retval[WORD_LEN+1];
  unsigned n = 0;
  unsigned char chr;

  retval[0] = 0;

  while(n < WORD_LEN && !strchr(spaces, chr = get_char(x+n, y)))
  {
    retval[n++] = chr;
    retval[n] = 0;
  }

  return retval;
}

unsigned is_scomment(unsigned x, unsigned y)
{
  if(get_char(x, y) == '/')
    if(get_char(++x, y) == '/')
      return 1;
  return 0;
}

unsigned is_mlcomment(unsigned x, unsigned y)
{
  if(get_char(x, y) == '/')
    if(get_char(++x, y) == '*')
      return 1;
  return 0;
}

void syntax_highlight(int line)
{
  unsigned row = 0;
  unsigned col;
  unsigned char chr;
  unsigned in_dquotes = 0;
  unsigned in_squotes = 0;
  unsigned in_comment = 0;
  unsigned c;
  char *word;

  if(line > 0)
    row = line;

  do
  {
    col = 0;
    do
    {
      chr = get_char(col, row);

      if(!in_comment && !in_squotes && chr == '"')
      {
        if(!(in_dquotes = ~in_dquotes))
          col++;
      }
      if(!in_comment && !in_dquotes && chr == '\'')
      {
        if(!(in_squotes = ~in_squotes))
          col++;
      }
      if(in_dquotes || in_squotes)
      {
       	set_color(col, row, 0xE, 3);
        if(chr == '\\')
        {
          col++;
        }
        col++;
        continue;
      }

      if(is_mlcomment(col, row))
      {
        in_comment = 1;
      }
      if(in_comment)
      {
        set_color(col, row, 0x9, 2);
        if(chr == '*')
          if(get_char(col + 1, row) == '/')
          {
            in_comment = 0;
            col += 2;
            continue;
          }
        col++;
        continue;
      }

      if(is_scomment(col, row))
      {
        set_color(col, row, 0x9, scr_cols - col - 1);
        break;
      }

      if(get_char(col, row) == '#')
      {
        set_color(col, row, 0xC, scr_cols - col - 1);
        break;
      }

      if(c = is_number(word = get_word(col, row)))
      {
        set_color(col, row, 0x0B, c);
        col += c;
      }

      else if(c = word_in_array(declwords, word))
      {
        set_color(col, row, 0x02, c);
        col += c;
      }
      else if(c = word_in_array(condwords, word))
      {
        set_color(col, row, 0xF, c);
        col += c;
      }
      else if(c = word_in_array(jumpwords, word))
      {
        set_color(col, row, 0x6, c);
        col += c;
      }
      else if(*word) col += strlen(word);
      else
      {
        if(strchr(spaces, chr))
          set_color(col, row, 0x09, 1);
        col++;
      }
    }
    while(col < scr_cols - 1);
  }
  while(line < 0 && ++row < scr_rows - 1);
}

