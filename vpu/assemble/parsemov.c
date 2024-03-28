const char *siregs[] =
{
  "cs", "ip", "ds", "sp"
};

static unsigned char nongpreg(char *reg)
{
  unsigned char nongpregs = sizeof(siregs)/sizeof(const char*);
  while(nongpregs--)
  {
    if(!strncmp(reg, siregs[nongpregs], 2))
      return 8|nongpregs;
  }
  return 0;
}

static void parsemov(char *instr, struct return_string *retval)
{
  unsigned char *infobyte = retval->str+1;
  unsigned char operand = 0;
  unsigned char *retvalp = retval->str+2;
  int number;
  signed char *sretvalbp;
  signed short *sretvalwp;
  unsigned short *retvalwp;
//  unsigned char nongpregs = sizeof(siregs)/sizeof(const char*);

  while(operand < 2)
  {
    // operand is a register
//    if(*instr >= 'a' && *instr <= 'a' + VPU_REGISTER_COUNT)
    if(*instr >= 'a' && *instr <= 'z')
    {
      *infobyte |= define_register_type(operand, instr);
      if(define_register_type(operand, instr))
      {
/*        while(nongpregs--)
          if(!strncmp(instr, siregs[nongpregs], 2))
          {
            *retvalp++ = 8|nongpregs;
            goto while_end;
          }*/
        if(*retvalp = nongpreg(instr))
        {
          retvalp++;
          goto while_end;
        }
        *retvalp++ = *instr - 'a';
      }
      else
      {
        *retvalp = (*instr - 'a') * 2;
        if(instr[1] == 'h') ++*retvalp;
        retvalp++;
      }
    }

    // operand is a pointer in register
    else if(*instr >= '*' && *instr <= ',')
    {
      if(*instr == '*')
        *infobyte |= 0x09 << (operand<<2);
      else if(*instr == '+')
        *infobyte |= 0x0A << (operand<<2);
      else
        *infobyte |= 0x0B << (operand<<2);
      instr++;
      if(*retvalp = nongpreg(instr))
        retvalp++;
      else
        *retvalp++ = *instr - 'a';
    }

    // operand is a symbol's address as an immediate number
    else if(*instr == '&')
    {
      *infobyte |= 0x0D << (operand<<2);
      *(unsigned short*)retvalp = get_symbol_addr(&instr[1]);
      retvalp+=2;
    }
    
    // operand is a symbol address
    else if(*instr == '$')
    {
      *infobyte |= define_symbol_type(operand, &instr[1]) & 0x77;
      // if const str...
      if(operand && !(*infobyte & 0x70))
        *infobyte |= 0xD0;
      *(unsigned short*)retvalp = get_symbol_addr(&instr[1]);
      retvalp+=2;
    }

    // operand is a symbol segment
    else if(*instr == '.')
    {
      *infobyte |= 0x0D << (operand<<2);
      *(unsigned short*)retvalp = get_symbol_seg(&instr[1]);
      retvalp+=2;
    }    

    // operand is a number
    else
    {
      *infobyte |= 0x0C << (operand<<2);
      number = getwordvalue(&instr);
      if(number < 0)
      {
        if(*infobyte & 0x04 || (signed char)number != number)
        {
          *infobyte |= 0x01 << (operand<<2);
          sretvalwp = (signed short*)retvalp;
          *sretvalwp = number;
          retvalp += 2;    
        }  
        else
        {
          sretvalbp = (signed char*)retvalp++;
          *sretvalbp = number;
        }        
      }
      else if(number > 0xFF)
      {
        *infobyte |= 0x01 << (operand<<2);
        retvalwp = (unsigned short*)retvalp;
        *retvalwp = number;
        retvalp += 2;
      }
      else
      {
        *retvalp++ = number;
      }
    }
    while_end:

    instr = skip_word(instr);
    instr = skip_whitespaces(instr);

//    while(*instr != ' ' && *instr != '\n')
//      instr++;
    operand++;
  }
  *retvalp = 0;
  retval->length = retvalp - retval->str;

}

