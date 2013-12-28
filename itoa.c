#include "itoa.h"
 
#define ITOABUF_SIZE 20
static char buffer[ITOABUF_SIZE]; 
 
char* itoa(int i)
{
   char* ptr = buffer+ITOABUF_SIZE-1;
   char sign = 0;
	 int j = 0;
	
	 for(j=0; j<ITOABUF_SIZE - 1; j++)
		buffer[j] = 0;
	
   *ptr-- = 0;
   if (i < 0)
   {
      sign = 1;
      i = -i;
   }
   do
   {
      *ptr-- = 48 + (i % 10);
      i /= 10;
   }
   while (i > 0);
   if (sign == 1) *ptr-- = '-';
   return ++ptr;
}
