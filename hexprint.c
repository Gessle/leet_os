#include <stdio.h>
#include <stdlib.h>

void main(int argc, char *argv[])
{
  FILE *fp;
  int c;
  if(argc==2)
  {
    fp = fopen(argv[1], "r");
    while((c = fgetc(fp)) != -1)
    {
      printf("%x ", c);
    }
    fclose(fp);
  }
}
