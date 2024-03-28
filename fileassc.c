/*
Returns a path to the program that is used to open files whose name end with
*suffix
*/
static char *get_file_assoc(char *suffix)
{
  FILE *fp;
  static char buffer[86];
  unsigned int n;

  if(!(fp = fopen("fileassc.dat", "r")))
    return 0;
  while(!feof(fp))
  {
    for(n=0;n<86;n++)
    {
      buffer[n] = fgetc(fp);
      if(buffer[n] == '\n' || feof(fp))
      {
        buffer[n] = 0;
        break;
      }      
    }
    for(n=0;n<3;n++)
      if(suffix[n] != buffer[n])
        break;
    if(!suffix[n] && buffer[n] == ' ')
    {
      fclose(fp);
      return &buffer[n+1];
    }    
  }
  fclose(fp);
  return 0;
}
