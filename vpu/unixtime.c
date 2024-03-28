/*struct unixtime
{
  posixtime time;
  unsigned long nanoseconds;
};*/

static unsigned long human2unixtime(unsigned short year, unsigned char month, unsigned char day, unsigned char hour,
  unsigned char min, unsigned char sec)
{
  unsigned char month_lengths[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
  unsigned n;
  unsigned long retval;
  //    *** This will cease to work at Sunday, February 7, 2106 6:28:15 AM GMT time! ***
  //    *** Remember to change the datatypes to 64-bit before that! ***
  // days since epoch at the beginning of the current year
//  retval = (unsigned long)(((year-1970)* 365) + ((year-1)-1968)/4);
  retval = (unsigned long)(((year-1970)* 365) + (((year-1)-1968)>>2));
  // + days since the beginning of the current year
  for(n=month-1;n--;)
  {
    retval += month_lengths[n];
    // add current year's leap day
//    if(n==1 && !((year-1972)%4))
    if(n==1 && !((year-1972)&3))
      retval++;
  }
  retval += day-1;

  // seconds from epoch at the beginning of the current day
  retval *= 86400;
  // + seconds since the beginning of the current day
  retval += (unsigned long)hour*60*60 + min*60 + sec;

  return retval;
}

struct unixtime unixtime(long offset)
{
  struct int21date *date = dosdate();
  struct int21time *time = dostime();
  struct unixtime retval;

  retval.time = human2unixtime(date->y, date->m, date->d, time->h, time->m, time->s);

  retval.time -= offset;

  retval.nanoseconds = ((unsigned)time->ss * 10000000 );

  return retval;
}

