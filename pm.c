struct power_manager_status
{
  unsigned power_loaded:1;
  unsigned cpu_throttled:1;
}
  power_manager_status;

unsigned int power_installation_check(void);
#pragma aux power_installation_check = \
  "mov ax, 0x5400" \
  "mov bx, 0x504D" \
  "int 0x2F" \
  value [bx] modify [ax bx];

void init_power_manager(void)
{
  power_manager_status.power_loaded = (power_installation_check() == 0x5354);
}

void apm_throttle_cpu(void);
#pragma aux apm_throttle_cpu = \
  "mov ax, 0x5402" \
  "int 0x2F" \
  modify [ax];
  
void throttle_cpu(void)
{
  if(power_manager_status.power_loaded && !power_manager_status.cpu_throttled)
  {
    power_manager_status.cpu_throttled = 1;
    apm_throttle_cpu();
  }
  else
    halt();
}

void apm_restore_cpu(void);
#pragma aux apm_restore_cpu = \
  "mov ax, 0x5403" \
  "int 0x2F" \
  modify [ax];

void restore_cpu_clock(void)
{
  if(power_manager_status.power_loaded && power_manager_status.cpu_throttled)
  {
    power_manager_status.cpu_throttled = 0;
    apm_restore_cpu();
  }
}

