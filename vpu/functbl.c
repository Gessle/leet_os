static int vpu_instr_wait(struct vpu *vpu, unsigned flags);

int (*instruction_func_ptr[])() =
{
  vpu_instr_wait,
  vpu_instr_mov,
  vpu_instr_jump,
  vpu_instr_inc,
  vpu_instr_dec,
  vpu_instr_cmp,
  vpu_instr_je,
  vpu_instr_push,
  vpu_instr_pop,
  vpu_instr_add,
  vpu_instr_sub,
  vpu_instr_dseg,
  vpu_instr_mul,
  vpu_instr_div,
  vpu_instr_printuint,
  vpu_instr_and,
  vpu_instr_or,
  vpu_instr_xor,
  vpu_instr_inv,
  vpu_instr_shl,
  vpu_instr_shr,
  vpu_instr_call,
  vpu_instr_ret,
  vpu_instr_int,
  vpu_instr_iret,
  vpu_instr_setint,
  vpu_instr_jne,
  vpu_instr_jg,
  vpu_instr_jge,
  vpu_instr_jle,
  vpu_instr_jcz,
  vpu_instr_jl,
  vpu_instr_printstr,
  vpu_instr_jfs,
  vpu_instr_jfns,
  vpu_instr_memcpy,
  vpu_instr_in,
  vpu_instr_out,
  vpu_instr_memset,
  vpu_instr_zeroflags,
  vpu_instr_printsint,
  vpu_instr_idiv,
  vpu_instr_sconvw,
  vpu_instr_sconvb,
  vpu_instr_getstr,
  vpu_instr_sys,
  vpu_instr_scr,
  vpu_select_primary_stack,
  vpu_select_secondary_stack,
  vpu_instr_nop,
  vpu_instr_test,
  vpu_instr_memchr,
  vpu_instr_memcmp,
  vpu_instr_xchg,
  vpu_instr_rol,
  vpu_instr_ror,
  vpu_instr_shcr,
  vpu_instr_shcl,
  vpu_instr_rcl,
  vpu_instr_rcr,
  vpu_instr_mul32,
  vpu_instr_div32,
  vpu_instr_adc,
  vpu_instr_sbb
};


















