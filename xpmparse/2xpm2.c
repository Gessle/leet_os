#include <stdio.h>
#include <stdlib.h>

/*
  Convert XPM3 image to XPM2 image
*/

int main(unsigned argc, char **argv)
{
  FILE *fp;
  FILE *output;
  unsigned inside_quotes = 0;
  int c;

  if(argc == 3)
  {
    fp = fopen(argv[1], "r");
    if(!fp)
      return 1;
    output = fopen(argv[2], "w");
    if(!output)
      return 2;
  }
  else
  {
    puts("This program converts XPM3 images to XPM2 images.\nUsage: $ 2XPM2 [SRCFILE] [DESTFILE]");
    return 0;
  }

  fputs("! XPM2\n", output);

  while(!feof(fp))
  {
    c = fgetc(fp);    
    if(c == '"')
    {
      if(inside_quotes)
        fputc('\n', output);
      inside_quotes = ~inside_quotes;
      continue;
    }
    if(inside_quotes)
      fputc(c, output);      
  }

  fclose(output);
  fclose(fp);

  return 0;
}

