int dev_cmd(unsigned driver, int device, void __far *cmd);
int dev_open(unsigned driver, unsigned device, unsigned mode);
int dev_write(unsigned driver, unsigned device, char *buff, unsigned count);
int dev_close(unsigned driver, unsigned device);
int dev_read(unsigned driver, unsigned device, char *buff, unsigned count);
int dev_fsync(unsigned driver, int device);
int dev_geterror(unsigned driver, int device);

#include "errdef.c"

static int save_file_errno(struct vpu *vpu, unsigned fd)
{
  
  if(!vpu->file_struct[fd]->is_ioport)
    vpu->error_code = errno;
  else if(vpu->file_struct[fd]->is_ioport == 9)
    vpu->error_code = dev_geterror(vpu->file_struct[fd]->module, vpu->file_struct[fd]->device);
  else vpu->error_code = 0;

  return vpu->error_code;
}
