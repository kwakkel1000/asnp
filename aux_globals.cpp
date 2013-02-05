#include "aux_globals.h"


void delay_int(unsigned long delay)
{
        while(delay--) asm volatile("nop");
};

// needed for new and delete
void * operator new(size_t size)
{
  return malloc(size);
}
void operator delete(void * ptr)
{
  free(ptr);
}

