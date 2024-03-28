/*
  This function makes sound
*/

void make_sound(unsigned int snd)
{
//  unsigned n;
  switch(snd)
  {
    // short beep
    case 1:
      sound(500);
      shortdelay(16000);
//      shortdelay(999999);
      nosound();
    return;
    // alarm
/*    case 2:
      n = 20;
      while(n--)
      {
        sound(440);
        shortdelay(48000);
        sound(330);
        shortdelay(48000);
      }
      nosound();
    return;*/
  }
}
