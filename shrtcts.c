inline void default_icon_program(char icon)
{
  typedef struct prog_window (*generic_fp)(void);
  
  static const struct prog_window (*icon_function[])() =
  {
    (generic_fp)init_run_window,
    (generic_fp)init_fileman_window,
    (generic_fp)init_cpanel_window,
    (generic_fp)init_clock_window
  };

  if(icon)
    (*icon_function[icon])();
  else
    (*icon_function[icon])(0);    
}
