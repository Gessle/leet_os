#include "fileassc.c"

static char _FILEMAN_TITLE[] = "File Manager";

struct ftime_t { 

    unsigned short  twosecs : 5;    /* seconds / 2 */ 

    unsigned short  minutes : 6; 

    unsigned short  hours   : 5; 

}; 

struct fdate_t { 

    unsigned short  day     : 5; 

    unsigned short  month   : 4; 

    unsigned short  year    : 7; 

};

struct fattrib
{
  struct ftime_t time;
  struct fdate_t date;
  unsigned char attrib;
};

unsigned char update_list = 0;

// gets directory contents to array of pointers **retval and returns the count of files
//
// the count of results is limited by arguments scroll (the point where the listing begins) and
// count (how many files are listed)
//
// all of **retval's entries must be freed afterwards

struct filename_clipboard
{
  char path[PATHSTRLEN];
  char filename[FILENAMESTRLEN];
  unsigned char pending_operation;
};

struct filename_clipboard filename_clipboard;

static unsigned long getfileinfo(char *path, struct fattrib __near *attrib)
{
  struct find_t fileinfo;
  if(findfirst(path, 0xFF, &fileinfo))
    return -1;
  if(attrib)
  {
    attrib->time = *(struct ftime_t*)&fileinfo.wr_time;
    attrib->date = *(struct fdate_t*)&fileinfo.wr_date;
    attrib->attrib = fileinfo.attrib;
  }
  return fileinfo.size;
}

static struct diskfree_t *check_drive_status(char *drive)
{
  static struct diskfree_t disk_data; 
  unsigned num;
  if(!drive[0] || drive[1] != ':') num = 0;
  else num = drive[0]&0x1F;
  if(!_dos_getdiskfree(num, &disk_data))
    return &disk_data;
  else
    return 0;
}

/*inline unsigned char disk_in_drive(unsigned char drive)
{
  unsigned char reg_ah = 0;
  unsigned int n = 3;
  unsigned char __far *read_sector = _fmalloc(512*sizeof(char));
  unsigned short __near *segaddr = (unsigned short __near*)((unsigned long)read_sector>>16);
  unsigned short __near *rs_offset = (unsigned short __near*)(read_sector);

  if(!read_sector) return 1;

  if(drive > 31)
    drive-=32;


  if(drive<2)
    do
    {
      __asm
      {
//      push ax
//      push dx
//      push cx
//      push es
//      push bx
    
      mov ah, 0x02
      mov al, 0x01
      mov ch, 0x00
      mov cl, 0x01
      mov dh, 0x00
      mov dl, drive
      mov es, segaddr;
      mov bx, rs_offset
    
      int 0x13

      mov reg_ah, ah
      
//      pop bx
//      pop es
//      pop cx
//      pop dx
//      pop ax
      }
    }
    while(n-- && reg_ah);

// the code below completely and permanently wrecks any win9x system for some reason...   
/*    if(drive>1)
    {      
      __asm
      {
//        push ax
//        push bx
//        push cx

        // check if drive is cd-rom drive
        mov ax, 0x1500
        mov bx, 0x0000
        
        int 0x2F

        cmp cx, 0
        je nocdrom

        cmp cx, drive16b

        // if cx > drive, jump to nocdrom
        jg nocdrom

        // to do: check if disk is inserted
        //
        // /////

        mov ax, 0x1505
        mov es, segaddr
        mov bx, rs_offset
        mov cx, drive16b
        mov dx, 0

        int 0x2F

        jnc nocdrom

        mov reg_ah, 0x01

//        mov is_cdrom, 0x01


        //////

        nocdrom:
//        pop cx
//        pop bx
//        pop ax
      }

    }*/
/*  _ffree(read_sector);  
  if(reg_ah) return 0;
  
  return 1;
}*/

static int get_dir_contents(char *directory, char *filenames, char **retval, unsigned int scroll, unsigned int count)
{
  struct find_t   fileinfo; 
  unsigned        rc;
  unsigned int c = 0;
  unsigned char *np, *fip;
  unsigned char filepath[PATHSTRLEN];
  unsigned attr;

  sprintf(filepath, "%s%s", directory, filenames);

  rc = findfirst(filepath, _A_SUBDIR, &fileinfo);
  while(!rc)
  {
    c++;
    if(c > scroll + count) break;
    if(c > scroll && retval)
    {
      if(!(np = retval[c-scroll-1] = malloc((FILENAMESTRLEN+1)* sizeof(char))))
        return -1;
      fip = fileinfo.name;
      sprintf(filepath, "%s%s", directory, fip);
      _dos_getfileattr(filepath, &attr);
      if(attr & _A_SUBDIR)
        sprintf(np, "%s\\", fip);
      else
        strcpy(np, fip);
    }    
    rc = findnext(&fileinfo);     
  }
  
  if((int)c-(int)scroll < 0) return 0;
  return c-scroll;  
}

static void goto_dir(char *path, char *dir)
{
  char *pathp;
  if(dir[0] == '.')
  {
    switch(dir[1])
    {
      case '\\': return;
      case '.':
        if(dir[2] == '\\')
        {
          pathp = path+strlen(path)-2;
          while(*(--pathp) != '\\' && pathp >= path);
          while(*++pathp)
            *pathp = 0;
          return;
        }
    }
  }
  sprintf(path, "%s%s", path, dir);
}

static unsigned openfile_program(struct prog_window *window, unsigned args, int *keypress)
{
  int c;
  if(keypress)
  {
    switch(*keypress)
    {
/*      case 112: case 80:
        args = 0;
        break;*/
      case 13:
        args = WM_WINPROGACT;
    }
    *keypress = -1;
  }
/*  if(!args)
  {
    c = edit_text(window, 0);
    if(c==1) args=1;
  }*/  
  if(args == WM_WINPROGACT || args == (1 | WM_WINPROGACT))
  {
    run_program(0, window->window_grid[0]->text, window->window_grid[3]->text, 0, DEFAULT_PRIVS);
  }
  return ~WM_WINPROGACT;  
}

static struct prog_window *init_openfile_window(char *path)
{
  struct prog_window *retval;

  retval = new_window(-1, -1, 200, 110, "Open file with program", 1, 1, 0, &openfile_program);

  if(!retval)
  {
    nomemory:
    if(retval)
      del_window(retval);
    nomemory();
    return 0;
  }

  if(!(window_textbox(retval, 10, 70, 130, "", PATHSTRLEN)))
    goto nomemory;
  if(!(window_button(retval, 10, 90, 50, _FONT_HEIGHT+2, "Go", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;  
  if(!(window_text(retval, 10, 10, "Open file", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;  
  if(!(window_text(retval, 10, 30, path, _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;  
  if(!(window_text(retval, 10, 50, "using the program:", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;  

  draw_window(retval, 1, 1);
  retval->initialized = 1;

  return retval;
}

static unsigned createdir_program(struct prog_window *window, unsigned args, int *keypress)
{
  int c;
  char fullpath[PATHSTRLEN];

  if(keypress)
  {
    switch(*keypress)
    {
      // N
/*      case 78: case 110:
        args = 0;
        break;*/
      case 13:
        args = WM_WINPROGACT;
    }
    *keypress = -1;
  }
/*  if(!args)
  {
    c = edit_text(window, 0);
    if(c==1) args=1;    
  }*/
  if(args == WM_WINPROGACT || args == (WM_WINPROGACT | 1))
  {
//    draw_mouse_cursor(2);                          
    sprintf(fullpath, "%s%s", window->window_grid[3]->text, window->window_grid[0]->text);
    mkdir(fullpath);
//    del_window(window);

    update_list = 1;

//    draw_mouse_cursor(1);

//    return -101;
    return 2;
  }
  return ~WM_WINPROGACT;  
}

static struct prog_window *init_createdir_window(char *path)
{
  struct prog_window *retval;

  retval = new_window(-1, -1, 200, 110, "Create directory", 1, 1, 0, &createdir_program);

  if(!retval)
  {
    nomemory:
    if(retval)
      del_window(retval);
    nomemory();
    return 0;
  }

  if(!(window_textbox(retval, 10, 70, 130, "", MAX_FILENAME)))
    goto nomemory;  
  if(!(window_button(retval, 10, 90, 50, _FONT_HEIGHT+2, "Go", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;  
  if(!(window_text(retval, 10, 10, "Create new subdirectory to", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;  
  if(!(window_text(retval, 10, 30, path, _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;  
  if(!(window_text(retval, 10, 50, "with the name:", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;  

//  retval->window_grid[0]->connect_element = fileman_window;
  
  draw_window(retval, 1, 1);
  retval->initialized = 1;

  return retval;
}

static unsigned rename_program(struct prog_window *window, unsigned args, int *keypress)
{
//  int c;
  char old_fullpath[PATHSTRLEN];
  char new_fullpath[PATHSTRLEN];

  switch(*keypress)
  {
/*    case 84: case 116:
      args=0;
      break;*/
    case 13:
      args=WM_WINPROGACT;
  }
  *keypress=-1;  
/*  if(!args)
  {
//    c = edit_text(window, 0);
//    if(c == 1)
    if(edit_text(window, 0) == 1)
      args = 1;
  }*/
  if(args == WM_WINPROGACT || args == (WM_WINPROGACT | 1))
  {
//    draw_mouse_cursor(2);                              
    sprintf(old_fullpath, "%s%s", window->window_grid[6]->text, window->window_grid[3]->text);
    sprintf(new_fullpath, "%s%s", window->window_grid[6]->text, window->window_grid[0]->text);
    if(old_fullpath[strlen(old_fullpath)-1] == '\\')
      old_fullpath[strlen(old_fullpath)-1] = 0;
    if(new_fullpath[strlen(new_fullpath)-1] == '\\')
      new_fullpath[strlen(new_fullpath)-1] = 0;      
    if(rename(old_fullpath, new_fullpath))
    {
      sprintf(old_fullpath, "Error %u when trying to rename file", errno);
//      dialog(20, 20, 250, 50, 0, 0, 1, old_fullpath, "Error", 0);
      show_error_msg(old_fullpath);
    }
    update_list = 1;
//    del_window(window);
//    draw_mouse_cursor(1);
//    return -101;
    return 2;
  }
  return ~WM_WINPROGACT;  
}

static struct prog_window *init_rename_window(char *dir, char *filename)
{
  struct prog_window *retval;

  retval = new_window(-1, -1, 200, 130, "Change file name", 1, 1, 0, &rename_program);

  if(!retval)
  {
    nomemory:
    if(retval)
      del_window(retval);
    nomemory();
    return 0;
  }

  if(!(window_textbox(retval, 10, 70, 130, "", MAX_FILENAME)))
    goto nomemory;  
  if(!(window_button(retval, 10, 110, 50, _FONT_HEIGHT+2, "Go", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;  
  if(!(window_text(retval, 10, 10, "Change file name from", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;  
  if(!(window_text(retval, 10, 30, filename, _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;  
  if(!(window_text(retval, 10, 50, "to:", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;  
  if(!(window_text(retval, 10, 90, "in ", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;  
  if(!(window_text(retval, 30, 90, dir, _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;  
  
  draw_window(retval, 1, 1);
  retval->initialized = 1;
  return retval;
}

static unsigned char checksuff(char *filename, char *suf)
{
  unsigned n;
  for(n=4;n;n--)
    if(filename[strlen(filename)-n] != suf[4-n]
      && filename[strlen(filename)-n] != suf[4-n]+0x20)
      break;        
  return n;
}

static int open_file(char *path, char *file, unsigned char viewer)
{
  char full_path[PATHSTRLEN];
  unsigned int n;
  char *assoc_program;

  // test if dir
  if(file[strlen(file)-1] == '\\')
  {
    goto_dir(path, file);
    return 0;
  }
  // test if executable
  if(!checksuff(file, comsuf) || !checksuff(file, exesuf)
  // test if .app bytecode program
    || !checksuff(file, appsuf))
    return 1;
  // text if batch file       
  if(!checksuff(file, batsuf) && !viewer)
    return 2;    

  sprintf(full_path, "%s%s", path, file);
  if(viewer)
  {
    // open file with text viewer
//    init_textview_window(file, full_path);
    // open file with NOTEPAD
    run_program(0, "NOTEPAD.APP", full_path, 0, DEFAULT_PRIVS);
  }
  else
  {
    // open file with the associated program
    if(!(assoc_program = get_file_assoc(str_char_lastocc(full_path, '.')+1)))
      init_openfile_window(full_path);
    else
      run_program(0, assoc_program, full_path, 0, DEFAULT_PRIVS);
    if(!checksuff(file, lnksuf))
    {
      reload_shortcuts();
      draw_screen();
    }
  }
  return 3;
}

static void copy_file(char *dir, char *file, unsigned char operation)
{
  sprintf(filename_clipboard.path, "%s%s", dir, file);
  sprintf(filename_clipboard.filename, "%s", file);
  filename_clipboard.pending_operation = operation;
}

static int paste_dir(char *dpath)
{
  char *args = malloc((MAX_PATH*2+5)* sizeof(char));
  char *destdir = malloc(MAX_PATH-MAX_FILENAME+1* sizeof(char));
  if(!args || !destdir)
  {
    free(args);
    free(destdir);
    return 0;
  }
  // make destination dir (remove trailing \ for mkdir);
  filename_clipboard.filename[strlen(filename_clipboard.filename)-1] = 0;
  sprintf(args, "%s%s", dpath, filename_clipboard.filename);
  mkdir(args);
  sprintf(destdir, "%s", args);
  filename_clipboard.path[strlen(filename_clipboard.path)-1] = 0;            
  sprintf(args, "%s %s /E", filename_clipboard.path, destdir);
//  dialog(20, 20, 600, 50, 0, 0, 0, args, "Info", 0);abort();
  run_program(0, "XCOPY", args, 0, 0xFFFF);
  free(destdir);
  free(args);
  if(filename_clipboard.pending_operation == 2)
  {
    // remove trailing \ for deltree
    run_program(0, "DELTREE", filename_clipboard.path, 0, 0xFFFF);
  }
  filename_clipboard.pending_operation = 0;
  return 1;
}

#define FILE_COPY_BUFF 512
struct file_copy
{
  int srcfp;
  int dstfp;
  unsigned long file_size;
  unsigned long write_count;
  unsigned char buff[FILE_COPY_BUFF];
  unsigned progress;
  char srcfile[81];
  unsigned delete_source:1;  
};

static void end_file_copy(struct prog_window *window)
{
  struct file_copy *cp_strct = window->function_status;

  close(cp_strct->dstfp);
  close(cp_strct->srcfp);
  if(cp_strct->delete_source)
    remove(cp_strct->srcfile);
}

static unsigned copy_file_program(struct prog_window *window, unsigned args, int *keypress)
{
  int read_count;
  struct file_copy *cp_strct = window->function_status;
  unsigned progress;
  char msg[20];
  unsigned char start_time = dostime()->ss;

  if(diskio_in_progress)
    return ~WM_WINPROGACT;

  if(!cp_strct->write_count)
  {
    draw_window(window, 1, 0);
  }

  do
  {
    if((read_count = read(cp_strct->srcfp, cp_strct->buff, FILE_COPY_BUFF)) > 0)
    {
      if(write(cp_strct->dstfp, cp_strct->buff, read_count) < read_count)
      {
        show_error_msg("Error while copying file");
        break;
      }
      cp_strct->write_count += read_count;
      progress = cp_strct->write_count * 100 / cp_strct->file_size;
      if(cp_strct->progress != progress)
      {
        cp_strct->progress = progress;
        sprintf(msg, "%u %%", progress);
        draw_window(window, 1, 3);
        gprint_text(window->x+10, window->y+30, msg, 0, _FONT_HEIGHT, -1, 0);
        call_drv_screenupdate();
      }
    }
    else
    {
      del_window(window);
      break;
    }
  }
  while(start_time == dostime()->ss);
  return ~WM_WINPROGACT;
}

static void copy_file_init(char *src, char *dst, char *filename)
{
  struct prog_window *window;
  struct file_copy *file_copy_strct = calloc(1, sizeof(struct file_copy));
  char msg[50];

  if(!file_copy_strct)
  {
    nomemory();
    return;
  }
  file_copy_strct->file_size = getfileinfo(src, 0);
  if(filename_clipboard.pending_operation == 2)
    file_copy_strct->delete_source = 1;
  strcpy(file_copy_strct->srcfile, src);

  window = new_window(-1, -1, 150, 50, "Copying file", 1, 1, 0, &copy_file_program);
  if(!window)
  {
    nomemory();
    free(file_copy_strct);
    return;
  }
  sprintf(msg, "%s %s", window->title, filename);
  window->function_status = file_copy_strct;
  window->on_close = (void*)end_file_copy;

  if(!window_text(window, 10, 10, msg, _WINDOW_BASICTEXTCOLOR)
/*  || !window_text(window, 10, 30, " 00 %", _WINDOW_BASICTEXTCOLOR)*/)
  {
    nomemory();    
    end:
    del_window(window);
    return;
  }

  file_copy_strct->srcfp = open(src, O_RDONLY | O_BINARY);
  file_copy_strct->dstfp = open(dst, O_WRONLY | O_TRUNC | O_CREAT | O_BINARY, S_IRWXU);
  if(file_copy_strct->dstfp == -1 || file_copy_strct->srcfp == -1)
  {
    if(file_copy_strct->dstfp >= 0) close(file_copy_strct->dstfp);
    if(file_copy_strct->srcfp >= 0) close(file_copy_strct->srcfp);
    window->on_close = 0;
    goto end;
  }
  window->initialized = 1;
}

/*void copy_file_proc(char *src, char *dst, char *filename)
{
  struct prog_window *window;
  unsigned long drive_space;
  unsigned long file_size = getfileinfo(src);
  char title[] = "Copying file";
  char msg[50];
  int srcfp, dstfp;
  int read_count;
  unsigned long write_count = 0;
  unsigned char buffer[512];
  static unsigned char percent = -1;

  window = new_window(-1, -1, 150, 40, title, 1, 1, 1, 0, 0);
  if(!window)
  {
    nomemory();
    goto end;
  }
    
  sprintf(msg, "%s %s", title, filename);
  
  if(!(window_text(window, 10, 10, msg, _WINDOW_BASICTEXTCOLOR)))
  {
    nomemory();
    goto end;
  }

  srcfp = open(src, O_RDONLY | O_BINARY);
  dstfp = open(dst, O_WRONLY | O_TRUNC | O_CREAT | O_BINARY);
  if(srcfp == -1 || dstfp == -1)
  {
    if(srcfp >= 0) close(srcfp);
    if(dstfp >= 0) close(dstfp);
    goto end;
  }

  while((read_count = read(srcfp, buffer, sizeof(buffer))) > 0)
  {
    if(write(dstfp, buffer, read_count) < read_count)
      break;
    write_count += read_count;
    if(percent != (write_count * 100 / file_size))
    {
      percent = (write_count * 100) / file_size;
      draw_window(window, 1, 3);             
      sprintf(msg, "%u %%", percent);
      gprint_text(window->x+10, window->y+30, msg, 0, _FONT_HEIGHT, -1, 0);       
      call_drv_screenupdate();    
    }
  }

  close(srcfp);
  close(dstfp);
    
  end:del_window(window);
}*/

static int paste_file(char *dpath)
{
  FILE *fp;
  FILE *source;
  int byte;
  char path[PATHSTRLEN];
  char message[51];
  if(!filename_clipboard.pending_operation)
  {
    sprintf(message, "Nothing to paste!");
    dialog(20, 20, 200, 50, 0, 0, 0, message, infotitle, 0);
    return 0;
  }
  // if directory
  if(filename_clipboard.filename[strlen(filename_clipboard.filename)-1] == '\\')
  {
    sprintf(message, "Copy directory %s using XCOPY?", filename_clipboard.filename);    
    if(!dialog(20, 20, 250, 50, 2, 0, 0, message, "Copy directory", 0))    
      if(!paste_dir(dpath))
        nomemory();
    return 0;
  }
  sprintf(path, "%s%s", dpath, filename_clipboard.filename);  
  if((fp = fopen(path, "rb")))
  {
    fclose(fp);
    sprintf(message, "Replace %s in the destination directory?", filename_clipboard.filename);
    if(dialog(20, 20, 300, 50, 2, 0, 0, message, "File exists!", 0))
      return 0;
  }
//  copy_file_proc(filename_clipboard.path, path, filename_clipboard.filename);
  copy_file_init(filename_clipboard.path, path, filename_clipboard.filename);


/*  if(filename_clipboard.pending_operation == 2)
    return remove(filename_clipboard.path);*/
  return 0;
}

static int delete_dir(char *dir, char *file)
{
  char path[84];
  char message[46];
  sprintf(message, "Delete directory %s using DELTREE?", file);
  if(!dialog(20, 20, 250, 50, 2, 0, 0, message, "Delete directory", 0))
  {
    sprintf(path, "%s%s /Y", dir, file);
    run_program(0, "DELTREE", path, 0, 0xFFFF);    
  }
  return 0;
  
}

static int delete_file(char *dir, char *file)
{
  char path[PATHSTRLEN];
  char message[27];
  // if dir
  if(file[strlen(file)-1] == '\\')
  {
    file[strlen(file)-1] = 0;
    delete_dir(dir, file);
    return 0;
  }
  sprintf(message, "Delete file %s?", file);
  if(!dialog(20, 20, 200, 50, 2, 0, 0, message, "Delete file", 0))
  {
    sprintf(path, "%s%s", dir, file);
    remove(path);
    if(!checksuff(file, lnksuf))
    {
      reload_shortcuts();
      draw_screen();
    }
    
  }
  return 0;
}

static int list_files(struct prog_window *window, unsigned int element, unsigned int scroller, char *dir, char *filenames, unsigned int scroll, unsigned int count)
{
  int n;
  int fc;
  for(n=count;n--;)
  {
    free(window->window_grid[element]->items[n]);
    window->window_grid[element]->items[n] = 0;
  }
  // how many files in directory
  if((n = (fc = get_dir_contents(dir, filenames, 0, 0, 0-1)) - count) > 0)
    window->window_grid[scroller]->c = n;
  else
    window->window_grid[scroller]->c = 0;
  window_drawelement(window, scroller);
  if(fc < 0) return 0;
  else get_dir_contents(dir, filenames, window->window_grid[element]->items, scroll, count);
//  window_drawelement(window, element);
  return 1;
}

static char *hbytes(unsigned long bytes)
{
  char *units[] =
  {
    "bytes",
    "kilo",
    "mega",
//    "giga"
  };
  unsigned int unit_change = 10;
  static char size[20];

  if(bytes > (unsigned long)unit_change<<20)
    sprintf(size, "%lu %s%s", bytes>>20, units[2], units[0]);  
  else if(bytes > (unsigned long)unit_change<<10)
    sprintf(size, "%lu %s%s", bytes>>10, units[1], units[0]);  
  else
    sprintf(size, "%lu %s", bytes, units[0]);           

  return size;
}

static void show_drive_info(char *dir)
{
  char msg[PATHSTRLEN];
  struct diskfree_t *driveinfo;
  struct prog_window *window;
  unsigned long drive_space;
  unsigned char drive;

//  if(dir[0] >= 97)
//    dir[0]-=32;
  if(dir[1] == ':')
    drive = dir[0];
  else
    drive = cwdrive + 'A'-1;
  
  sprintf(msg, "Drive %c - Info", drive);
  window = new_window(-1, -1, 180, 150, msg, 1, 1, 0, 0);
  if(!window)
  {
    nomemory:
    if(window)
      del_window(window);
    nomemory();
    return;
  }
  driveinfo = check_drive_status(dir);
  if(driveinfo)
  {
    sprintf(msg, "Total clusters: %u", driveinfo->total_clusters);
    if(!(window_text(window, 10, 10, msg, _WINDOW_BASICTEXTCOLOR)))
      goto nomemory;

    sprintf(msg, "Available clusters: %u", driveinfo->avail_clusters);
    if(!(window_text(window, 10, 30, msg, _WINDOW_BASICTEXTCOLOR)))
      goto nomemory;

    sprintf(msg, "Sectors/cluster: %u", driveinfo->sectors_per_cluster);
    if(!(window_text(window, 10, 50, msg, _WINDOW_BASICTEXTCOLOR)))
      goto nomemory;

    sprintf(msg, "Bytes per sector: %u", driveinfo->bytes_per_sector);
    if(!(window_text(window, 10, 70, msg, _WINDOW_BASICTEXTCOLOR)))
      goto nomemory;

    // show total space
    drive_space = (unsigned long)driveinfo->bytes_per_sector * driveinfo->sectors_per_cluster * driveinfo->total_clusters;
    sprintf(msg, "Drive size: %s", hbytes(drive_space));
      if(!(window_text(window, 10, 90, msg, _WINDOW_BASICTEXTCOLOR)))
        goto nomemory;

    // show free space
    drive_space = (unsigned long)driveinfo->bytes_per_sector * driveinfo->sectors_per_cluster * driveinfo->avail_clusters;
    sprintf(msg, "Free space: %s", hbytes(drive_space));
      if(!(window_text(window, 10, 110, msg, _WINDOW_BASICTEXTCOLOR)))
        goto nomemory;    
  }

  draw_window(window, 1, 1);      
}

static void show_file_info(char *dir, char *file, char *path)
{
  char msg[PATHSTRLEN];
//  unsigned attr;
  unsigned n=0;
  struct fattrib attrib;
  struct prog_window *retval = new_window(-1, -1, 200, 150, file, 1, 1, 0, 0);
  if(!retval)
  {
    nomemory:
    if(retval)
      del_window(retval);
    nomemory();
    return;
  }
  sprintf(msg, "File name: %s", file);
  if(!(window_text(retval, 10, 10, msg, _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;
  sprintf(msg, "Directory: %s", dir);
  if(!(window_text(retval, 10, 30, msg, _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;  
  sprintf(msg, "Size: %s", hbytes(getfileinfo(path, (void __near*)&attrib)));
  if(!(window_text(retval, 10, 50, msg, _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;
  sprintf(msg, "Timestamp: %u. %u. %u %.2u:%.2u:%.2u",
    attrib.date.day, attrib.date.month, attrib.date.year+1980,
    attrib.time.hours, attrib.time.minutes, attrib.time.twosecs<<1);  
  if(!(window_text(retval, 10, 70, msg, _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;
  if(!(window_text(retval, 10, 90, "Attributes:", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;  
  if(attrib.attrib & _A_RDONLY)
    if(!(window_text(retval, 70, 90+n++*20, "Read only", _WINDOW_BASICTEXTCOLOR)))
      goto nomemory;    
  if(attrib.attrib & _A_HIDDEN)
    if(!(window_text(retval, 70, 90+n++*20, "Hidden", _WINDOW_BASICTEXTCOLOR)))
      goto nomemory;        
  if(attrib.attrib & _A_SYSTEM)
    if(!(window_text(retval, 70, 90+n++*20, "System", _WINDOW_BASICTEXTCOLOR)))
      goto nomemory;        
  if(attrib.attrib & _A_VOLID)
    if(!(window_text(retval, 70, 90+n++*20, "Volume ID", _WINDOW_BASICTEXTCOLOR)))
      goto nomemory;        
  if(attrib.attrib & _A_SUBDIR)
    if(!(window_text(retval, 70, 90+n++*20, "Directory", _WINDOW_BASICTEXTCOLOR)))
      goto nomemory;        
  if(attrib.attrib & _A_ARCH)
    if(!(window_text(retval, 70, 90+n++*20, "Archive", _WINDOW_BASICTEXTCOLOR)))
      goto nomemory;        

  draw_window(retval, 1, 1);
}

static unsigned fileman_place_elements(struct prog_window *window)
{
  unsigned int n;
  void *new_pointer;

  window->window_grid[0]->width = window->width-80;
  window->window_grid[1]->width = window->width-80;

  window->window_grid[4]->width = window->width-100;
  window->window_grid[4]->height = window->height-90;
  window->window_grid[4]->selectc = -1;              

  for(n=window->window_grid[4]->c;n--;)
    free(window->window_grid[4]->items[n]);

  window->window_grid[4]->c = window->window_grid[4]->height/(_FONT_HEIGHT+2);
  
  if(new_pointer = calloc(window->window_grid[4]->c, sizeof(char*)))
  {
    free(window->window_grid[4]->items);
    window->window_grid[4]->items = (char **)new_pointer;
  }
  else
  {
//    del_window(window);
    nomemory();
    return ~WM_WINPROGACT;
  }
    
  for(n=0;n<window->window_grid[4]->c;n++)
    if(!(window->window_grid[4]->items[n] = calloc(FILENAMESTRLEN+1, sizeof(char))))
    {
//      del_window(window);
      nomemory();
      return ~WM_WINPROGACT;
    }

  window->window_grid[5]->x = window->width-89;
  window->window_grid[5]->width = window->height-116;

  window->window_grid[6]->x = window->width-70;
  window->window_grid[7]->x = window->width-70;
  window->window_grid[8]->x = window->width-70;
  window->window_grid[9]->x = window->width-70;
  window->window_grid[10]->x = window->width-70;
  window->window_grid[11]->x = window->width-70;
  window->window_grid[14]->x = window->width-70;

  window->window_grid[12]->y = window->height-42;
  window->window_grid[13]->y = window->height-22;
  window->window_grid[15]->y = window->height-42;    

  window_recalc_hitboxes(window);
//  draw_window(window, 0, 1);
  if(!screen_redraw)
    draw_window(window, 1, 0);
  return WM_WINPROGACT | 3;
}

#define MAX_DRIVELETTERS ('Z' - 'A' + 1)

inline unsigned fileman_keypress(int *keypress, struct prog_window *window, unsigned args)
{
  unsigned count = 1;
  struct window_element *scrollbar = window->window_grid[5];
  struct window_element *filelist = window->window_grid[4];
  unsigned row_count = filelist->height / (_FONT_HEIGHT+2);

  if(!*keypress)
  {
    switch(getch())
    {
      // F5
      case 63:
        *(char*)window->function_status = 1;
        update_list=1;
        break;
      // alt+t
      case 20:
        return WM_WINPROGACT | 14+MAX_DRIVELETTERS;
      // delete
      case 83:
        return WM_WINPROGACT | 9;
      // page down
      case 81:
        count=row_count;
      // down arrow
      case 80:
        if(filelist->selectc + count < filelist->c)
        {
          window_list_selectrow(window, 4, filelist->selectc + count);
        }
        else
        {
          count -= filelist->c - filelist->selectc - 1;
          filelist->selectc = filelist->c-1;
          if(scrollbar->scroll_x + count < scrollbar->c)
            scrollbar->scroll_x += count;
          else
          {
            scrollbar->scroll_x = scrollbar->c;
          }
          return WM_WINPROGACT | 3;    
        }
        return ~WM_WINPROGACT;
      case 73: // page up
        count = row_count;
      // up arrow
      case 72:
        if(filelist->selectc >= count)
          window_list_selectrow(window, 4, filelist->selectc - count);
        else
        {
          count -= filelist->selectc;
          filelist->selectc = 0;
          if(scrollbar->scroll_x > count) scrollbar->scroll_x -= count;
          else scrollbar->scroll_x = 0;
          return WM_WINPROGACT | 3;
        }
        return ~WM_WINPROGACT;
    }
  }
  switch(*keypress)
  {
    // ctrl+c
    case 3:
      return WM_WINPROGACT | 6;
    // ctrl+x
    case 24:
      return WM_WINPROGACT | 7;
    // ctrl+v
    case 22:
      return WM_WINPROGACT | 8;
    // enter
    case 13:
      return WM_WINPROGACT | 4;
    // V
    case 118: case 86:
      return WM_WINPROGACT | 5;
    // I
    case 105: case 73:
      return WM_WINPROGACT | 10;
    // C
    case 99: case 67:
      return WM_WINPROGACT | 11;
    // N
    case 110: case 78:
      return WM_WINPROGACT | 12;
    // O
    case 79: case 111:
      return WM_WINPROGACT | 13;
/*    // D
    case 68: case 100:
      return 0;
    // F
    case 70: case 102:
      return 1;*/
  }
  return args;
}

inline void update_title(struct prog_window *window, char *dir)
{
  void *new_pointer;
  char *charp;
  while((charp = strchr(dir, '\\')) && *(++charp))
    dir=charp;
  if(*dir && !strncmp(window->title, dir, strlen(dir)))
    return;
  if(new_pointer = calloc(strlen(_FILEMAN_TITLE)+strlen(dir)+4, sizeof(char)))
  {
    free(window->title);
    window->title = new_pointer;
  }
  else return;
    
  sprintf(window->title, "%s - %s", dir, _FILEMAN_TITLE);
  if(windows[active_window] == window)
    draw_window(window, 1, 2);          
}

static unsigned fileman_program(struct prog_window *window, unsigned args, int *keypress)
{
  int c;
  char *dir = window->window_grid[0]->text;
  char *filenames = window->window_grid[1]->text;
  char fullpath[PATHSTRLEN];
//  void *new_pointer;

  if(diskio_in_progress)
    return ~WM_WINPROGACT;

  if(keypress)
  {
    args = fileman_keypress(keypress, window, args);
    *keypress = -1;    
  }

/*  if(window->resized)
  {
    if(fileman_place_elements(window))
    {
        draw_window(window, 1, 0);
      window->resized=0;      
      goto list;
    }
  }*/
  
  if(args == (WM_WINPROGACT | 3))
  {
    list:
//    draw_mouse_cursor(2);    
    update_title(window, dir);    
    // check if disk is in drive
/*    if(dir[1] == ':' && !disk_in_drive(dir[0]-'A'))
    {
      show_error_msg("Cannot read from floppy!");
    }
    else*/
    {
      list_files(window, 4, 5, dir, filenames, window->window_grid[5]->scroll_x, window->window_grid[4]->c);      
      window_drawelement(window, 4);
    }
//    draw_mouse_cursor(1);
  }
  if(args == WM_WINPROGACT || args==(WM_WINPROGACT|1))
  {
/*    c = edit_text(window, args);
    if(c)
    {*/
      if(args == WM_WINPROGACT)
      {
        list2:// add backslash if necessary
        if(*window->window_grid[0]->text && window->window_grid[0]->text[strlen(window->window_grid[0]->text)-1] != '\\')
          if(strlen(window->window_grid[0]->text) < MAX_PATH-MAX_FILENAME)
            strcat(window->window_grid[0]->text, "\\");
      }
      window->window_grid[5]->scroll_x = 0;
      window_drawelement(window, 5);
      window->window_grid[4]->selectc = -1;
      args=~WM_WINPROGACT; goto list;
//    }
  }
  if(args == (WM_WINPROGACT|2))
  {
    if(window->window_grid[4]->connect_element == window->window_grid[4]->selectc)
    {
      args = WM_WINPROGACT|4;
      window->window_grid[4]->connect_element = -1;
    }
  }
  if(args == (WM_WINPROGACT|4) || args == (WM_WINPROGACT|5) || args == (WM_WINPROGACT|10))
    sprintf(fullpath, "%s%s", dir, window->window_grid[4]->items[window->window_grid[4]->selectc]);  
  if(args == (WM_WINPROGACT|4) || args == (WM_WINPROGACT|5))
  {
//    draw_mouse_cursor(2);                                  
    if(window->window_grid[4]->selectc != -1 && window->window_grid[4]->items[window->window_grid[4]->selectc])
    {    
      // if dir
      if(!(c = open_file(dir, window->window_grid[4]->items[window->window_grid[4]->selectc], args-4)))
      {
        window_drawelement(window, 0);
        window->window_grid[5]->scroll_x = 0;
        window->window_grid[4]->selectc = -1;
/*        if(new_pointer = realloc(window->title, 16+strlen(dir)*sizeof(char)))
          window->title = (char*)new_pointer;
      
        sprintf(window->title, "%s - %s", dir, _FILEMAN_TITLE);*/
        args=~WM_WINPROGACT; goto list;
      }
      else if(c == 1 || c == 2)
      {
        // if executable
        if(c == 1)
        {
          run_program(dir, fullpath, 0, 0, DEFAULT_PRIVS);
        }
        // if batch
        else if(c == 2)
        {
          run_program(dir, fullpath, 0, 1, DEFAULT_PRIVS);
        }  
      }
    }
//    draw_mouse_cursor(1);                                  
  }
  if(args == (WM_WINPROGACT|6) || args == (WM_WINPROGACT|7))
  {
    if(window->window_grid[4]->selectc != -1)
      copy_file(dir, window->window_grid[4]->items[window->window_grid[4]->selectc], args-5);    
  }
  if(args == (WM_WINPROGACT|8))
  {
    paste_file(dir);
    args=~WM_WINPROGACT; goto list;        
  }
  if(args == (WM_WINPROGACT|9))
  {
//    draw_mouse_cursor(2);    
    if(window->window_grid[4]->selectc != -1)
      delete_file(dir, window->window_grid[4]->items[window->window_grid[4]->selectc]);
//    draw_mouse_cursor(1);
    args=~WM_WINPROGACT; goto list;        
  }
  if(args == (WM_WINPROGACT|10))
  {
//    draw_mouse_cursor(2);
    if(window->window_grid[4]->selectc != -1)
      show_file_info(dir, window->window_grid[4]->items[window->window_grid[4]->selectc], fullpath);
//    draw_mouse_cursor(1);            
  }

  if(args == (WM_WINPROGACT|11))
  {
    init_createdir_window(dir);
    *(char*)window->function_status = 1;
  }
  if(update_list && *(char*)window->function_status == 1)
  {
    update_list = 0;
    *(char*)window->function_status = 0;
    goto list;
  }

  if(args == (WM_WINPROGACT|12))
  {
    if(window->window_grid[4]->selectc != -1)
    {
      init_rename_window(dir, window->window_grid[4]->items[window->window_grid[4]->selectc]);
      *(char*)window->function_status = 1;
    }
  }
  if(args == (WM_WINPROGACT|13))
  {
//    draw_mouse_cursor(2);    
    show_drive_info(dir);
//    draw_mouse_cursor(1);    
  }
  if(args == (WM_WINPROGACT|14+MAX_DRIVELETTERS))
    run_program(dir, comspec, 0, 0, 0xFFFF);

  else if((args&~WM_WINPROGACT) >= window->hitbox_count && (args&~WM_WINPROGACT) < window->hitbox_count+MAX_DRIVELETTERS)
  {
    sprintf(dir, "%c:", 'A' + (args &~WM_WINPROGACT) - window->hitbox_count);
    window_drawelement(window, 0);
    goto list2;
  }

//  return args+100;  
  return ~WM_WINPROGACT;
}

inline int build_drive_menu(struct prog_window *window)
{
  unsigned drives;
  char *drivestr = "A:";
  unsigned n;
  void *pmenu = window->menus.menubar;

  unsigned char bios_equipment_flags = *(unsigned char *)MK_FP(0x40, 0x10);
  unsigned char bios_floppy_count = bios_equipment_flags & 0x01;
  if(bios_floppy_count) bios_floppy_count += bios_equipment_flags >> 6;

  _dos_setdrive(0, &drives);

  for(n=0;n<drives;n++)
  {
    if(n < bios_floppy_count || n >= 2)
    {
      *drivestr = 'A' + n;
      if(!new_menuitem(pmenu, drivestr, WM_WINPROGACT | (window->hitbox_count + n)))
        return 0;
    }
  }
  return 1;
}

static struct prog_window *init_fileman_window(char *path)
{
  struct window_element *element;
  struct prog_window *retval = new_window(-1, -1, 250, 190, _FILEMAN_TITLE, 1, 1, "ICONS/FILEMAN.ICO", &fileman_program);
  char dir[] = "\\";
  char filenames[] = "*.*";
  char *dirstr;

  if(!retval)
  {
    nomemory:
    if(retval) del_window(retval);
    return 0;    
  }

  if(!(retval->function_status = malloc(sizeof(char))))
    goto nomemory;
//  else
  {
    if(path) dirstr = path;
    else dirstr = dir;

    if(!(window_textbox(retval, 70, 3, 170, dirstr, MAX_PATH-MAX_FILENAME)))
      goto nomemory;
    if(!(window_textbox(retval, 70, 23, 170, filenames, MAX_FILENAME)))
      goto nomemory;
    if(!(window_text(retval, 10, 3, "Directory:", _WINDOW_BASICTEXTCOLOR)))
      goto nomemory;
    if(!(window_text(retval, 10, 23, "File names:", _WINDOW_BASICTEXTCOLOR)))
      goto nomemory;
    if(!(element = window_list(retval, 10, 43, 150, 100, 0, 10)))
      goto nomemory;
    if(!(element->items = calloc(10, sizeof(char*))))
      goto nomemory;
    if(!(window_scrollbar(retval, 161, 43, 0, 100-26, 0, -1, 0)))
      goto nomemory;

    if(!(window_button(retval, 180, 43, 60, _FONT_HEIGHT+2, "Open", _WINDOW_BASICTEXTCOLOR)))
      goto nomemory;
    if(!(window_button(retval, 180, 63, 60, _FONT_HEIGHT+2, "\021View", _WINDOW_BASICTEXTCOLOR)))
      goto nomemory;
    if(!(window_button(retval, 180, 83, 60, _FONT_HEIGHT+2, "Copy", _WINDOW_BASICTEXTCOLOR)))
      goto nomemory;    
    if(!(window_button(retval, 180, 103, 60, _FONT_HEIGHT+2, "Cut", _WINDOW_BASICTEXTCOLOR)))
      goto nomemory;    
    if(!(window_button(retval, 180, 123, 60, _FONT_HEIGHT+2, "Paste", _WINDOW_BASICTEXTCOLOR)))
      goto nomemory;    
    if(!(window_button(retval, 180, 143, 60, _FONT_HEIGHT+2, "Delete", _WINDOW_BASICTEXTCOLOR)))
      goto nomemory;    
    if(!(window_button(retval, 10, 148, 60, _FONT_HEIGHT+2, "File \021info", _WINDOW_BASICTEXTCOLOR)))
      goto nomemory;    
    if(!(window_button(retval, 10, 168, 133, _FONT_HEIGHT+2, "\021Create directory", _WINDOW_BASICTEXTCOLOR)))
      goto nomemory;    
    if(!(window_button(retval, 180, 163, 60, _FONT_HEIGHT+2, "Re\021name", _WINDOW_BASICTEXTCOLOR)))
      goto nomemory;
    if(!(window_button(retval, 75, 148, 68, _FONT_HEIGHT+2, "Drive inf\021o", _WINDOW_BASICTEXTCOLOR)))
      goto nomemory;          

    retval->menus.menubar = new_menu("Drive", 0);
    if(!retval->menus.menubar)
      goto nomemory;
    if(!build_drive_menu(retval))
      goto nomemory;
    
      
    if(!(list_files(retval, 4, 5, dirstr, filenames, retval->window_grid[5]->scroll_x, 10)))
      goto nomemory;

    draw_window(retval, 1, 1);      
    draw_menubar();
    retval->on_resize = fileman_place_elements;
  }
  retval->initialized = 1;
  return retval;
}
