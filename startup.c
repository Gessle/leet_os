static void run_startup_programs(void)
{
  struct find_t fileinfo;
  char startup_dir[] = "startup\\";
  char path[MAX_PATH+1];

  sprintf(path, "%s*.*", startup_dir);
  
  if(!_dos_findfirst(path, _A_NORMAL, &fileinfo))
    do
    {
      sprintf(path, "%s%s", startup_dir, fileinfo.name);
      run_program(0, path, 0, 0, 0xFFFF);
    }
    while(!_dos_findnext(&fileinfo));
}
