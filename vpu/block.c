static int process_is_stopped(struct vpu *vpu)
{
  if(vpu->pause)
  {
    if(vpu->sleep && vpu->sleep <= unixtime(timezone).time)
    {
      send_vpu_signal(vpu, SIGCONT);
      process_vpu_signal(vpu);
    }
    return -1;
  }
  return 0;
}

int all_proc_blocking(void)
{
  unsigned n = vpu_count;
  while(n--)
  {
    if(!vpus[n]->blocking && !vpus[n]->exiting && !vpus[n]->wait && !vpus[n]->zombie
        && !process_is_stopped(vpus[n]))
      return 0;
  }
  return 1;
}
