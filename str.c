/*unsigned int farstrlen(char __far *str)
{
  unsigned int retval=0;
  while(*str++)
    retval++;
  return retval;
}*/

//#define strlen(str) farstrlen(str)

/*unsigned str_find_char(char *str, char needle)
{
  while(*str)
    if(*str++ == needle)
      return 1;
  return 0;
}*/

static char *str_char_lastocc(char *str, char needle)
{
  str += strlen(str);
  while(*str != needle)
    str--;
  return str;
}

/*char __far *farstrcpy(char __far *dest, char __far *src)
{
  char *retval = dest;
  while(*dest++ = *src++);
  return retval;
}

void __far *farmemset(void __far *dest, int c, size_t n)
{
  void __far *retval = dest;
  unsigned char __far *bdest = dest;
  while(n--)
    *bdest++ = c;
  return retval;
}

void __far *farmemcpy(void __far *dest, void __far *src, size_t n)
{
  void __far *retval = dest;
  unsigned char __far *bdest = dest;
  unsigned char __far *bsrc = src;
  while(n--)
    *bdest++ = *bsrc++;
  return retval;
}

int farstrcmp(char __far *str1, char __far *str2)
{
  int retval = 0;
  while(*str1 && *str2)
    if((retval = (*str1++ - *str2++)))
      break;
  return retval;
}

int farstrncmp(char __far *str1, char __far *str2, size_t n)
{
  int retval = 0;
  while(*str1 && *str2 && n--)
    if((retval = (*str1++ - *str2++)))
      break;
  return retval;
}

char __far *farstrcat(char __far *dest, char __far *src)
{
  char __far *retval = dest;
  while(*dest++);
  while(*dest++ = *src++);
  return retval;
}

#define strcpy(str1, str2) farstrcpy(str1, str2)
#define memset(dest, c, n) farmemset(dest, c, n)
#define memcpy(dest, src, n) farmemcpy(dest, src, n)
#define strcmp(str1, str2) farstrcmp(str1, str2)
#define strncmp(str1, str2, n) farstrncmp(str1, str2, n)
#define strcat(str1, str2) farstrcat(str1, str2)*/
