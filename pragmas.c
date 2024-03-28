inline void assert_msg(char *msg, char *file, unsigned line)
{
#ifdef ASSERT_ON
  char assert_msg[120];
  sprintf(assert_msg, "Assertion failed (file %s, line %d): %s", file, line, msg);
  show_error_msg(assert_msg);
#endif
}

#define debugmsg(x, a) ((!(x)) ? (assert_msg(a, __FILE__, __LINE__), 0) : 1)

void __far * __far copymem(void __far *dst, const void __far *src, unsigned count);
#pragma aux copymem parm [es di] [ds si] [cx] value [ds si];

// safe memcpy for planar display memory
void copybytes(void __far *dst, const void __far *src, unsigned count);
#pragma aux copybytes = \
  "pushf" \
  "cld" \
  "rep movsb" \
  "popf" \
  parm [es di] [ds si] [cx];

/*void __far * __far setmem(void __far *dst, const unsigned char byte, unsigned count);
#pragma aux setmem parm [es di] [ax] [cx] value [es di];

unsigned __far lenstr(const char __far *str);
#pragma aux lenstr parm [es di] value [ax];

void __far * __far copystr(char __far *dst, const char __far *src);
#pragma aux copystr parm [es di] [ds si] value [es di];

#define memcpy(a, b, c) copymem(a, b, c)
#define strlen(a) lenstr(a)
#define strcpy(a, b) copystr(a, b)
#define memset(a, b, c) setmem(a, b, c)*/

// this "memcpy" is safe when copying overlapping data from higher to lower address
//#define memmovel(a, b, c) copymem(a, b, c)
#define memmovel(a, b, c) memmove(a, b, c)

//#define memmovel(a, b, c) farmemcpy(a, b, c)
//#define memcpy(a, b, c) farmemcpy(a, b, c)
