/*
  Starts a screen saver
*/

static void screen_saver(void)
{
  char screensaver_path[22];
  sprintf(screensaver_path, "SCRNSVRS/%s", screensaver_program);
  run_program(0, screensaver_path, 0, 0, 0xFFFF);
  screensaver_timer = 0;
}
