#include <stdio.h>
#include <stdlib.h>
#include "../serial.h"


int main(void)
{
   FILE *fout;
   int i, fserial;


   fout = fopen("rom56.bin", "w");
   if (fout == NULL)
   {
      fprintf(stderr, "can't open rom56.bin for writing\n");
      exit(1);
   }
   fserial = sopen("/dev/ttyS0", B19200);
   if (fserial == -1)
   {
      fprintf(stderr, "can't open serial port\n");
      exit(1);
   }

   for (i=0; i<0x1000; i++)
   {
      fputc(sgetc(fserial), fout);
      printf(".");
   }

   fclose(fout);
   sclose(fserial);

   return 0;
}

