#include <stdio.h>
#include "serial.h"


#define ACK 6
#define NAK 21
#define SOH 1
#define EOT 4


int fserial;
FILE *fout;
char gotname;



void getpacket()
{
   char fname[256];
   int inp, i;
   char rec[128];
   char chk, blockno;


   blockno = sgetc(fserial);
   inp = sgetc(fserial);
   chk = 0;
   for (i = 0; i<128; i++)
   {
      rec[i] = sgetc(fserial);
      chk += rec[i];
   }

   inp = sgetc(fserial);
   if (inp != chk)
   {
      sputc(NAK, fserial);
      return;
   }

   if (blockno == 0 && !gotname)
   {
      gotname = 1;
      i = 0;
      while (rec[i] != 26 && rec[i] != 0)
      {
	 fname[i] = rec[i];
	 i++;
      }
      fname[i] = '\0';
      fout = fopen(fname, "w");
      printf("Opening %s, \n", fname);
   }
   else
      fwrite(rec, 128, 1, fout);

   sputc(ACK, fserial);
}



int main(void)
{
   int inp, retries;

   gotname = 0;
   fserial = sopen("/dev/ttyS0", B19200);

   retries = 5;


start:
   printf("\nPress a key when ready (x to exit).\n");
   if (getc(stdin) == 'x')
   {
      sclose(fserial);
      return 0;
   }

   printf("\nReceiving...\n");

   if (retries <= 0)
   {
      sclose(fserial);
      return 0;
   }

   tcflush(fserial, TCIFLUSH);
   sputc(NAK, fserial);

   sgetc(fserial);
   sgetc(fserial);

   while (1)
   {
      inp = sgetc(fserial);

      if (inp == EOT)
      {
	 sputc(ACK, fserial);
	 fclose(fout);
	 gotname = 0;
	 printf("Done.\n");
	 retries = 5;
	 sputc(ACK, fserial);  /* was NAK */
	 goto start;
      }

      getpacket();
   }
}

