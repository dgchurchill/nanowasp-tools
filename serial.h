
#ifndef SERIAL_H
#define SERIAL_H

#include <termios.h>


unsigned char sgetc(int f);
void sputc(unsigned char c, int f);
int sopen(const char *name, speed_t speed);
void sclose(int f);

#endif /* SERIAL_H */

