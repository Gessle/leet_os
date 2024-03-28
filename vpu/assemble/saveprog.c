/*
  Program executable file format:
  Length of header as unsigned char
  Header
  Length of data as unsigned long
  Data
  Length of code as unsigned long
  Code
*/

static int save_program(char *filename, struct return_string *header, unsigned char *code)
{
  unsigned n;
  FILE *fp;
  unsigned char headerbyte;

  fp = fopen(filename, "wb");
  if(!fp) return 1;

  fputc(header->length, fp);

  for(n=0;n<header->length;n++)
    fputc(header->str[n], fp);

  if(data_auto_len)
  {
    datap_offset++;
    headerbyte = 0;
    // data
    headerbyte |= 0x20;
    // data segment count
    if(datap_segment >= 0xFF)
      headerbyte |= 0x08;
    else
      headerbyte |= 0x04;
    // data offset
    if(datap_offset >= 0xFF)
      headerbyte |= 0x02;
    else
      headerbyte |= 0x01;
    fputc(headerbyte, fp);
    if(datap_segment >= 0xFF)
    {
      header->length+=3;
      fputc(datap_segment, fp);      
      fputc(datap_segment>>8, fp);     
    }
    else
    {
      header->length+=2;
      fputc(datap_segment, fp);
    }
    if(datap_offset >= 0xFF)
    {
      header->length+=2;
      fputc(datap_offset, fp);      
      fputc(datap_offset>>8, fp);      
    }
    else
    {
      fputc(datap_offset, fp);
      header->length++;
    }
    rewind(fp);
    fputc(header->length, fp);
    fseek(fp, header->length+1, SEEK_SET);
  }

  if(code_auto_len)
  {
    codep_offset++;
    headerbyte = 0;
    // code
    headerbyte |= 0x40;
    // code segment count
    if(codep_segment >= 0xFF)
      headerbyte |= 0x08;
    else
      headerbyte |= 0x04;
    // code offset
    if(codep_offset >= 0xFF)
      headerbyte |= 0x02;
    else
      headerbyte |= 0x01;
    fputc(headerbyte, fp);
    if(codep_segment >= 0xFF)
    {
      header->length+=3;
      fputc(codep_segment, fp);      
      fputc(codep_segment>>8, fp);     
    }
    else
    {
      header->length+=2;
      fputc(codep_segment, fp);
    }
    if(codep_offset >= 0xFF)
    {
      header->length+=2;
      fputc(codep_offset, fp);      
      fputc(codep_offset>>8, fp);      
    }
    else
    {
      fputc(codep_offset, fp);
      header->length++;
    }
    rewind(fp);
    fputc(header->length, fp);
    fseek(fp, header->length+1, SEEK_SET);
  }

  putstr("Header ready...");

  fputc(datap_offset, fp);
  fputc(datap_offset>>8, fp);

  putstr("Data segment length info ready...");

  for(n=0;n<datap_offset;n++)
    fputc(data[0][n], fp);

  putstr("Data segment ready...");

  fputc(codep_offset, fp);
  fputc(codep_offset>>8, fp);

  putstr("Code segment length info ready...");

  for(n=0;n<codep_offset;n++)
    fputc(code[n], fp);

  putstr("Code segment ready...");

  fclose(fp);

  putstr("Success!");

  return 0;
}
