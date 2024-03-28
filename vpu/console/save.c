char error_while_opening[] = "Error while opening file";

static void save_asm(char *filename)
{
  static char *pfilename = 0;
  unsigned n = 0;
  FILE *fp = fopen(filename, "r");
  if(!fp || (pfilename && !strcmp(pfilename, filename)))
  {
    fp = fopen(filename, "w");
    if(!fp)
    {
      putstr(error_while_opening);
      return;
    }
    while(n < asm_content->count)
    {
      if(n) fputc('\n', fp);
      fputs(asm_content->asm_code[n++], fp);
    }
    fclose(fp);
    free(pfilename);
    pfilename = 0;
  }
  else
  {
    fclose(fp);
    pfilename = malloc((strlen(filename)+1)*sizeof(char));
    if(!pfilename)
    {
      putstr(outofmemory);
      return;
    }
    strcpy(pfilename, filename);
    putstr("File exists! Repeat save command to overwrite.");
  }
}

static void load_asm(char *filename)
{
  void *new_pointer;
  unsigned n;
  int c;
  FILE *fp = fopen(filename, "r");
  if(!fp)
  {
    putstr(error_while_opening);
    return;
  }
  while(asm_content->count)
    free(asm_content->asm_code[--asm_content->count]);
//  asm_content->count = 0;
  do
  {
    new_pointer = realloc(asm_content->asm_code, (asm_content->count+1) * sizeof(char*));
    if(!new_pointer)
    {
      putstr(outofmemory);
      return;
    }
    asm_content->asm_code = new_pointer;
    asm_content->asm_code[asm_content->count] = calloc(SCR_COLS+1, sizeof(char));
    if(!asm_content->asm_code[asm_content->count])
    {
      putstr(outofmemory);
      return;
    }
//    fgets(asm_content.asm_code[asm_content.count-1], SCR_COLS+1, fp);
    n = 0;
    while( n < SCR_COLS-1 && (c = fgetc(fp)) != '\n' && c != -1)
    {
      asm_content->asm_code[asm_content->count][n++] = c;
    }
  }
  while(++asm_content->count && c != -1);
  fclose(fp);
}
