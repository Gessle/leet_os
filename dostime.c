/*struct int21time
{
  unsigned char h;
  unsigned char m;
  unsigned char s;
  unsigned char ss;
};

struct int21date
{
  unsigned short int y;
  unsigned char m;
  unsigned char d;
  unsigned char wd;
};*/

struct int21time *dostime(void)
{
  static struct int21time retval;
/*  unsigned char *h = &retval.h;
  unsigned char *m = &retval.m;
  unsigned char *s = &retval.s;
  unsigned char *ss = &retval.ss;

  __asm
  {
    mov ah, 0x2C
    int 0x21
    mov *h, ch
    mov *m, cl
    mov *s, dh
    mov *ss, dl
  }*/
  union REGS regs;
  regs.h.ah = 0x2C;
  int86(0x21, &regs, &regs);
  retval.h = regs.h.ch;
  retval.m = regs.h.cl;
  retval.s = regs.h.dh;
  retval.ss = regs.h.dl;

  return &retval;
}

struct int21date *dosdate(void)
{
  static struct int21date retval;
/*  unsigned short int *y = &retval.y;
  unsigned char *m = &retval.m;
  unsigned char *d = &retval.d;
  unsigned char *wd = &retval.wd;*/

/*  __asm
  {
    mov ah, 0x2A
    int 0x21
    mov *y, cx
    mov *m, dh
    mov *d, dl
    mov *wd, al
  }*/

  union REGS regs;
  regs.h.ah = 0x2A;
  int86(0x21, &regs, &regs);
  retval.y = regs.x.cx;
  retval.m = regs.h.dh;
  retval.d = regs.h.dl;
  retval.wd = regs.h.al;

  return &retval;
}

unsigned char *weekdays[] = {
  "sun", "mon", "tue", "wed", "thu", "fri", "sat"
  };

//unsigned char *months[] = {
//  "jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec"
//}

/*
This function returns a pointer to a string that contains date and time in
human-readable format
*/

unsigned char *human_date(void)
{
  static unsigned char retval[80];
  struct int21date *datep = dosdate();
  struct int21time *timep = dostime();
/*  unsigned int n=0;
  unsigned char space = ' ';
  unsigned char *day = datep->d;
  unsigned char *month = datep->m;
  unsigned char *year = datep->y;
  unsigned char *retday10;
  unsigned char *retday1;
  unsigned char *retmon10;
  unsigned char *retmon1;
  unsigned char *retyear1000;
  unsigned char *retyear100;
  unsigned char *retyear10;
  unsigned char *retyear1;
*/

  sprintf(retval, "%s %u. %u. %u - %i:%.2i:%.2i", weekdays[datep->wd], datep->d, datep->m, datep->y, timep->h, timep->m, timep->s);
//  sprintf(retval, "%u. %u. %u - %i:%.2i:%.2i", datep->d, datep->m, datep->y, timep->h, timep->m, timep->s);
/*  while(weekdays[datep->wd][n])
    retval[n] = weekdays[datep->wd][n++];
  retval[n++] = space;
  
  retday10 = &retval[n++];
  retday1 = &retval[n++];
  
  retval[n++] = space;

  retmon10 = &retval[n++];
  retmon1 = &retval[n++];

  retval[n++] = space;

  retyear1000 = &retval[n++];
  retyear100 = &retval[n++];
  retyear10 = &retval[n++];
  retyear1 = &retval[n++];  
  
  __asm
  {
    mov ax, *day
    div 10
    mov retday10, al
    mov retday1, ah
    mov ax, *month
    div 10
    mov retmon10, al
    mov retmon1, ah
    mov ax, *year
    div 1000
    mov retyear1000, *retyear1000
    
  }
 */

 return retval;
}

/*

*/

void shortdelay(unsigned long interval)
{
  union REGPACK regs;
  regs.h.ah = 0x86;
  regs.x.cx = interval>>16;
  regs.x.dx = interval;
  intr(0x15, &regs);
}
