//#define DEBUG_ON

#ifdef DEBUG_ON
void debug_exit(int status);
#define exit(x) debug_exit(x)
void *debug_malloc(unsigned count);
void *debug_calloc(unsigned count, unsigned bsize);
void *debug_realloc(void __far *ptr, unsigned size);
void debug_free(void __far *ptr);
void *debug_salloc(unsigned count);
void *debug_memset(void __far *str, int c, unsigned count);
void *debug_memcpy(void __far *dest, const void __far *src, unsigned count);
char *debug_strcpy(char __far *dest, const char __far *src);
char *debug_strncpy(char __far *dest, const char __far *src, unsigned count);

void debug_callstack_push(char *function, unsigned len);
void debug_callstack_pop(void);

#define malloc(size) debug_malloc(size)
#define calloc(count, bsize) debug_calloc(count, bsize)
#define realloc(ptr, size) debug_realloc(ptr, size)
#define free(ptr) debug_free(ptr)
#define memset(str, c, count) debug_memset(str, c, count)
#define memcpy(dest, src, count) debug_memcpy(dest, src, count)
#define strcpy(dest, src) debug_strcpy(dest, src)
#define strncpy(dest, src, count) debug_strncpy(dest, src, count)

#endif
