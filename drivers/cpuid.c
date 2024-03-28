#include <stdlib.h>

void CPUID(void);
#pragma aux CPUID = \
  "cpuid";

int main()
{
  CPUID();
}
