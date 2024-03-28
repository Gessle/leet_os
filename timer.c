extern void __far * __far old_timer_rtn;

volatile extern unsigned __far clock_ticks;

void __interrupt __far timer_rtn(void);

inline void set_timer_rtn(void)
{
  old_timer_rtn = _dos_getvect(0x1C);
  _dos_setvect(0x1C, timer_rtn);
}

inline void reset_timer_rtn(void)
{
  _dos_setvect(0x1C, old_timer_rtn);
}

