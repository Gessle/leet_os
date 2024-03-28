/*
  Struct for drivers

  dev_name = string that contains the name of the device

  dev_count = count of devices
    0 = driver loaded but no devices available
    -1 = driver is one-device type driver, no multiple devices possible
    >0 = many devices, device names are postfixed with the index

  cmd_func_arglen = length of the argument array of the cmd_func function.

  Calling conventions:
  read_func: device number (if available) to ax, returns to ax
  write_func: data to ax, device number (if available) to bx, returns to ax
  open_func: mode to ax, device number (if available) to bx, returns to ax
  close_func: device number (if available) to ax, returns to ax
  lseek_func: offset to ax, device number (if available) to bx, returns to ax
  fsync_func: device number (if available) to ax, returns to ax
  cmd_func: pointer to command to DX:AX, device number (if available) to bx, returns to ax
*/

struct driver
{
  char dev_name[6];
  int dev_count;

  
  int __far (*read_func)();
  int __far (*write_func)();
  int __far (*open_func)();
  int __far (*close_func)();
  int __far (*lseek_func)();
  int __far (*fsync_func)();
  int __far (*error_func)();
  int __far (*unload_func)();

  int __far (*cmd_func)();

  unsigned cmd_func_arglen;
};
