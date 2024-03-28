#define FILENAME_LEN 256

struct line
{
  unsigned long file_offset;
  unsigned long length;
};

struct file
{
  unsigned long cursor_row;
  unsigned long cursor_col;
  unsigned long scr_row;
  unsigned long scr_col;
  unsigned editmode:1;
  unsigned syntaxhighlight:1;
  unsigned invisiblechars:1;
  unsigned longnewlines:1;
  char filename[FILENAME_LEN+1];
  char editfilename[13];
  FILE *fp;
  FILE *editfp;
  FILE *tempfp;
  unsigned long file_pos;
  unsigned long lines;
  struct line *screen_lines;
  unsigned long length;
};
