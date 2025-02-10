/*
  Starts a screen saver
*/

static void screen_saver(void)
{
  char screensaver_path[22];
  sprintf(screensaver_path, "SCRNSVRS/%s", screensaver_program);
  run_program(0, screensaver_path, 0, 0, 0xFFFF);
  zero_screensaver_timer();
}

void power_zero_counters(void);
#pragma aux power_zero_counters = \
  "mov ax, 0x5401" \
  "int 0x2F" \
  modify [ax];

static void zero_screensaver_timer(void)
{
  screensaver_timer = 0;
  if(power_manager_status.power_loaded)
    power_zero_counters();
}
