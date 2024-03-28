void (__interrupt __far *prev_int_23)(void);

/*
Sets the original 0x23 interrupt
*/
static void reset_int23(void)
{
  _dos_setvect( 0x23, prev_int_23 );
}

// break key handler
static void __interrupt int23handler(void)
{
  ungetch(3);
  outbyte(0x20, 0x20);
}

static void set_int23(void)
{
  prev_int_23 = _dos_getvect( 0x23 ); 
  _dos_setvect(0x23, int23handler);
}
