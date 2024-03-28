/*
  Returns one path from PATH environment variable
  This function is meant to be called in a loop, where the counter starts 
  from 0. The counter variable is passed to this function as a second argument
*/

static char *get_paths(char *pathenv, unsigned int pathnum)
{
  unsigned int n=0;
  char *retval = malloc(67*sizeof(char));

  if(!retval)
  {
    nomemory();
    return 0;
  }

  if(!pathenv)
    return 0;

  while(pathnum--)
  {
    if(pathenv[n] == ';')
      pathenv += n+1;
    if(!pathenv[n] || !*pathenv)
    {
      free(retval);
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
