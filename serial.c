#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include "serial.h"


unsigned char sgetc(int f)
{
   char ret;

   read(f, &ret, 1);
   return ret;
}


void sputc(unsigned char c, int f)
{
   write(f, &c, 1);
}


int sopen(const char *name, speed_t speed)
{
   struct termios tios;
   int f;

   f = open(name, O_RDWR);
   tcgetattr(f, &tios);
   cfmakeraw(&tios);
   cfsetispeed(&tios, speed);
   cfsetospeed(&tios, speed);
   tcsetattr(f, TCSANOW, &tios);

   return f;
}


void sclose(int f)
{
   close(f);
}

