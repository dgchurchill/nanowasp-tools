#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../serial.h"


#define CMD_STOP       0
#define CMD_SETDRIVE   1
#define CMD_SEEKTR0    2
#define CMD_SEEK       3
#define CMD_DUMPTRACK  4
#define CMD_READSECTOR 5



void sendcmd(unsigned char cmd, int s)
{
   usleep(100);
   sputc('D', s);
   sputc('C', s);
   sputc(cmd, s);
}



int setdrive(unsigned char drive, unsigned char side, int dbldense, int s)
{
   unsigned char arg;

   arg = drive & 0x03;
   arg += (side & 0x01) << 2;
   if (dbldense)
      arg += 8;

   sendcmd(CMD_SETDRIVE, s);
   sputc(arg, s);

   return sgetc(s) == 0;
}



int seektr0(int s)
{
   sendcmd(CMD_SEEKTR0, s);

   return (sgetc(s) & 0x04) != 0;
}



int seektrack(unsigned char track, int s)
{
   sendcmd(CMD_SEEK, s);
   sputc(track, s);

   sgetc(s);
   return 1;
}


int readsector(unsigned char sector, FILE *f, int s)
{
   unsigned char result;
   unsigned char buf[10000];
   unsigned int chksum, chksumsent, len;
   int i;


   sendcmd(CMD_READSECTOR, s);
   sputc(sector, s);
   result = sgetc(s);
   
   chksum = 0;

   len = sgetc(s);
   len += sgetc(s)*256;

   for (i=0; i<len; i++)
   {
      buf[i] = sgetc(s);
      chksum += buf[i];
   }

   chksumsent = sgetc(s);
   chksumsent += sgetc(s)*256;

   chksum &= 0xFFFF;
   chksumsent &= 0xFFFF;

   if (((result & 0x1C) == 0) && (chksum == chksumsent))
   {
      fwrite(buf, len, 1, f);
      return 1;
   }
   else
   {
      fprintf(stderr, "readsector: read failed - result %02Xh, "
		      "calculated checksum %04Xh, received checksum %04Xh.\n",
	      result, chksum, chksumsent);
      return 0;
   }
}




int main(int argc, char *argv[])
{
   FILE *f;
   unsigned char side, track, sect;
   int i, fserial;


   if (argc != 2 && argc != 5)
   {
      fprintf(stderr, "usage: %s FILE [SIDE TRACK SECTOR]\n", argv[0]);
      exit(1);
   }

   fserial = sopen("/dev/ttyS0", B19200);
   if (fserial == -1)
   {
      fprintf(stderr, "error opening serial port\n");
      exit(1);
   }

   f = fopen(argv[1], "w");
   if (f == NULL)
   {
      fprintf(stderr, "error opening %s for writing\n", argv[1]);
      exit(1);
   }


   side = track = 0;
   sect = 1;

   if (argc == 5)
   {
      side = atoi(argv[2]);
      track = atoi(argv[3]);
      sect = atoi(argv[4]);

      if (side > 1)
	 side = 1;
      if (track > 39)
	 track = 39;
      if (sect > 10)
	 sect = 10;
   }


   while (side < 2)
   {
/*      sleep(1); */
      setdrive(1, side, 1, fserial);
      seektr0(fserial);
      while (track < 40)
      {
	 seektrack(track, fserial);
	 while (sect < 11)
	 {
	    printf("S:%2i T:%3i S:%3i\n", side, track, sect);
	    i = 0;
	    while (i < 3)
	    {
	       if (readsector(sect, f, fserial))
		  break;
	       i++;
	    }
	    if (i >= 3)
	    {
	       fprintf(stderr, "\nreadsector failed\n");
	       exit(1);
	    }
	    sect++;
	 }
	 sect = 1;
	 track++;
      }
      track = 0;
      side++;
   }

   fclose(f);

   setdrive(0, 0, 1, fserial);
   sendcmd(CMD_STOP, fserial);

   sclose(fserial);

   return 0;
}

