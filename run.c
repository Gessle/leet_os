static char *find_executable_2(char *program)
{
  unsigned int n;
  char *pathp;
//  char *programpath = malloc(MAX_PATH*sizeof(char));
  static char programpath[MAX_PATH];
  struct find_t   fileinfo;
  if(!programpath) return 0;
  // if found in current working directory, return normally
  if(!_dos_findfirst(program, _A_NORMAL, &fileinfo))
  {
    sprintf(programpath, "%s", program);
    return programpath;
  }
  // else search from PATH
  for(n=1;;n++)
  {
    if(!(pathp = get_paths(path, n)))
    {
//      free(programpath);
      return 0;
    }
    sprintf(programpath, "%s\\%s", pathp, program);
    free(pathp);
    if(!_dos_findfirst(programpath, _A_NORMAL, &fileinfo))
      return programpath;
  }
}

/*
This function searches executable files from $PATH

Arguments: a pointer to a string containing the filename of the program executable

Returns: Pointer to a string containing full path to the executable file
The memory allocated to the pointer must be freed
*/
char *find_executable(char *program, unsigned bytecodeprog)
{
  char filename[13];
  
  if(bytecodeprog)
    if(find_executable_2(program))
      return find_executable_2(program);

//  if(!str_find_char(program, '.'))
  if(!strchr(program, '.'))
  {
    if(strlen(program) > 8) return 0;
    
    sprintf(filename, "%s%s", program, comsuf);
    if(find_executable_2(filename))
      return find_executable_2(filename);
    sprintf(filename, "%s%s", program, exesuf);
    if(find_executable_2(filename))
      return find_executable_2(filename);
    sprintf(filename, "%s%s", program, appsuf);
    if(find_executable_2(filename))
      return find_executable_2(filename);      
    sprintf(filename, "%s%s", program, batsuf);
      return find_executable_2(filename);
  }
  else
  {
    return find_executable_2(program);
  }
}

/*
Prompts the user to press any key...
*/
static void run_pressakey(void)
{
  printf("Press any key to return to shell.\n");
  getch();
}

/*
Changes the drive to argument drive
If !drive, ignores the first argument and changes back to initial working drive
*/
int chdrive(unsigned drive)
{
  unsigned total;
  static int olddrive = -1;

  if(drive)
  {
/*    drive-=64;
    if(drive > 32)
      drive-=32;*/
    drive &= 0x1F;
    if(olddrive != drive) 
    {
      _dos_setdrive(drive, &total);
      _dos_getdrive(&total);
      if(drive != total)
        return 1;
      olddrive = drive;    
    }
  }
  else
  {
//    _dos_getdrive(&total);
//    if(total != cwdrive)
    if(olddrive != cwdrive)
    {
      _dos_setdrive(cwdrive, &total);
      olddrive = cwdrive;
    }
  }
  return 0;
}

char **split_args(unsigned *argc, char *args, char *programname)
{
  char **retval = calloc(++*argc, sizeof(char *));
  unsigned n;
  unsigned arg_len;
  void *new_pointer;

  if(!retval)
  {
    nomemorynoargs:
    --*argc;
    return 0;
  }

  retval[0] = calloc(strlen(programname)+1, sizeof(char));
  if(!retval[0]) goto nomemorynoargs;
  strcpy(retval[0], programname);

  if(!args)
    return retval;
  while(1)
  {
    while(*args && *args <= 0x20) args++;    
    if(!*args) break;

    new_pointer = realloc(retval, (*argc + 1)* sizeof(char *));
    if(!new_pointer)
      goto outofmemory;
    else
      retval = new_pointer;        
              
    for(arg_len=0;args[arg_len] && args[arg_len] != ' ';arg_len++);
    
    retval[*argc] = calloc(arg_len+1, sizeof(char));
    if(!retval[*argc])
    {
      outofmemory:
      while((*argc)--)
        free(retval[*argc]);
      free(retval);
      return 0;
    }
    n=arg_len;
    while(n--)
      retval[*argc][n] = args[n];
    args+=arg_len;
    ++*argc;
  }
  return retval;
}

/*
Runs a program
Arguments: Working directory, path to the program executable, arguments to the
program, pointer to the mouse status, unsigned getkey*, privileges of the new process

*if getkey != 0, prompts the user to press any key after executing the program
*/
void *run_program(char *dir, char *program, char *args, unsigned getkey, unsigned privileges)
{
  char errormsg[128];
  int c;
  unsigned char *found_program = 0;
  struct prog_window *return_vpuwin;
  unsigned total_drives;

  // if there is no backslashes in the program name, search for program executable from PATH
  if(!strchr(program, '\\'))
  {
    sprintf(errormsg, "Cannot find %s", program);
    found_program = find_executable(program, 0); 
    if(!found_program)
    {
      show_error_msg(errormsg);
      goto end;
    }
    program = found_program;
  }

  if(!checksuff(program, appsuf))
  {
    if(return_vpuwin = init_vpuconsole_window(program))
    {
      ((struct console_struct *)return_vpuwin->function_status)->vcpu->argv
         = split_args(&((struct console_struct *)return_vpuwin->function_status)->vcpu->argc, args, program);
        getcwd(((struct console_struct *)return_vpuwin->function_status)->vcpu->cwd, MAX_PATH);
      if(dir)
        append_path(((struct console_struct*)return_vpuwin->function_status)->vcpu, 
                    ((struct console_struct*)return_vpuwin->function_status)->vcpu->cwd, dir);
      ((struct console_struct *)return_vpuwin->function_status)->vcpu->privileges = privileges;
      return return_vpuwin->function_status;
    }
    return 0;
  }

  if(!(privileges & VPU_PRIV_SYSTEM)) // no privileges to run DOS program
    return 0;
  
  reset_vpu_interrupt_handlers();
  reset_dosidle_handler();
  
  // if *dir is set, change working directory to it
  if(dir && dir[0])
  {
    // if the second character is :, change drive to dir[0]
    if(dir[1] == ':')
//      chdrive(dir[0]);
      _dos_setdrive(dir[0] & 0x1F, &total_drives);
    // if the last char is not \, chdir
    if(dir[strlen(dir)-1] != '\\')
      chdir(dir);
    // else remove \ before chdirring
    else
    {
      dir[strlen(dir)-1] == 0;
      chdir(dir);
      dir[strlen(dir)] == '\\';
    }
  }
  restore_cpu_clock();
  video_mode(0x03);
//  reset_int23();    
  reset_keyboard_handler();
  reset_int24();
  if(checksuff(program, batsuf))
    c = dosexec(program, args);
  else
  {
    sprintf(errormsg, "/C %s %s", program, args);
    found_program = find_executable(comspec, 0);
    if(found_program)
      c = dosexec(found_program, errormsg);
    else
      c = 2;
  }
  screen_redraw = 1;
  //  reset drive
//  chdrive(0);    
  _dos_setdrive(cwdrive, &total_drives);
  // switch back to normal working directory  
  chdir(cwd);
  if(getkey)
    run_pressakey();
  set_int24();
  set_keyboard_handler();
//  set_int23();
//  if(!tty)
    restore_videomode();
//  else
//    new_tty = tty;
  set_vpu_interrupt_handlers();
  set_dosidle_handler();
  end:
 
  if(c)
  {
//    sprintf(errormsg, "Error code: %X",  c );
//    dialog(20, 20, 150, 100, 0, 1, 1, errormsg, "Error!", "This dialog shows error codes.");
    show_error_code(c);
  }      

  return 0;
}

/*
Runs a .BAT file
using the function above
*/
/*static unsigned run_batch(char *dir, char *program, unsigned getkey)
{
  char *args = malloc(127*sizeof(char));
  if(!args)
  {
    nomemory();
    return 0;
  }
  sprintf(args, "/C %s", program);
  run_program(dir, comspec, args, 1, 0xFFFF);
  free(args);
  return 1;
}*/

/*void run_batch(char *dir, char *program, unsigned getkey)
{
  char *cwd;
//  unsigned drive, total;
  unsigned char n = _VIDEO_MODE;
  if(!(cwd = getcwd(NULL, 0)))
  {
    nomemory();
    return;
  }
  video_mode(0x03);
  reset_int24();    
  //_dos_getdrive(&drive);
  if(dir[0] && dir[0] != '\\' && dir[1] == ':')
//    _dos_setdrive(dir[0]+1-'A', &total);
    chdrive(dir[0], 0);
  if(dir[strlen(dir)-1] != '\\')
    chdir(dir);
  else
  {
    dir[strlen(dir)-1] == 0;
    chdir(dir);
    dir[strlen(dir)] == '\\';
  }
  system(program);
//  _dos_setdrive(drive, &total);
  chdrive(0, 1);
  chdir(cwd);
  free(cwd);
  if(getkey)
    run_pressakey();
  set_int24();  
  video_mode(n);
  draw_screen();  
}*/
