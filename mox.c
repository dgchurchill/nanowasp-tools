/* bugs: this is a crufty hack.
         don't use pathnames -- the file should be in the current directory. */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "serial.h"

#define ACK 6
#define NAK 21
#define SOH 1
#define EOT 4
#define CAN 24
#define CTRLZ 26



FILE *fin;
char *fname;
char blockno;
int fserial;



int sendpacket(void)
{
   char rec[128];
   char chk, inp;
   int resend, i;


   memset(rec, CTRLZ, 128);
   fread(rec, 1, 128, fin);

   resend = 10;
   while (resend > 0)
   {
      sputc(SOH, fserial);
      sputc(blockno, fserial);
      sputc(255-blockno, fserial);

      chk = 0;
      for (i=0; i<128; i++)
      {
	 sputc(rec[i], fserial);
	 chk += rec[i];
      }

      sputc(chk, fserial);

      inp = sgetc(fserial);
      if (inp == ACK)
	 resend = -1;
      else if (inp == CAN)
      {
	 printf("\nCancel received, aborting.\n");
	 sclose(fserial);
	 fclose(fin);
	 exit(1);
      }
      else
	 resend--;
   }


   if (resend == 0)
   {
      printf("Block %i not sent after ten retries\n", blockno);
      sclose(fserial);
      fclose(fin);
      exit(1);
   }

   blockno++;

   if (feof(fin))
      return 0;
   else
      return 1;
}



void sendfname(void)
{
   char name[256], *dot;
   int i, dotpos, len;
   char chk, inp;


   dot = strchr(fname, '.');
   len = strlen(fname);

   if (dot != NULL)
   {
      dotpos = dot - fname;
      for (i=0; i<8; i++)
	 if (i<dotpos && i<len)
	    name[i] = toupper(fname[i]);
	 else
	    name[i] = ' ';
      for (i=0; i<3; i++)
	 if (dotpos+1+i<len)
	    name[8+i] = toupper(fname[dotpos+1+i]);
	 else
	    name[8+i] = ' ';
   }
   else
   {
      for (i=0; i<8; i++)
	 if (i<len)
	    name[i] = toupper(fname[i]);
	 else
	    name[i] = ' ';
      for (i=0; i<3; i++)
	 name[8+i] = ' ';
   }


   chk = 0;
   for (i=0; i<11; i++)
   {
      sputc(name[i], fserial);
      chk += name[i];
      inp = sgetc(fserial);
      if (inp != ACK)
      {
	 printf("No ACK on filename!\n");
	 sclose(fserial);
	 exit(1);
      }
   }

   sputc(CTRLZ, fserial);
   chk += CTRLZ;
   inp = sgetc(fserial);

   if (inp != chk)
   {
      printf("Filename checksum error!");
      sclose(fserial);
      exit(1);
   }

   sputc(ACK, fserial);
}




int main(int argc, char *argv[])
{
   char inp;


   if (argc != 2)
   {
      printf("usage: %s FILENAME\n", argv[0]);
      exit(1);
   }

   fname = argv[1];
   fin = fopen(fname, "r");
   if (fin == NULL)
   {
      printf("Unable to open file %s\n", fname);
      exit(1);
   }

   fserial = sopen("/dev/ttyS0", B9600);

   printf("Sending %s...\n", fname);

   inp = sgetc(fserial);
   if (inp != NAK)
   {
      printf("NAK wasn't received!\n");
      fclose(fin);
      sclose(fserial);
      exit(1);
   }

   sputc(ACK, fserial);

   sendfname();
   blockno = 1;
   while (sendpacket())
      ;


   sputc(EOT, fserial);
   printf("EOT\n");

   if (sgetc(fserial) != ACK)
      printf("Warning: final ACK not received.\n");

   fclose(fin);
   sclose(fserial);

   return 0;
}

