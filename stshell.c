#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <malloc.h>
#include <process.h>
#include <direct.h> 
#include <string.h>

#include "dosexec.c" 

void main()
{
  char compath[81];
  char comargs[128];
  char shellpath[81];
  char *env = malloc(2048*sizeof(char));;
  unsigned n = 0;
  unsigned m;
  int c;

  FILE *fp = fopen("shell.pth", "r");

  if(!fp)
  {
    printf("Cannot open file SHELL.PTH !\n");
    return;
  }

  while((c = fgetc(fp)) != '\n')
    compath[n++] = c;
  compath[n] = 0;
  n=0;
  while((c = fgetc(fp)) != '\n')
    comargs[n++] = c;
  comargs[n] = 0;
  n=0;
  while((c = fgetc(fp)) != -1 && c != '\n')
    shellpath[n++] = c;
  shellpath[n] = 0;
  fclose(fp);

  fp = fopen("envs.txt", "r");
  if(fp)
  {
    while(!feof(fp))
    {
      n=0;
      while((c = fgetc(fp)) != '\n' && c != -1)
      {
        if(c == '=')
        {
          env[n++] = 0;
          m = n;
        }
        else
          env[n++] = c;
      }
      env[n] = 0;
      setenv(env, &env[m], 0);
    }
    fclose(fp);
  }

  // run autoexec.bat first
  c = dosexec(compath, comargs);
  if(c)
    printf("Could not run command interpreter! Error code: %X\n", c);

  // then start freedosshell
  chdir(shellpath);
  _chdrive(shellpath[0] - 'A' + 1);
  execle("MAIN.EXE", "MAIN.EXE", 0, 0);
  printf("Error %X while trying to start shell\n", errno);
}
