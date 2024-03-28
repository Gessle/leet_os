unsigned str_find_char(char *str, char needle);
char *str_char_lastocc(char *str, char needle);
char __far *farstrcpy(char __far *dest, char __far *src);
void __far *farmemset(void __far *dest, int c, size_t n);
void __far *farmemcpy(void __far *dest, void __far *src, size_t n);
int farstrcmp(char __far *str1, char __far *str2);
int farstrncmp(char __far *str1, char __far *str2, size_t n);
char __far *farstrcat(char __far *dest, char __far *src);
unsigned int farstrlen(char __far *str);

#define strlen(str) farstrlen(str)
#define strcpy(str1, str2) farstrcpy(str1, str2)
#define memset(dest, c, n) farmemset(dest, c, n)
#define memcpy(dest, src, n) farmemcpy(dest, src, n)
#define strcmp(str1, str2) farstrcmp(str1, str2)
#define strncmp(str1, str2, n) farstrncmp(str1, str2, n)
#define strcat(str1, str2) farstrcat(str1, str2)
