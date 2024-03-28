#include <stdio.h>
#include <stdlib.h>
#include <dos.h>


char *get_paths(char *pathenv, unsigned int pathnum)
{
  unsigned int n=0;
  static char retval[67];

  if(!pathenv)
    return 0;

  while(pathnum--)
  {
    if(pathenv[n] == ';')
      pathenv += n+1;
    if(!pathenv[n] || !*pathenv)
    {
      return 0;
    }
    for(n=0;pathenv[n] && pathenv[n]!=';';n++)
    {
      retval[n] = pathenv[n];
    }
    retval[n] = 0;
  }

  return retval;
}

void main( unsigned int argc, char *argv[] ) 

{ 
    unsigned int n=0;
    char *path;
    char *pathr;

    path = getenv( "PATH" );
    printf("%s\n", path);

    for(;;n++)
    {
      pathr = get_paths(path, n);
    printf("0x%p %Ws\n", pathr, pathr);
    delay(1000);
    }

} 


