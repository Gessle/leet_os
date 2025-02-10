static int con_stdout_write(unsigned char c);

static int vpu_instr_printstr(struct vpu *vpu, unsigned flags)
{
  unsigned short data_pointer = vpu->regs[vpu_next_code_byte(vpu) & 0x07];

  if(debug)
  {
    sprintf(message, "String is in address 0x%.2X", data_pointer);
    output_debug_info(message);
  }

  if(!memory_allowed(vpu, data_pointer))
  {
    if(debug)
    {
      sprintf(message, "  Pointer value is too big!");
      output_debug_info(message);
    }
    return 4;
  }

  while(memory_allowed(vpu, data_pointer) && vpu->data[vpu->data_segment][data_pointer])
  {
    con_stdout_write(vpu->data[vpu->data_segment][data_pointer++]);
  }
  con_stdout_write(10);

  return 3;
}


