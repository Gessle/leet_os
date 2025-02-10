#define OPEN_O_READ 0x0000
#define OPEN_O_WRITE 0x0001
#define OPEN_O_APPEND 0x0002
#define OPEN_O_RW 0x0004
#define OPEN_O_SYNC 0x0008
#define OPEN_O_NONBLOCK 0x1000
#define PIPE_BUF 0x200

#define GLOBAL_MAX_FILES 30

//#include "biosser.c"
//#include "biospar.c"

unsigned global_open_files = 0;
int load_driver(char *program, char *modname);
unsigned char device_exists(unsigned driver, unsigned device);
int driver_id(char *name);
extern struct driver *drivers;

char parallel_init(unsigned short port, unsigned int mode);
unsigned char parallel_write(unsigned short port, unsigned char chr);

void con_move_back(void);

int chdrive(unsigned drive);

int serial_init(unsigned char port, unsigned short mode);
int serial_read(unsigned char port);
unsigned char serial_write(unsigned char port, unsigned char chr);

char modfs[] = "DEV:\\";

void append_path(struct vpu *vpu, char *dst, char *relpath)
{
  if(*relpath == '\\' || relpath[1] == ':')
    strncpy(dst, relpath, MAX_PATH);
  else
  {
    strcpy(dst, vpu->cwd);
    if(dst[strlen(dst)-1] != '\\')
      strcat(dst, "\\");
    strncat(dst, relpath, MAX_PATH);
  }
}

void vpu_sched_yield(struct vpu *vpu)
{
  vpu->yield = 1;
  vpu->wait = 1;
}

static void sync_dup(struct file_struct *fd0, struct file_struct *fd1)
{
//  sprintf(message, "Synkataan dupit %p ja %p", fd0, fd1);
//  putstr(message);
  
  if(fd0->file_pos && fd1->file_pos)
    *fd0->file_pos = *fd1->file_pos;
  fd0->pipe_from = fd1->pipe_from;
  fd0->pipe_to = fd1->pipe_to;
  fd0->flags = fd1->flags;
  fd0->bufrpos = fd1->bufrpos;
  fd0->bufwpos = fd1->bufwpos;
}

static void fd_dup_sync(struct file_struct *fd)
{
  struct file_struct *dupfd = fd;

  while(dupfd = dupfd->dup)
  {
    sync_dup(dupfd, fd);
  }

  dupfd = fd;
  while(dupfd = dupfd->orig)
  {
    sync_dup(dupfd, fd);
  }
}

static void reset_fd(struct vpu *vpu, unsigned fd)
{
  memset(vpu->file_struct[fd], 0, sizeof(struct file_struct));
  vpu->file_struct[fd]->open = 1;
}

static void dd_dup_sync(struct dir_struct *dd)
{
 // todo...
}

static void vpu_dd_makedup(struct vpu *vpu0, unsigned dd0, struct vpu *vpu1, unsigned dd1)
{
  struct dir_struct *orig = vpu1->dir_handles[dd1];
  
  *vpu0->dir_handles[dd0] = *vpu1->dir_handles[dd1];

  while(orig->dup) orig = orig->dup;

  vpu0->dir_handles[dd0]->orig = orig;
  orig->dup = vpu0->dir_handles[dd0];
}

static int pipe_read(struct file_struct *pipe, char *buff, unsigned count)
{
  unsigned *read_pos = &pipe->bufrpos;
  unsigned *write_pos = &pipe->bufwpos;
  unsigned data_count = *write_pos - *read_pos;

  if(count > data_count) count = data_count;
  ringcopy(buff, pipe->buff, pipe->bufrpos, PIPE_BUF, count);
  *read_pos += count;
  return count;
}

char stdin_endchars[] =
{
  0x0A, // newline
  0x04, // EOT    
  0
};

static int stdin_has_endchar(struct console_struct* console)
{
  char *ptr = stdin_endchars;

  do
    if(memchr(console->con_stdin.buff, *ptr, console->input_col))
      return 1;
  while(*++ptr);

  return 0;
}

static int con_stdin_read(void)
{  
  if(!((struct console_struct*)running_console->function_status)->nolinebuffer
/*     && !memchr(((struct console_struct*)running_console->function_status)->con_stdin.buff, 10,
     ((struct console_struct*)running_console->function_status)->input_col))*/
     && !stdin_has_endchar(running_console->function_status))
     return -1;

  if(((struct console_struct*)running_console->function_status)->con_stdin.buff
    [((struct console_struct*)running_console->function_status)->con_stdin.pos])
    return ((struct console_struct*)running_console->function_status)->con_stdin.buff
      [((struct console_struct*)running_console->function_status)->con_stdin.pos++];

  *((struct console_struct*)running_console->function_status)->con_stdin.buff = 0;
  ((struct console_struct*)running_console->function_status)->con_stdin.pos = 0;
  ((struct console_struct*)running_console->function_status)->input_col = 0;

  if(((struct console_struct*)running_console->function_status)->con_stdin.len != 32)
  {
    ((struct console_struct*)running_console->function_status)->con_stdin.buff
      = realloc(((struct console_struct*)running_console->function_status)->con_stdin.buff, 32*sizeof(char));
    ((struct console_struct*)running_console->function_status)->con_stdin.len = 32;      
  }
  
  return -1;
}

static void con_fsync(void)
{
  unsigned n;
  flush_stdout();
  ((struct console_struct*)running_console->function_status)->con_stdout.pos = 0;
  if(((struct console_struct*)running_console->function_status)->con_stdout.len != 32)
  {
    ((struct console_struct*)running_console->function_status)->con_stdout.buff
      = realloc(((struct console_struct*)running_console->function_status)->con_stdout.buff, 32*sizeof(char));
    ((struct console_struct*)running_console->function_status)->con_stdout.len = 32;
  }  

}

static int con_stdout_write(unsigned char c)
{
  void *new_pointer;

  if(c == 8)
  {
    con_move_back();
    return 3;
  }

  if(c == 10)
  {
    con_fsync();
    inc_row();
    return 3;
  }
  
  ((struct console_struct*)running_console->function_status)->con_stdout.buff
    [((struct console_struct*)running_console->function_status)->con_stdout.pos++] = c;

  if(((struct console_struct*)running_console->function_status)->con_stdout.pos
    == ((struct console_struct*)running_console->function_status)->con_stdout.len-1)
  {
    new_pointer = realloc(((struct console_struct*)running_console->function_status)->con_stdout.buff,
      (((struct console_struct*)running_console->function_status)->con_stdout.len += 32) * sizeof(char));

    if(!new_pointer)
    {
      con_fsync();
      return 3;
    }
    ((struct console_struct*)running_console->function_status)->con_stdout.buff = new_pointer;
  }


  return 0;
}

static int vpu_syscall_read(struct vpu *vpu)
{
  unsigned char *buf = &vpu->data[vpu->data_segment][vpu->regs[1]];
  unsigned short count = vpu->regs[2];
  unsigned short n = -1;
  unsigned fd = vpu->regs[3];
  struct file_struct *file_struct = vpu->file_struct[fd];
  int c = 0;

  if(count && !memory_allowed(vpu, (long)vpu->regs[1] + count - 1))  return 4;

  if(fd < MAX_FILES && file_struct->open)
  {
    n++;
    if(!count)
    {
      vpu->regs[0] = 0;
      return 0;
    }
    if(!file_struct->is_ioport)
      if(diskio_in_progress)
      {
        wait_diskio:
        vpu_block(vpu);
        return 3;
      }
      else n = read(file_struct->handle, buf, count);
    else if(file_struct->is_ioport == 9) // device
    {
      if(diskio_in_progress)
        goto wait_diskio;
      n = dev_read(file_struct->module, file_struct->device, buf, count);
    }
    else if(file_struct->is_ioport == 11) // read end of a pipe
      n = pipe_read(file_struct, buf, count);
    else while(count--)    
    {
      if(file_struct->is_ioport <= 4) // serial port
        c = serial_read(file_struct->is_ioport-1);
      else if(file_struct->is_ioport == 12) // console
        c = con_stdin_read();
      else goto error; // write end of a pipe or something
        
      if(c == -1)
        break;
      else if(c == -2)
      {
        error:n = 0xFFFF;
        break;
      }
      buf[n++] = c;
      if(file_struct->file_pos)
        ++*file_struct->file_pos;
    }

    if(!n && !(file_struct->flags & OPEN_O_NONBLOCK))
    {
      vpu_block(vpu);
      return 3;
    }
    else if(n == -1)
    {
      if(save_file_errno(vpu, fd) <= 0)
        vpu->error_code = EWOULDBLOCK;
    }
    else if(file_struct->file_pos)
      *file_struct->file_pos += n;

    fd_dup_sync(file_struct);        
  }
  vpu->regs[0] = n;
  return 0;
}

static int pipe_write(struct vpu *vpu, struct file_struct *pipe, char *buf, unsigned count)
{
  unsigned *write_pos = &pipe->pipe_to->bufwpos;
  unsigned *read_pos = &pipe->pipe_to->bufrpos;
  unsigned buffer_end;
  unsigned max_write_count;

  if(!pipe->pipe_to)
  {
    if(send_vpu_signal(vpu, SIGPIPE))
      send_vpu_signal(vpu, SIGABRT);
    return 4;
  }

  buffer_end = *read_pos + PIPE_BUF;
  max_write_count = buffer_end - *write_pos;
    
  if(count > max_write_count) count = max_write_count;
  copy2ring(pipe->pipe_to->buff, buf, *write_pos, PIPE_BUF, count);
  *write_pos += count;

  fd_dup_sync(pipe->pipe_to);

  return count;
}

static int vpu_syscall_write(struct vpu *vpu)
{
  unsigned char *buf = &vpu->data[vpu->data_segment][vpu->regs[1]];
  unsigned short count = vpu->regs[2];
  unsigned short n = -1;
  unsigned fd = vpu->regs[3];
  struct file_struct *file_struct = vpu->file_struct[fd];
  unsigned char r;
  int retval = 0;

  if(count && !memory_allowed(vpu, (long)vpu->regs[1] + count - 1)) return 4;

  if(fd < MAX_FILES && file_struct->open)
  {
    n++;

    if(!count)
    {
      vpu->regs[0] = 0;
      return 0;
    }
    if(!file_struct->is_ioport)
    {
      if(diskio_in_progress)
      {
        wait_diskio:
        vpu_block(vpu);
        return 3;
      }
      n = write(file_struct->handle, buf, count);
      if(file_struct->flags == OPEN_O_SYNC)
        if(fsync(file_struct->handle))
          n = -1;
    }
    else if(file_struct->is_ioport == 9) // device
    {
      if(diskio_in_progress)
        goto wait_diskio;
      n = dev_write(file_struct->module, file_struct->device, buf, count);
    }
    else if(file_struct->is_ioport == 10) // pipe
      n = pipe_write(vpu, file_struct, buf, count);
    else while(count--)
    {     
      if(file_struct->is_ioport <= 4) // serial port
        r = serial_write(file_struct->is_ioport-1, buf[n]);
      else if(file_struct->is_ioport < 9) // parallel port
        r = parallel_write(file_struct->is_ioport-5, buf[n]);
      else if(file_struct->is_ioport == 12) // console
      {
        if((r = con_stdout_write(buf[n])) == 3)
        {
          vpu_sched_yield(vpu);
          retval = r;
        }
      }
      else goto error; // writing to read end of a pipe or something?

      if(r == 1) // out of paper or some other non-critical error that prevents writing, block or return count of written bytes
        break;
      if(r == 2) // catastroof happenedz, return -1
      {
        error:
        n = -1;
        break;
      }
      n++;
    }
    if(!n && !(file_struct->flags & OPEN_O_NONBLOCK))
    {
      vpu_block(vpu);
      return 3;
    }
    else if(n == -1)
    {
      if(save_file_errno(vpu, fd) <= 0)
        vpu->error_code = EWOULDBLOCK;
    }
    else if(file_struct->file_pos)
      *file_struct->file_pos += n;
    fd_dup_sync(file_struct);      
  }
  vpu->regs[0] = n;
  return retval;
}

static int vpu_syscall_fsync(struct vpu *vpu)
{
  unsigned fd = vpu->regs[1];
  struct file_struct *file_struct = vpu->file_struct[fd];
  if(fd < MAX_FILES && file_struct->open)
  {
    if(!file_struct->is_ioport)
    {
      if(vpu->regs[0] = fsync(file_struct->handle))
        save_file_errno(vpu, fd);
    }
    else if(file_struct->is_ioport == 9)
    {
      if(dev_fsync(file_struct->module, file_struct->device))
        save_file_errno(vpu, fd);
    }
    else if(file_struct->is_ioport == 12)
    {
      con_fsync();
    }
    fd_dup_sync(file_struct);      
  }
  return 0;
}

static unsigned openflags2openmod(unsigned short flags)
{
  unsigned retval = O_BINARY;
  if(flags & OPEN_O_WRITE) 
    retval = O_WRONLY | O_TRUNC | O_CREAT | O_BINARY;
  if(flags & OPEN_O_APPEND)
    retval = O_APPEND | O_WRONLY | O_BINARY;
  if(flags & OPEN_O_RW)
    retval |= O_RDWR;
  return retval;
}

static unsigned char detect_ioport(char *filename)
{
  if(strlen(filename) == 4 && filename[3] > 0x30 && filename[3] < 0x35)
  {
    if(!strncmp(filename, "COM", 3))
      return filename[3] - 0x30;
    if(!strncmp(filename, "LPT", 3))
      return filename[3] - 0x30+4;      
  }
  else if(!strcmp(filename, "CON"))
    return 12;
  return 0;
}

static int vpu_syscall_open(struct vpu *vpu)
{
  char *filename = &vpu->data[vpu->data_segment][vpu->regs[3]];
//  unsigned short filemode = vpu->data[vpu->data_segment][vpu->regs[2]]; // permissions...
  unsigned short n, m;
  unsigned short file_descriptor;
  char file_path[MAX_PATH*2+2];
  char *ptr;
  unsigned devnum = 0;
  int i;
  unsigned fileflags = openflags2openmod(vpu->regs[1]);
  struct file_struct *file_struct;
  unsigned char io_type;
  int retval = 0;

  if(!(vpu->privileges & VPU_PRIV_FS_WRITE))
    if(vpu->regs[1] & (OPEN_O_WRITE | OPEN_O_APPEND | OPEN_O_RW))
      goto error2;
  

  n = strlen(filename);
  if(!memory_allowed(vpu, (long)vpu->regs[3] + n))
    return 4;
  if(!n)
  {
    end:
    vpu->regs[0] = 0xFFFF;
    return 0;
  }
      

  for(file_descriptor=0;vpu->file_struct[file_descriptor]->open;file_descriptor++)
    if(file_descriptor == MAX_FILES)
    {
      goto end;
    }
  
  file_struct = vpu->file_struct[file_descriptor];
  if(io_type = detect_ioport(filename))
  {
    if(io_type <= 4)
    {
      if(serial_init(io_type-1, vpu->regs[1]))
        goto error;
    }          
    else if(io_type <= 8)
    {
      if(parallel_init(io_type-5, vpu->regs[1]))
        goto error;
    }
    reset_fd(vpu, file_descriptor);
    goto ioport;
  }
  else if(!strncmp(modfs, filename, strlen(modfs)))
  {
    if(!(vpu->privileges & VPU_PRIV_MODULES))
      goto error2;
    strncpy(file_path, &filename[strlen(modfs)], 8);
    file_path[8] = 0;
    if((i = driver_id(file_path)) < 0)
    {
      for(m=0;file_path[m] >= 'a';m++);
      devnum = strtoul(&file_path[m], &ptr, 0x10);
      file_path[m] = 0;
      if((i = driver_id(file_path)) < 0)
        goto error2;
      if(devnum >= drivers[i].dev_count) goto error2;
    }
    reset_fd(vpu, file_descriptor);
    file_struct->module = i;
    io_type = 9;
    file_struct->device = devnum;
    if(dev_open(file_struct->module, devnum, vpu->regs[1]))
      goto error2;
    goto ioport; 
  }
  else
  {
    if(global_open_files == GLOBAL_MAX_FILES) goto error;
    if(!(vpu->privileges & VPU_PRIV_FS_READ))
      goto end;
    if(!(vpu->privileges & VPU_PRIV_RFS_ACCESS))
    {
      if(!is_subpath(filename, vpu->iwd, vpu->cwd))
        goto end;
    }      
    append_path(vpu, file_path, filename);
    i = open(file_path, fileflags, S_IRWXU);
    if(i == -1)
    {
      error:
      vpu->error_code = errno;
      error2:
      vpu->regs[0] = 0xFFFF;
      return 0;
    }
    else global_open_files++;
    reset_fd(vpu, file_descriptor);
    file_struct->flags |= OPEN_O_NONBLOCK; // real files are nonblocking...
    file_struct->handle = i;
  }
  if(vpu->regs[4])
    if(memory_allowed(vpu, (long)vpu->regs[4] + sizeof(unsigned long)-1))
    {
      file_struct->file_pos = (unsigned long*)&vpu->data[vpu->data_segment][vpu->regs[4]];
      file_struct->filepos_segment = vpu->data_segment;
      file_struct->filepos_offset = vpu->regs[4];
      *file_struct->file_pos = lseek(file_struct->handle, 0, SEEK_CUR);
    }
    else retval = 4;
  ioport:
  file_struct->is_ioport = io_type;
  vpu->regs[0] = file_descriptor;
  file_struct->flags |= vpu->regs[1];

  return retval;
}


static unsigned vpu_fd_close(struct vpu *vpu, unsigned fd)
{
  struct file_struct *file_struct = vpu->file_struct[fd];
  unsigned rc;
  if(file_struct->dup || file_struct->orig)
  {    
    if(file_struct->orig)
    {
      if(file_struct->pipe_from
         && file_struct->pipe_from->pipe_to == file_struct)
      {
        file_struct->pipe_from->pipe_to = file_struct->orig;
        fd_dup_sync(file_struct->pipe_from);
      }
      if(file_struct->pipe_to
         && file_struct->pipe_to->pipe_from == file_struct)
      {
        file_struct->pipe_to->pipe_from = file_struct->orig;
        fd_dup_sync(file_struct->pipe_to);
      }
      file_struct->orig->dup = file_struct->dup;
    }
    if(file_struct->dup)
    {
      if(file_struct->pipe_from
         && file_struct->pipe_from->pipe_to == file_struct)
      {
        file_struct->pipe_from->pipe_to = file_struct->dup;
        fd_dup_sync(file_struct->pipe_from);        
      }
      if(file_struct->pipe_to
         && file_struct->pipe_to->pipe_from == file_struct)
      {
        file_struct->pipe_to->pipe_from = file_struct->dup;
        fd_dup_sync(file_struct->pipe_to);
      }      
      file_struct->dup->orig = file_struct->orig;
    }
    close_ok:rc = 0;
  }
  
  else
  {
    if(!file_struct->is_ioport)
    {
      global_open_files--;
      if(rc = close(file_struct->handle))
        vpu->error_code = errno;
    }
    else if(file_struct->is_ioport == 9)
    {
      if(rc = dev_close(file_struct->module, file_struct->device))
        vpu->error_code = save_file_errno(vpu, fd);
    }
    else
    {
      if(file_struct->is_ioport <= 4)
        close_serialport(file_struct->is_ioport -1);
      else if(file_struct->is_ioport == 10) // close pipe's write end
      {
        if(file_struct->pipe_to)
        {
          file_struct->pipe_to->pipe_from = 0;
          file_struct->pipe_to->flags |= OPEN_O_NONBLOCK; // make read end non-blocking
          fd_dup_sync(file_struct->pipe_to);
        }
      }
      else if(file_struct->is_ioport == 11) // close pipe's read end
      {
        free(file_struct->buff);
        if(file_struct->pipe_from)
        {
          file_struct->pipe_from->pipe_to = 0;
          fd_dup_sync(file_struct->pipe_from);
        }
      }
    }
    goto close_ok;
  }
  memset(file_struct, 0, sizeof(struct file_struct));  
  return rc;
}

static int vpu_syscall_close(struct vpu *vpu)
{
  unsigned fd = vpu->regs[1];
  if(fd < MAX_FILES && vpu->file_struct[fd]->open)
  {
    if(diskio_in_progress &&
       vpu->file_struct[fd]->is_ioport != 10 &&
       vpu->file_struct[fd]->is_ioport != 11)
    {
      vpu_block(vpu);
      return 3;
    }
    vpu->regs[0] = vpu_fd_close(vpu, fd);
  }
  else
    vpu->regs[0] = -1; // TODO: error code...
  return 0;
}

static int vpu_syscall_lseek(struct vpu *vpu)
{
  long lseek_ret;
  unsigned fd = vpu->regs[3];
  struct file_struct *file_struct = vpu->file_struct[fd];
  long seekcnt = *(signed short*)&vpu->regs[1];

  if(vpu->regs[0] & 0x8000)
    seekcnt |= ((unsigned long)vpu->regs[4] << 16);
  
  if(fd < MAX_FILES && file_struct->open && !file_struct->is_ioport)
  {
    switch(vpu->regs[2])
    {
      case 0:
        lseek_ret = lseek(file_struct->handle, seekcnt, SEEK_SET);
        break;
      case 1:
        lseek_ret = lseek(file_struct->handle, seekcnt, SEEK_CUR);
        break;
      case 2:
        lseek_ret = lseek(file_struct->handle, seekcnt, SEEK_END);
    }
    if(lseek_ret == -1)
      vpu->error_code = errno;
    if(file_struct->file_pos)
      *file_struct->file_pos = lseek_ret;
    fd_dup_sync(file_struct);      
    if(vpu->regs[0] & 0x8000)
      vpu->regs[1] = lseek_ret>>16;
    vpu->regs[0] = lseek_ret;
  }
  return 0;
}

static void vpu_sync_dynsegments(struct vpu *vpu)
{
  unsigned n = vpu_count;
  while(n--)
  {
    if(vpus[n]->code == vpu->code && vpus[n] != vpu)
    {
      vpus[n]->dynsegment_count = vpu->dynsegment_count;
      vpus[n]->dynsegments = vpu->dynsegments;
      vpus[n]->dataseg_table_len = vpu->dataseg_table_len;
      vpus[n]->data = vpu->data;
    }
  }
}

static int vpu_allocate_new_segment(struct vpu *vpu, unsigned segment)
{
  void *new_pointer;
  unsigned length = vpu->regs[2];
  unsigned char *segptr = calloc(length, sizeof(unsigned char));

  if(!segptr) return -1;

  if(!vpu->dynsegment_count++)
  {
    if(!(vpu->dynsegments = malloc(vpu->dynsegment_count * sizeof(struct dynsegment))))
    {
      nomemory:
      vpu->dynsegment_count--;
      return -1;
    }
  }
  else
    if(new_pointer = realloc(vpu->dynsegments, vpu->dynsegment_count * sizeof(struct dynsegment)))
      vpu->dynsegments = new_pointer;
    else goto nomemory;
      
  vpu->dynsegments[vpu->dynsegment_count-1].segment = segment;
  vpu->dynsegments[vpu->dynsegment_count-1].length = length;
  
  if(vpu->dataseg_table_len < segment+1) 
    if(new_pointer = realloc(vpu->data, (segment+1) * sizeof(unsigned char*)))
    {
      vpu->data = new_pointer;
      vpu->dataseg_table_len = segment+1;
    }
    else
    {
      vpu->dynsegments = realloc(vpu->dynsegments, --vpu->dynsegment_count * sizeof(struct dynsegment));
      return -1;
    }
    
 
  vpu->data[segment] = segptr;

  vpu_sync_dynsegments(vpu);
  
  return 0;
}

static unsigned char vpu_segment_in_use(struct vpu *vpu, unsigned short segment)
{
  unsigned short n;
  
  if(segment < vpu->data_count)
    return 1;
  n = vpu->dynsegment_count;
  while(n--)
  {  
    if(vpu->dynsegments[n].segment == segment)
      return 1;
  }
  
  return 0;
}

static int vpu_syscall_mmap(struct vpu *vpu)
{
  unsigned char *fpreadp;
  int c;
  unsigned segment = vpu->regs[1];
  unsigned n;
  unsigned long fpos = vpu->regs[4] | ((unsigned long)vpu->regs[5] << 16);
  unsigned fd = vpu->regs[3];
  
  if(!vpu->regs[2])
  {
//    putstr("Cannot allocate zero bytes!");
    vpu->regs[0] = 0xFFFF;
    return 0;
  }
  // find free segment if segment [bx] already used
  if(vpu_segment_in_use(vpu, segment))
  {
    for(segment=vpu->data_count; vpu_segment_in_use(vpu, segment) ;segment++)
    {
      if(segment == 0xFFFF)
      {
        error:
        vpu->regs[0] = 0xFFFF;
        return 0;
      }
    }
  }
  if(vpu_allocate_new_segment(vpu, segment))
  {
    goto error;
  }
  // if file descriptor argument is set, read file contents into newly allocated memory
  if(fd && vpu->file_struct[fd]->open)
  {
    if(vpu->file_struct[fd]->is_ioport)
    {
      vpu->error_code = EACCESS;
      goto error;
    }
    fpreadp = vpu->data[segment];
    lseek(vpu->file_struct[fd]->handle, fpos, SEEK_SET);
    n = vpu->regs[2];
   
    read(vpu->file_struct[fd]->handle, fpreadp, n);
    // save file position to file struct
    if(vpu->file_struct[fd]->file_pos)
      *vpu->file_struct[fd]->file_pos = lseek(vpu->file_struct[fd]->handle, 0, SEEK_CUR);
    fd_dup_sync(vpu->file_struct[fd]);
  }
  vpu->regs[0]=segment;
  return 0;
}

static void vpu_free_dynamic_segment(struct vpu *vpu, unsigned short segment)
{
  free(vpu->data[vpu->dynsegments[segment].segment]);

  memmove(&vpu->dynsegments[segment], &vpu->dynsegments[segment+1],
    (--vpu->dynsegment_count - segment)*sizeof(struct dynsegment));

  if(vpu->dynsegment_count)
    vpu->dynsegments = realloc(vpu->dynsegments, vpu->dynsegment_count*sizeof(struct dynsegment));
  else
  {
    free(vpu->dynsegments);
    vpu->dynsegments = 0;
  }

  vpu_sync_dynsegments(vpu);
}

static int vpu_syscall_munmap(struct vpu *vpu)
{
  unsigned short n = vpu->dynsegment_count;
  // cannot free current segment...
  if(vpu->data_segment == vpu->regs[1])
  {
    vpu->regs[0] = 0xFFFF;
    return 0; 
  }
  while(n--)
    if(vpu->dynsegments[n].segment == vpu->regs[1])
    {
      vpu_free_dynamic_segment(vpu, n);
      vpu->regs[0] = 0;
      return 0;
    }
  vpu->regs[0] = 0xFFFF;

  return 0;
}

static int vpu_syscall_exit(struct vpu *vpu)
{
  set_exiting(vpu);
  if(vpu->parent && !vpu->parent->zombie && vpu->exit_status_off)
    if(segment_exists(vpu->parent, vpu->exit_status_seg) > vpu->exit_status_off+1)
      *(unsigned short*)&vpu->parent->data[vpu->exit_status_seg][vpu->exit_status_off] = vpu->regs[1];

  return 3;
}

// returns argument count to AX
static int vpu_syscall_getargc(struct vpu *vpu)
{
  vpu->regs[0] = vpu->argc;
  return 0;
}

// returns offset [CX] of arg [BX] to AX
static int vpu_syscall_getargv(struct vpu *vpu)
{
  if(vpu->regs[1] < vpu->argc && vpu->regs[2] <= strlen(vpu->argv[vpu->regs[1]]))
  {
    vpu->regs[0] = vpu->argv[vpu->regs[1]][vpu->regs[2]];
  }
  return 0;
}

// spawns new process
void *run_program(char *dir, char *program, char *args, unsigned getkey, unsigned privs);
static int vpu_syscall_spawn(struct vpu *vpu)
{
  char *path = &vpu->data[vpu->data_segment][vpu->regs[2]];
  char *args = &vpu->data[vpu->data_segment][vpu->regs[3]];
  unsigned *pid = (unsigned*)&vpu->data[vpu->data_segment][vpu->regs[1]];
  struct console_struct *return_console;
  unsigned n,m;

  if(!(vpu->privileges & VPU_PRIV_SYSTEM))
    goto error;
  
  n = strlen(path);
  m = strlen(args);  
   
  if(!memory_allowed(vpu, (long)vpu->regs[1] + sizeof(unsigned short)-1) || !memory_allowed(vpu, (long)vpu->regs[2]+n) || !memory_allowed(vpu, (long)vpu->regs[3]+m))
    return 4;

  vpu_cwd(vpu);  
  chdrive(*vpu->cwd);
  
  return_console = run_program(vpu->cwd, path, args, 0, vpu->privileges);

  chdrive(0);

  if(!return_console)
  {
    error:
    vpu->regs[0] = 0xFFFF;
    return 0;
  }
  *pid = return_console->vcpu->pid;
  return_console->vcpu->privileges = vpu->regs[4];
  vpu->regs[0] = 0;
  return 0;
}

static void vpu_fd_makedup(struct vpu *vpu0, unsigned fd0, struct vpu *vpu1, unsigned fd1)
{
  struct file_struct *orig;

  orig = vpu1->file_struct[fd1];
  while(orig->dup)
    orig = orig->dup;
  orig->dup = vpu0->file_struct[fd0];

  *vpu0->file_struct[fd0] = *vpu1->file_struct[fd1];
  vpu0->file_struct[fd0]->orig = orig;
  vpu0->file_struct[fd0]->dup = 0;
  vpu0->file_struct[fd0]->file_pos = 0;
}

inline unsigned char copy_vpu(struct vpu *vpu0, struct vpu *vpu1)
{
  unsigned n;
  void *new_pointer;

  vpu0->code = 0;
  vpu0->data = 0;
  vpu0->interrupts = 0;
  vpu0->data_count=0;
  vpu0->code_count=0;
  vpu0->dynsegments = 0;
  vpu0->dynsegment_count=0;
  vpu0->file_struct=0;
  vpu0->dir_handles=0;
  vpu0->ports=0;
  vpu0->stack = vpu0->secondary_stack = vpu0->primary_stack = 0;
  if(!init_vpu(vpu0, vpu1->stack_size, vpu1->code_size, vpu1->code_count, vpu1->data_size, vpu1->data_count, vpu1->interrupt_count))
  {
    return 0;
  }

  vpu0->psp = vpu1->psp;
  vpu0->ssp = vpu1->ssp;
  vpu0->sp = vpu1->sp;  

  if(vpu1->stack == vpu1->secondary_stack)
    vpu0->stack = vpu0->secondary_stack;

  memcpy(vpu0->primary_stack, vpu1->primary_stack, vpu1->stack_size*sizeof(unsigned short));
  memcpy(vpu0->secondary_stack, vpu1->secondary_stack, vpu1->stack_size*sizeof(unsigned short));  
  for(n=vpu1->code_count;n--;)
    memcpy(vpu0->code[n], vpu1->code[n], vpu1->code_size);
  for(n=vpu1->data_count;n--;)
    memcpy(vpu0->data[n], vpu1->data[n], vpu1->data_size);
  memcpy(vpu0->interrupts, vpu1->interrupts, vpu1->interrupt_count*sizeof(struct vpu_interrupt));

  if(vpu1->dynsegment_count)
  {
    vpu0->dynsegments = malloc(vpu1->dynsegment_count * sizeof(struct dynsegment));
    if(!vpu0->dynsegments)
      return 0;
    vpu0->dataseg_table_len = vpu1->dataseg_table_len;
    new_pointer = realloc(vpu0->data, vpu0->dataseg_table_len*sizeof(char *));
    if(!new_pointer)
      return 0;
    vpu0->data = new_pointer;
    while(vpu0->dynsegment_count < vpu1->dynsegment_count)
    {
      vpu0->dynsegments[vpu0->dynsegment_count] = vpu1->dynsegments[vpu0->dynsegment_count];
      
      vpu0->data[vpu0->dynsegments[vpu0->dynsegment_count].segment] = malloc(vpu0->dynsegments[vpu0->dynsegment_count].length * sizeof(unsigned char));
      if(!vpu0->data[vpu0->dynsegments[vpu0->dynsegment_count].segment])
        return 0;
      memcpy(vpu0->data[vpu0->dynsegments[vpu0->dynsegment_count].segment], vpu1->data[vpu0->dynsegments[vpu0->dynsegment_count].segment], vpu0->dynsegments[vpu0->dynsegment_count].length);
      vpu0->dynsegment_count++;
    }
  }

  for(n=MAX_FILES;n--;)
  {
//    if(vpu1->file_handles[n])
    if(vpu1->file_struct[n]->open)
    {
      vpu_fd_makedup(vpu0, n, vpu1, n);
    }
  }
  for(n=MAX_DIRS;n--;)
  {
    if(vpu1->dir_handles[n])
    {
      vpu_dd_makedup(vpu0, n, vpu1, n);
    }
  }

  vpu0->privileges = vpu1->privileges;
  
  return 1;
}

static unsigned vpu_assign_child(struct vpu *child, struct vpu *parent, unsigned term_sig, unsigned retval_seg, unsigned retval_off)
{
  void *new_pointer;
  if(!parent->child_count++)
  {
    parent->child = malloc(parent->child_count * sizeof(struct vpu*));
    if(!parent->child)
      goto nomemory;
  }
  else
  {    
    new_pointer = realloc(parent->child, parent->child_count * sizeof(struct vpu*));
    if(!new_pointer)
    {
      nomemory:
      parent->child_count--;
      return 0;
    }
    else
      parent->child = new_pointer;
  }
  parent->child[parent->child_count-1] = child;
  child->parent = parent;
  child->termination_signal = term_sig;
  child->exit_status_seg = retval_seg;  
  child->exit_status_off = retval_off;
  return 1;
}

static struct vpu *clone_vpu(struct vpu *parent)
{
  struct vpu *clone = spawn_new_vpu();
  unsigned pid;

  if(!clone)
  {
   nomemory();
   return 0;
  }
  pid = clone->pid;
  // copy all registers, stack pointer etc.  
  *clone = *parent;
  // reserve pid
  clone->pid = pid;
  clone->child = 0;
  clone->child_count = 0;
  clone->parent=0;
  clone->sys_wait = 0;
  clone->byteregs = (unsigned char*)&clone->regs;
  return clone;
}

static int vpu_syscall_fork(struct vpu *vpu)
{
  struct vpu *forked_clone = clone_vpu(vpu);
  unsigned n;

  if(!forked_clone)
  {
    outofmemory:
    vpu->regs[0] = 0xFFFF;
    return 0;
  }
  forked_clone->is_clone = 0;  

  
  if(!copy_vpu(forked_clone, vpu) || !vpu_assign_child(forked_clone, vpu, vpu->regs[3], vpu->data_segment, vpu->regs[4]))
  {
    nomemory();
    delete_vpu(forked_clone);
    goto outofmemory;
  }

  vpu->regs[0] = forked_clone->pid;
  forked_clone->regs[0] = 0;

  return 0;
}

static int vpu_syscall_clone(struct vpu *vpu)
{
  struct vpu *clone = clone_vpu(vpu);
  struct vpu *parent = vpu;

  if(!clone)
  {
    outofmemory:
    vpu->regs[0] = 0xFFFF;
    return 0;
  }
  clone->is_clone=1;    
  // allocate own stack for the child
  clone->primary_stack = calloc(clone->stack_size, sizeof(unsigned short));
  clone->secondary_stack = calloc(clone->stack_size, sizeof(unsigned short));

  if(!clone->primary_stack || !clone->secondary_stack || !vpu_assign_child(clone, parent, vpu->regs[3], vpu->data_segment, vpu->regs[4]))
  {
    nomemory();
    delete_vpu(clone);
    goto outofmemory;
  }
  if(vpu->stack == vpu->primary_stack)
    clone->stack = clone->primary_stack;
  else
    clone->stack = clone->secondary_stack;
      
  clone->psp = clone->ssp = clone->sp = clone->stack_size-1;

  clone->code_segment = vpu->regs[1];
  clone->ip = vpu->regs[2];
 
  vpu->regs[0] = clone->pid;
  clone->regs[0] = vpu->regs[5];

  return 0;
}

inline void remove_fd_filepos_ptrs(struct vpu *vpu)
{
  unsigned n = MAX_FILES;
  while(n--)
  {
    vpu->file_struct[n]->file_pos = 0;
  }
}

char **split_args(unsigned *argc, char *args, char *programname);

static void reset_vpu(struct vpu *vpu);

// todo: test this
static void vpu_kill_clones(struct vpu *vpu)
{
  unsigned n = vpu->child_count;

  while(n--)
    if(vpu->child[n]->is_clone)
      free_vpu_resources(vpu->child[n]);
}

char *find_executable(char *program, unsigned bytecodeprog);

static int vpu_syscall_exec(struct vpu *vpu)
{
  char *program = &vpu->data[vpu->data_segment][vpu->regs[1]];
  char *args = &vpu->data[vpu->data_segment][vpu->regs[2]];
  unsigned n, m;
  char programstr[MAX_PATH*2+2];
  FILE *fp;
  char *found_program;
  int rc;

  n = strlen(program);
  m = strlen(args);
  if(!memory_allowed(vpu, (long)vpu->regs[1] + n) || !memory_allowed(vpu, (long)vpu->regs[2] + m))
    return 4;

  if(!n)
  {
    error:
    vpu->regs[0] = -1;
    return 0;
  }

  if(!strchr(program, '\\'))
  {
    chdrive(*vpu->cwd);
    vpu_cwd(vpu);
    found_program = find_executable(program, 1);
    chdrive(0);
    if(found_program)
      program = found_program;
  }

  append_path(vpu, programstr, program);

  if(!(vpu->privileges & VPU_PRIV_RFS_ACCESS))
  {
    if(!is_subpath(program, vpu->iwd, vpu->cwd))
      goto error;
  }

  if(!(fp = fopen(programstr, "rb")))
  {
    goto error;
  }
  fclose(fp);

  // if VPU is a clone, switch to parent vpu
  while(vpu->is_clone)
  {
    strcpy(vpu->parent->cwd, vpu->cwd);
    vpu = vpu->parent;
  }
  // then kill all clones
  vpu_kill_clones(vpu);

  free_vpu_args(vpu);
  
  vpu->argv = split_args(&vpu->argc, args, program);    
  
  while(vpu->dynsegment_count)
    vpu_free_dynamic_segment(vpu, 0);

  
  while(vpu->data_count)
    free(vpu->data[--vpu->data_count]);
  free(vpu->data);
  vpu->data=0;
  free(vpu->ports);
  vpu->ports = 0;  
  while(vpu->code_count)
    free(vpu->code[--vpu->code_count]);
  free(vpu->code);
  vpu->code=0;
  free(vpu->interrupts);
  vpu->interrupts=0;
  free(vpu->primary_stack);
  free(vpu->secondary_stack);
  vpu->primary_stack = vpu->secondary_stack = 0;

  if((rc = load_program(programstr, vpu)))
  {
    if(rc == 1) nomemory();
    set_exiting(vpu);
  }

  remove_fd_filepos_ptrs(vpu);

  strcpy(vpu->iwd, vpu->cwd);

  reset_vpu(vpu);

  return 0;
}

long timezone = 0;

static int vpu_syscall_gettime(struct vpu *vpu)
{
  struct unixtime utime = unixtime(timezone);
//  vpu->regs[0] = utime.time>>48;
//  vpu->regs[1] = utime.time>>32;
  vpu->regs[0] = 0;
  vpu->regs[1] = 0;
  vpu->regs[2] = utime.time>>16;
  vpu->regs[3] = utime.time;

  vpu->regs[4] = utime.nanoseconds>>16;
  vpu->regs[5] = utime.nanoseconds;
  return 0;
}

static int vpu_syscall_sound(struct vpu *vpu)
{
  sound(vpu->regs[1]);
  return 0;
}

static int vpu_syscall_nosound(struct vpu *vpu)
{
  nosound();
  return 0;
}

static int vpu_syscall_waitscrtrace(struct vpu *vpu)
{
  if(!vpu_has_active_window(vpu))
    return 3;

  if(videodriver == -1)
  {
    __asm
    {
      mov dx, 0x03da
      jump1:    
      in al, dx
      test al, 0x08
      jnz jump1
      jump2:
      in al, dx
      test al, 0x08
      jz jump2
    }
  }
  return 3;
}

static int vpu_syscall_sleep(struct vpu *vpu)
{
  vpu->pause = 1;
  vpu->sleep |= vpu->regs[4];
  vpu->sleep |= ((unsigned long)vpu->regs[3])<<16;
//  vpu->sleep |= vpu->regs[2]>>32;
//  vpu->sleep |= vpu->regs[1]>>48;
  return 3;
}

/*
  Returns pointer to the VPU struct by the given PID
*/

static struct vpu *pid_to_vpu(unsigned pid)
{
  unsigned n = vpu_count;
  while(n--)
    if(vpus[n]->pid == pid)
      return vpus[n];
  return 0;
}


static int vpu_syscall_sendsignal(struct vpu *vpu)
{
  struct vpu *target;

  if(!(vpu->privileges & VPU_PRIV_SIGNALS))
  {
    error:
    vpu->regs[0] = -1;
    return 0;
  }
  
  if(!vpu->regs[1])
  {
    send_vpu_signal(vpu, vpu->regs[2]);
  }
  else if(target = pid_to_vpu(vpu->regs[1]))
  {
    if(send_vpu_signal(target, vpu->regs[2]))
      goto error;
  }
  else goto error;
  vpu->regs[0] = 0;

  return 0;
}

static int vpu_syscall_procinfo(struct vpu *vpu)
{
  char *process_name;
  unsigned n = window_count;
  static char zombie[] = "Zombie";
  
  if(vpu->regs[1] >= vpu_count)
  {
    vpu->regs[0] = 0;
    return 0;
  }
  else
  {
    vpu->regs[0] = vpus[vpu->regs[1]]->pid;
    if(vpus[vpu->regs[1]]->zombie)
      process_name = zombie;
    else if(vpus[vpu->regs[1]]->argc/* && *vpus[vpu->regs[1]]->argv[0]*/)
      process_name = vpus[vpu->regs[1]]->argv[0];
    else
      while(n--)
        if(windows[n]->function_pointer == vpuconsole_program
          && ((struct console_struct*)windows[n]->function_status)->vcpu == vpus[vpu->regs[1]])
        {
          process_name = windows[n]->title;
          break;
        }
    if(memory_allowed(vpu, (long)vpu->regs[2]+strlen(process_name)))
      strcpy(&vpu->data[vpu->data_segment][vpu->regs[2]], process_name);                  
    else
      return 4;
  }

  return 0;
}

extern char cwd[];

static int vpu_syscall_chdir(struct vpu *vpu)
{
  unsigned n;
  char *dirname = &vpu->data[vpu->data_segment][vpu->regs[1]];
  char dir_path[MAX_PATH*2+2];

  n = strlen(dirname);
  if(!memory_allowed(vpu, (long)vpu->regs[1] + n))
    return 4;

  if(!(vpu->privileges & VPU_PRIV_FS_READ))
  {
    goto error;
  }
  if(!(vpu->privileges & VPU_PRIV_RFS_ACCESS))
  {
    if(!is_subpath(dirname, vpu->iwd, vpu->cwd))
      goto error;
  }

  if(dirname[1] == ':')
  {
    if(chdrive(dirname[0]))
    {
      error:
      vpu->regs[0] = -1;
      return 0;
    }
  }
  else if(vpu->cwd[1] == ':')
    if(chdrive(vpu->cwd[0]))
      goto error;

  append_path(vpu, dir_path, dirname);

  vpu->regs[0] = chdir(dir_path);
  getcwd(&vpu->cwd, MAX_PATH);

  chdrive(0);

  chdir(cwd);

  vpu_cwd(vpu);

  return 0;
}

static int vpu_syscall_opendir(struct vpu *vpu)
{
  unsigned dir_descriptor;
  unsigned n;
  char *dirname = &vpu->data[vpu->data_segment][vpu->regs[1]];
  char path[85];
  
  n = strlen(dirname);

  if(!memory_allowed(vpu, (long)vpu->regs[1] + n))
    return 4;
  if(n>80)
    goto error;

  if(!*dirname)
    dirname = vpu->cwd;

  if(!(vpu->privileges & VPU_PRIV_FS_READ))
    goto error;
  if(!(vpu->privileges & VPU_PRIV_RFS_ACCESS))
  {
    if(!is_subpath(dirname, vpu->iwd, vpu->cwd))
      goto error;
  }

  sprintf(path, "%s\\*.*", dirname);
  
  for(dir_descriptor=0;vpu->dir_handles[dir_descriptor] && dir_descriptor < MAX_DIRS;dir_descriptor++);

  if(dir_descriptor < MAX_DIRS)
  {
    vpu->dir_handles[dir_descriptor] = calloc(1, sizeof(struct dir_struct));
    if(!vpu->dir_handles[dir_descriptor])
    {
      nomemory();
      goto error;
    }
    if(findfirst(path, 0xFF, &vpu->dir_handles[dir_descriptor]->fileinfo))
    {
      free(vpu->dir_handles[dir_descriptor]);
      vpu->dir_handles[dir_descriptor] = 0;          
      goto error;
    }
    vpu->regs[0] = dir_descriptor;   
  }
  else
  {
    error:
    vpu->regs[0] = 0xFFFF;
  }

  return 0;
}

static int vpu_syscall_readdir(struct vpu *vpu)
{
  if(vpu->regs[1] < MAX_DIRS && vpu->dir_handles[vpu->regs[1]] && !vpu->dir_handles[vpu->regs[1]]->error)
  {
    if(memory_allowed(vpu, (long)vpu->regs[2] + strlen(vpu->dir_handles[vpu->regs[1]]->fileinfo.name)))
      strcpy(&vpu->data[vpu->data_segment][vpu->regs[2]], vpu->dir_handles[vpu->regs[1]]->fileinfo.name);
    else
      return 4;
    vpu->regs[3] = vpu->dir_handles[vpu->regs[1]]->fileinfo.attrib;      
    vpu->regs[0] = 0;
    vpu->dir_handles[vpu->regs[1]]->error = findnext(&vpu->dir_handles[vpu->regs[1]]->fileinfo);
  }
  else
    vpu->regs[0] = 0xFFFF;

  return 0;
}

static void vpu_dd_close(struct vpu *vpu, unsigned dd)
{
  if(vpu->dir_handles[dd]->dup || vpu->dir_handles[dd]->orig)
  {
    if(vpu->dir_handles[dd]->dup)
      vpu->dir_handles[dd]->dup->orig = vpu->dir_handles[dd]->orig;
    if(vpu->dir_handles[dd]->orig)
      vpu->dir_handles[dd]->orig->dup = vpu->dir_handles[dd]->dup;
  }
  else free(vpu->dir_handles[dd]);
  vpu->dir_handles[dd] = 0;
}

static int vpu_syscall_closedir(struct vpu *vpu)
{
  if(vpu->regs[1] < MAX_DIRS && vpu->dir_handles[vpu->regs[1]])
  {
    vpu_dd_close(vpu, vpu->regs[1]);
  }
  return 0;
}

typedef struct { 

    unsigned short  twosecs : 5;    /* seconds / 2 */ 

    unsigned short  minutes : 6;    /* minutes (0,59) */ 

    unsigned short  hours   : 5;    /* hours (0,23) */ 

} ftime_t; 
typedef struct { 

    unsigned short  day     : 5;    /* day (1,31) */ 

    unsigned short  month   : 4;    /* month (1,12) */ 

    unsigned short  year    : 7;    /* 0 is 1980 */ 

} fdate_t; 
static int vpu_syscall_stat(struct vpu *vpu)
{
  char *filename = &vpu->data[vpu->data_segment][vpu->regs[2]];
  unsigned char *return_struct = &vpu->data[vpu->data_segment][vpu->regs[1]];
  unsigned char return_offset = 0;
  unsigned short n;
  struct find_t fileinfo;
  unsigned year;
  unsigned char month, day, hour, min, sec;     

  n = strlen(filename);
  if(!memory_allowed(vpu, (long)vpu->regs[2] + n))
    return 4;
  
  if(findfirst(filename, 0xFF, &fileinfo))
  {
    vpu->regs[0] = 0xFFFF;
    return 0;
  }
  else
  {
//    if(memory_allowed(vpu, (long)vpu->regs[1]+return_offset))
    if(memory_allowed(vpu, (long)vpu->regs[1]+1+sizeof(unsigned long)+sizeof(unsigned long long)))
      *return_struct = fileinfo.attrib;
    else
      return 4;
    vpu->regs[0] = 0;
    return_offset++;
//    if(memory_allowed(vpu, (long)vpu->regs[1]+return_offset+sizeof(unsigned long)-1))
    *(unsigned long*)&return_struct[return_offset] = fileinfo.size;
    return_offset += sizeof(unsigned long);

    year = (*(fdate_t*)(&fileinfo.wr_date)).year+1980;
    month = (*(fdate_t*)(&fileinfo.wr_date)).month;
    day = (*(fdate_t*)(&fileinfo.wr_date)).day;
    hour = (*(ftime_t*)(&fileinfo.wr_time)).hours;
    min = (*(ftime_t*)(&fileinfo.wr_time)).minutes;
    sec = (*(ftime_t*)(&fileinfo.wr_time)).twosecs<<1;    
    
//    if(memory_allowed(vpu, (long)vpu->regs[1]+return_offset+sizeof(unsigned long long)-1))
      *(unsigned long long*)&return_struct[return_offset] = human2unixtime(year, month, day, hour, min, sec);
  }

  return 0;
}

static int vpu_syscall_setpriority(struct vpu *vpu)
{
  struct vpu *tvpu;
  
  if(!vpu->regs[1])
    vpu->priority = vpu->regs[2];
  else
    if(tvpu = pid_to_vpu(vpu->regs[1]))
    {
      tvpu->priority = vpu->regs[2];
      vpu->regs[0] = 0;
    }
    else
      vpu->regs[0] = -1;

  return 0;
}

static int vpu_syscall_getpriority(struct vpu *vpu)
{
  struct vpu *tvpu;
  
  if(!vpu->regs[1])
    vpu->regs[0] = vpu->priority;
  else
    if(tvpu = pid_to_vpu(vpu->regs[1]))
      vpu->regs[0] = tvpu->priority;
    else
      vpu->regs[0] = -1;

  return 0;
}

static int vpu_syscall_loadmod(struct vpu *vpu)
{
  unsigned n, m;
  char *filename = &vpu->data[vpu->data_segment][vpu->regs[1]];
  char *modname = &vpu->data[vpu->data_segment][vpu->regs[2]];

  if(!(vpu->privileges & VPU_PRIV_SYSTEM))
  {
    vpu->regs[0] = -1;
    return 0;
  }

  n = strlen(filename);
  m = strlen(modname);

  if(!memory_allowed(vpu, (long)vpu->regs[1] + n) || !memory_allowed(vpu, (long)vpu->regs[2] + m))
  {
    return 4;
  }

  vpu->regs[0] = load_driver(filename, modname);  

  return 0;
}

int unload_module(char *);

static int vpu_syscall_unloadmod(struct vpu *vpu)
{
  char *modname = &vpu->data[vpu->data_segment][vpu->regs[1]];
  unsigned l = strlen(modname);

  if(memory_allowed(vpu, (long)vpu->regs[1]+l))
    vpu->regs[0] = unload_module(modname);
  else
    return 4;

  return 0;
}

static int vpu_syscall_cmdmod(struct vpu *vpu)
{
  unsigned n, m;
  int i;
  char *devname = &vpu->data[vpu->data_segment][vpu->regs[1]];
  char *args = &vpu->data[vpu->data_segment][vpu->regs[2]];
  unsigned devnum = 0;
  char devname2[9];
  char *ptr;

  if(!(vpu->privileges & VPU_PRIV_MODULES))
  {
    goto error;
  }

  n = strlen(devname);
  m = strlen(args);

  if(!memory_allowed(vpu, (long)vpu->regs[1] + n) || !memory_allowed(vpu, (long)vpu->regs[2] + m))
  {
    return 4;
  }

  strncpy(devname2, devname, 8);
  devname2[8] = 0;

  if((i = driver_id(devname2)) < 0)
  {
    for(n=0;devname2[n] >= 'a';n++);
    devnum = strtoul(&devname2[n], &ptr, 0x10);
    devname2[n] = 0;
    if((i = driver_id(devname2)) < 0)
      goto error;
    if(devnum >= drivers[i].dev_count)
    {
      error:
      vpu->regs[0] = 0xFFFF;
      return 0;
    }
  }
 
  if(!memory_allowed(vpu, (long)vpu->regs[2] + drivers[i].cmd_func_arglen))
    return 4;

  vpu->regs[0] = dev_cmd(i, devnum, args);

  if(vpu->regs[0] == -1)
    vpu->error_code = dev_geterror(i, devnum);

  return 0;
}

static int vpu_syscall_dup(struct vpu *vpu)
{
  unsigned oldfd = vpu->regs[1];
  unsigned file_descriptor;

  for(file_descriptor=0;vpu->file_struct[file_descriptor]->open && file_descriptor < MAX_FILES;file_descriptor++);

  if(file_descriptor < MAX_FILES && oldfd < MAX_FILES && vpu->file_struct[oldfd]->open)
  {
    vpu_fd_makedup(vpu, file_descriptor, vpu, oldfd);
    
    vpu->regs[0] = file_descriptor;
  }
  else
    vpu->regs[0] = 0xFFFF;

  return 0;
}

static int vpu_syscall_dup2(struct vpu *vpu)
{
  unsigned oldfd = vpu->regs[1];
  unsigned newfd = vpu->regs[2];

  if(oldfd < MAX_FILES && newfd < MAX_FILES && vpu->file_struct[oldfd]->open)
  {
    if(oldfd == newfd)
    {
      vpu->regs[0] = newfd;
      return 0;
    }    
    if(vpu->file_struct[newfd]->open)
      vpu_fd_close(vpu, newfd);

    vpu_fd_makedup(vpu, newfd, vpu, oldfd);                         
    
    vpu->regs[0] = newfd;
  }
  else
    vpu->regs[0] = 0xFFFF;  

  return 0;
}

static int vpu_syscall_pipe(struct vpu *vpu)
{
  unsigned readend, writeend;
  void *buff;

  for(readend=0;vpu->file_struct[readend]->open && readend < MAX_FILES;readend++);
  for(writeend=readend+1;vpu->file_struct[writeend]->open && writeend < MAX_FILES;writeend++);

  if(writeend < MAX_FILES)
  {
    buff = malloc(PIPE_BUF * sizeof(char));
    if(!buff)
      goto error;
    reset_fd(vpu, readend);
    vpu->file_struct[readend]->buff = buff;
    reset_fd(vpu, writeend);
    vpu->file_struct[writeend]->pipe_to = vpu->file_struct[readend];
    vpu->file_struct[readend]->pipe_from = vpu->file_struct[writeend];
    vpu->file_struct[writeend]->is_ioport = 10;
    vpu->file_struct[readend]->is_ioport = 11;
  }
  else
  {
    error:
    vpu->regs[0] = 0xFFFF;
    return 0;
  }
  vpu->regs[0] = 0;
  vpu->regs[1] = readend;
  vpu->regs[2] = writeend;

  return 0;
}

static int vpu_syscall_mkdir(struct vpu *vpu)
{
  unsigned n;
  char *dirname = &vpu->data[vpu->data_segment][vpu->regs[1]];
  
  n = strlen(dirname)+1;
  if(!memory_allowed(vpu, (long)vpu->regs[1] + n))
    return 4;

  if(n > 80)
  {
    vpu->regs[0] = 0xFFFF;
    return 0;
  }

  vpu->regs[0] = mkdir(dirname);

  return 0;
}

static int vpu_syscall_sendfd(struct vpu *vpu)
{
  struct vpu *receiver = pid_to_vpu(vpu->regs[1]);
  unsigned oldfd = vpu->regs[2];
  unsigned newfd;

  if(!receiver)
  {
    error:
    vpu->regs[0] = 0xFFFF;
    return 3;
  }

  if(!vpu->file_struct[oldfd]->open) goto error;

  if(receiver->interrupt_count < SIGFDRECVD || !receiver->interrupts[SIGFDRECVD].address)
    goto error;

  for(newfd=0;receiver->file_struct[newfd]->open;newfd++)
    if(newfd == MAX_FILES) goto error;

  if(send_vpu_signal(receiver, SIGFDRECVD))
    vpu_block(vpu);

  vpu_fd_makedup(receiver, newfd, vpu, vpu->regs[2]);

  receiver->ports[0] = newfd;

  vpu->regs[0] = 0;

  return 0;
}

static int vpu_syscall_sethwint(struct vpu *vpu)
{
  unsigned hw_int = vpu->regs[1];
  unsigned vpu_int = vpu->regs[2];

  if(!set_vpu_hw_int(vpu, hw_int, vpu_int, vpu->regs[3]))
    vpu->regs[0] = 0xFFFF;
  else
    vpu->regs[0] = 0;
  return 0;
}

static int vpu_syscall_unsethwint(struct vpu *vpu)
{
  unsigned hw_int = vpu->regs[1];
  unsigned n;

  for(n=vpu_hwint_count;n--;)
    if(vpu_hardware_int_table[n].vpu == vpu && vpu_hardware_int_table[n].hw_int == hw_int)
    {
      remove_vpu_hw_int(vpu, vpu_hardware_int_table[n].vpu_int);
      vpu->regs[0] = 0;
      return 0;
    }
  vpu->regs[0] = 0xFFFF;
  return 0;
}

static int vpu_syscall_geterrno(struct vpu *vpu)
{
  vpu->regs[0] = vpu->error_code;
  return 0;
}

static int vpu_syscall_seterrno(struct vpu *vpu)
{
  vpu->error_code = vpu->regs[1];
  return 0;
}

static int vpu_syscall_unlink(struct vpu *vpu)
{
  char *filename = &vpu->data[vpu->data_segment][vpu->regs[1]];

  if(!memory_allowed(vpu, (long)vpu->regs[1] + strlen(filename)))
    return 4;

  vpu_cwd(vpu);

  if(!remove(filename))
  {
    vpu->regs[0] = 0;
    return 0;
  }

  vpu->regs[0] = 0xFFFF;
  vpu->error_code = errno;

  return 0;
}

static int vpu_syscall_rmdir(struct vpu *vpu)
{
  char *dirname = &vpu->data[vpu->data_segment][vpu->regs[1]];

  if(!memory_allowed(vpu, (long)vpu->regs[1] + strlen(dirname)+1))
    return 4;

  vpu_cwd(vpu);

  if(!rmdir(dirname))
  {
    vpu->regs[0] = 0;
    return 0;
  }

  vpu->regs[0] = 0xFFFF;
  vpu->error_code = errno;

  return 0;
}

static int vpu_syscall_setprivs(struct vpu *vpu)
{
  unsigned pid = vpu->regs[1];
  unsigned privs = vpu->regs[2];
  struct vpu *tvpu;

  if(!(vpu->privileges & VPU_PRIV_ROOT))
    goto error;

  if(!pid) pid = vpu->pid;

  if(tvpu = pid_to_vpu(pid))
  {
    tvpu->privileges = privs;
    vpu->regs[0] = 0;
  }
  else
    error:vpu->regs[0] = -1;
  return 0;
}

static int vpu_syscall_getkernelparm(struct vpu *vpu)
{
  if((vpu->privileges & VPU_PRIV_RFS_ACCESS) &&
    vpu->regs[1] < sizeof(kernel_parm) / sizeof(unsigned *))
  {
    vpu->regs[0] = *kernel_parm[vpu->regs[1]];
  }
  else
  {
    vpu->regs[0] = -1;
  }
  return 0;
}

static int vpu_syscall_setkernelparm(struct vpu *vpu)
{
  if((vpu->privileges & VPU_PRIV_SYSTEM) &&
    vpu->regs[1] < sizeof(kernel_parm) / sizeof(unsigned *))
  {
    *kernel_parm[vpu->regs[1]] = vpu->regs[2];
    vpu->regs[0] = 0;
  }
  else
  {
    vpu->regs[0] = -1;
  }
  return 0;
}

static int vpu_syscall_getcwd(struct vpu *vpu)
{
  char *dest = &vpu->data[vpu->data_segment][vpu->regs[1]];
  unsigned n;
  unsigned max_len = vpu->regs[2];

  if(!memory_allowed(vpu, (long)vpu->regs[1]+max_len))
  {
    return 4;
  }

  for(n=0;vpu->cwd[n] && n < max_len;n++)
    *dest++ = vpu->cwd[n];
  vpu->regs[0] = n;
  return 0;
}

static int vpu_syscall_powersave(struct vpu *vpu)
{
  vpu->blocking = 1;
  return 3;
}

static int vpu_syscall_getenv(struct vpu *vpu)
{
  unsigned envnum = vpu->regs[1];
  unsigned target_offset = vpu->regs[3];
  unsigned max_len = vpu->regs[2];

  if(envnum >= vpu->env_count)
  {
    vpu->regs[0] = -1;
  }
  else if(memory_allowed(vpu, (long)target_offset + max_len))
  {
    strncpy(&vpu->data[vpu->data_segment][target_offset], vpu->envs[envnum],
      max_len);
  }
  else return 4;
  return 0;
}

static int vpu_syscall_setenv(struct vpu *vpu)
{
  unsigned name_offset = vpu->regs[1];
  unsigned value_offset = vpu->regs[2];
  unsigned overwrite = vpu->regs[3];
  char *name = &vpu->data[vpu->data_segment][name_offset];
  char *value = &vpu->data[vpu->data_segment][value_offset];
  unsigned envnum;
  struct vpu *svpu;

  if(!memory_allowed(vpu, (long)name_offset + strlen(name)+1)
     || overwrite<2 && !memory_allowed(vpu, (long)value_offset + strlen(value)+1))
  {
    return 4;
  }

  if(svpu = vpu_shares_envs(vpu))
    if(vpu->regs[0] = vpu_copy_envs(vpu, svpu))
      return 0;

  if(overwrite == 2)
    vpu_unsetenv(vpu, name);
  else
    vpu_setenv(vpu, name, value, overwrite);
  vpu->regs[0] = 0;

  return 0;
}

int (*vpu_instr_syscall[])(struct vpu *) =
{
  vpu_syscall_read, // 0
  vpu_syscall_write, // 1
  vpu_syscall_open, // 2
  vpu_syscall_close, // 3
  vpu_syscall_chdir, // 4
  vpu_syscall_opendir, // 5
  vpu_syscall_readdir, // 6
  vpu_syscall_closedir, // 7
  vpu_syscall_lseek, // 8
  vpu_syscall_mmap, // 9
  vpu_syscall_stat, // 10
  vpu_syscall_munmap, // 11
  vpu_syscall_exit, // 12
  vpu_syscall_getargc, // 13
  vpu_syscall_getargv, // 14
  vpu_syscall_spawn, // 15
  vpu_syscall_clone, // 16
  vpu_syscall_fork, // 17
  vpu_syscall_gettime, // 18
  vpu_syscall_sound, // 19
  vpu_syscall_nosound, // 20
  vpu_syscall_waitscrtrace, // 21
  vpu_syscall_sleep, // 22
  vpu_syscall_sendsignal, // 23
  vpu_syscall_procinfo, // 24
  vpu_syscall_getpriority, // 25
  vpu_syscall_setpriority, // 26
  vpu_syscall_fsync, // 27
  vpu_syscall_loadmod, // 28
  vpu_syscall_cmdmod, // 29
  vpu_syscall_exec, // 30,
  vpu_syscall_dup, // 31,
  vpu_syscall_dup2, // 32,
  vpu_syscall_pipe, // 33
  vpu_syscall_mkdir, // 34
  vpu_syscall_sendfd, // 35
  vpu_syscall_sethwint, // 36
  vpu_syscall_unsethwint, // 37
  vpu_syscall_geterrno, // 38
  vpu_syscall_seterrno, // 39
  vpu_syscall_unlink, // 40
  vpu_syscall_rmdir, // 41
  vpu_syscall_setprivs, // 42
  vpu_syscall_getkernelparm, // 43
  vpu_syscall_setkernelparm, // 44
  vpu_syscall_getcwd, // 45
  vpu_syscall_powersave, // 46
  vpu_syscall_unloadmod, // 47
  vpu_syscall_getenv, // 48
  vpu_syscall_setenv // 49
};

static int vpu_disk_busy(struct vpu *vpu, unsigned function)
{
//  unsigned function = vpu->regs[0] & 0x7FFF;
  if(function == 2
     || function > 3 && function <= 12
//     || function >= 15 && function <= 17
     || function == 15
     || function >= 27 && function <= 30
//     || function >= 34 && function <= 37
     || function == 34
     || function == 40
     || function == 41
/*     || function == 47 || function == 49*/)
    if(diskio_in_progress)
    {
      vpu_block(vpu);
      return 1;
    }
  return 0;
}

static int vpu_instr_sys(struct vpu *vpu, unsigned flags)
{
  int rc;
  unsigned function = vpu->regs[0] & 0x7FFF;
  if(vpu_disk_busy(vpu, function))
    return 3;
  if(function < sizeof(vpu_instr_syscall) / sizeof(void*) && vpu_instr_syscall[function])
  {
    vpu->sys_wait = 1;
    rc = (*vpu_instr_syscall[function])(vpu);
    vpu->sys_wait = 0;
  }
  else
  {
    if(send_vpu_signal(vpu, SIGSYS))
      send_vpu_signal(vpu, SIGABRT);
    rc = 4;
  }

  return rc;
}
