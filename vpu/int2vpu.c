struct vpu_hw_int *vpu_hardware_int_table = 0;
unsigned vpu_hwint_count = 0;
void __interrupt __far dummy_interrupt_mask_handler();
void __far dummy_interrupt_mask_handler_end();

void __interrupt __far dummy_interrupt_mask_handler_cont();
void __far dummy_interrupt_mask_handler_cont_end();

void reset_vpu_interrupt_handlers(void)
{
  unsigned n = vpu_hwint_count;

  while(n--)
    _dos_setvect(vpu_hardware_int_table[n].hw_int, vpu_hardware_int_table[n].prev_int);
}

void set_vpu_interrupt_handlers(void)
{
  unsigned n = vpu_hwint_count;

  while(n--)
    _dos_setvect(vpu_hardware_int_table[n].hw_int, vpu_hardware_int_table[n].handler);
}

inline void *new_vpu_interrupt_mask_handler(unsigned char hw_int, void (__far __interrupt **old_handler)(), unsigned chain)
{
  const void __far *dummy_handler;
  unsigned dummy_handler_len;
  unsigned char *retval;

  if((*old_handler = _dos_getvect(hw_int)) && chain)
  {
    dummy_handler = dummy_interrupt_mask_handler_cont;
    dummy_handler_len = (unsigned long)dummy_interrupt_mask_handler_cont_end - (unsigned long)dummy_handler;
    retval = malloc(dummy_handler_len);    
  }
  else
  {
    dummy_handler = dummy_interrupt_mask_handler;
    dummy_handler_len = (unsigned long)dummy_interrupt_mask_handler_end - (unsigned long)dummy_handler;    
    retval = malloc(dummy_handler_len);    
  }
  if(!retval) return 0;

  memcpy(retval, dummy_handler, dummy_handler_len);
  
  // replace MOV's source operand with interrupt number
  retval[2] = hw_int;

  return retval;
}

static int set_vpu_hw_int(struct vpu *vpu, unsigned char hw_int, unsigned short vpu_int, unsigned chain)
{
  void __far *new_pointer;
  unsigned long __far *func_ptr;
  void (__far __interrupt *old_handler)();
  unsigned n = vpu_hwint_count;

  while(n--)
  {
    if(vpu_hardware_int_table[n].vpu == vpu && vpu_hardware_int_table[n].hw_int == hw_int)
      return 0;
  }
  
  if(!vpu_hwint_count++)
  {
    vpu_hardware_int_table = calloc(vpu_hwint_count, sizeof(struct vpu_hw_int));
    if(!vpu_hardware_int_table)
    {
      nomemory:
      --vpu_hwint_count;
      return 0;
    }
  }
  else
  {
    new_pointer = realloc(vpu_hardware_int_table, vpu_hwint_count * sizeof(struct vpu_hw_int));
    if(!new_pointer) goto nomemory;
    vpu_hardware_int_table = new_pointer;
  }

  if(!(new_pointer = new_vpu_interrupt_mask_handler(hw_int, &old_handler, chain)))
  {
    vpu_hardware_int_table = realloc(vpu_hardware_int_table, vpu_hwint_count-1 * sizeof(struct vpu_hw_int));
    goto nomemory;
  }

  vpu_hardware_int_table[vpu_hwint_count-1].hw_int = hw_int;
  vpu_hardware_int_table[vpu_hwint_count-1].vpu = vpu;
  vpu_hardware_int_table[vpu_hwint_count-1].vpu_int = vpu_int;
  vpu_hardware_int_table[vpu_hwint_count-1].handler = new_pointer;  
  if((vpu_hardware_int_table[vpu_hwint_count-1].prev_int = old_handler) && chain)
  {
    func_ptr = (unsigned long*)&((unsigned char*)new_pointer)[12]; // Set JMP destination to the previous interrupt handler
    *func_ptr = (unsigned long)vpu_hardware_int_table[vpu_hwint_count-1].prev_int;
  }
  _dos_setvect(hw_int, new_pointer);

  return 1;
}

static void remove_vpu_hw_int(struct vpu *vpu, unsigned int vpu_int)
{
  unsigned n = vpu_hwint_count;
//  void *func_ptr;

  while(n--)
    if(vpu_hardware_int_table[n].vpu == vpu && vpu_hardware_int_table[n].vpu_int == vpu_int)
      break;

  free(vpu_hardware_int_table[n].handler);
  
  _dos_setvect(vpu_hardware_int_table[n].hw_int, vpu_hardware_int_table[n].prev_int);

  for(;n<vpu_hwint_count-1;n++)
    vpu_hardware_int_table[n] = vpu_hardware_int_table[n+1];

  if(!(vpu_hwint_count = n))
    free(vpu_hardware_int_table);

  else
    vpu_hardware_int_table = realloc(vpu_hardware_int_table, n* sizeof(struct vpu_hw_int));
}
