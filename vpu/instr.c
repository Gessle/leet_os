char *INSTR[] =
{
"wait",
"mov",
"jmp",
"inc",
"dec",
"cmp",
"je",
"push",
"pop",
"add",
"sub",
"dseg",
"mul",
"div",
"printuint",
"and",
"or",
"xor",
"inv",
"shl",
"shr",
"call",
"ret",
"int",
"iret",
"setint",
"jne",
"jg",
"jge",
"jle",
"jcz",
"jl",
"printstr",
"jfs",
"jfns",
"memcpy",
"in",
"out",
"memset",
"zflags",
"printsint",
"idiv",
"sconvw",
"sconvb",
"getstr",
"sys",
"scr",
"pstack",
"sstack",
"nop",
"test",
"memchr",
"memcmp",
"xchg",
"rol",
"ror",
"shcr",
"shcl",
"rcl",
"rcr",
"lmul",
"ldiv",
"adc",
"sbb"
};

unsigned char VINSTR[] =
{
OPCODE_WAIT,
OPCODE_RET,
OPCODE_IRET,
OPCODE_MEMCPY,
OPCODE_MEMSET,
OPCODE_ZFLAGS,
OPCODE_SYSCALL,
OPCODE_SCRCALL,
OPCODE_PSTACK,
OPCODE_SSTACK,
OPCODE_NOP,
OPCODE_MEMCHR,
OPCODE_MEMCMP,
0
};

unsigned char SBYTE_INSTR[] =
{
OPCODE_INC,
OPCODE_DEC,
OPCODE_PUSH,
OPCODE_POP,
OPCODE_PRINTUINT,
OPCODE_INV,
OPCODE_PRINTSTR,
OPCODE_PRINTSINT,
OPCODE_IN,
OPCODE_OUT,
OPCODE_DSEG,
0
};

unsigned char SWORD_INSTR[] =
{
OPCODE_JE,
OPCODE_INT,
OPCODE_JNE,
OPCODE_JG,
OPCODE_JGE,
OPCODE_JLE,
OPCODE_JCZ,
OPCODE_JL,
0
};

unsigned char DBYTE_INSTR[] =
{
OPCODE_ADD,
OPCODE_SUB,
OPCODE_ADC,
OPCODE_SBB,
OPCODE_MUL,
OPCODE_DIV,
OPCODE_AND,
OPCODE_OR,
OPCODE_XOR,
OPCODE_SHL,
OPCODE_SHR,
OPCODE_ROL,
OPCODE_ROR,
OPCODE_SHCL,
OPCODE_SHCR,
OPCODE_RCL,
OPCODE_RCR,
OPCODE_CMP,
OPCODE_IDIV,
OPCODE_SCONVW,
OPCODE_SCONVB,
OPCODE_GETSTR,
OPCODE_TEST,
OPCODE_XCHG,
0
};

unsigned char OPERIMM_INSTR[] =
{
OPCODE_ADD,
OPCODE_SUB,
OPCODE_ADC,
OPCODE_SBB,
OPCODE_MUL,
OPCODE_DIV,
OPCODE_AND,
OPCODE_OR,
OPCODE_XOR,
OPCODE_SHL,
OPCODE_SHR,
OPCODE_ROL,
OPCODE_ROR,
OPCODE_SHCL,
OPCODE_SHCR,
OPCODE_RCL,
OPCODE_RCR,
OPCODE_CMP,
OPCODE_IDIV,
OPCODE_TEST,
OPCODE_LMUL,
OPCODE_LDIV,
0
};

unsigned char DWORD_INSTR[] =
{
//OPCODE_JUMP,
//OPCODE_CALL,
OPCODE_JFS,
OPCODE_JFNS,
0
};

unsigned char TWORD_INSTR[] =
{
OPCODE_SETINT,
0
};

unsigned char TBYTE_INSTR[] =
{
OPCODE_LMUL,
OPCODE_LDIV,
0
};

unsigned char WREG_INSTR[] =
{
OPCODE_PUSH,
OPCODE_POP,
OPCODE_PRINTSTR,
OPCODE_GETSTR,
0
};

unsigned char BYTEINT_INSTR[] = 
{
OPCODE_IN,
OPCODE_OUT,
0
};

unsigned char JUMP_INSTR[] =
{
  OPCODE_CALL,
  OPCODE_JUMP,
  OPCODE_JFS,
  OPCODE_JFNS,
  OPCODE_JE,
  OPCODE_JNE,
  OPCODE_JG,
  OPCODE_JGE,
  OPCODE_JLE,
  OPCODE_JCZ,
  OPCODE_JL,
  0  
};

static unsigned char instr_in_array(unsigned char ch, unsigned char *arr)
{
  do
  {
    if(ch == *arr)
      return 1;
  }
  while(*(++arr));
  return 0;
}
