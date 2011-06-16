#include <stdio.h>
#include <stdlib.h>


int main(void)
{
   FILE *fin, *fout;
   char buf[512];
   int i;


   fin = fopen("test1.img", "r");
   fout = fopen("test1good.img", "w");


   for (i=0; i<800; i++)
   {
      fread(buf, 512, 1, fin);
      fwrite(buf, 512, 1, fout);
      fseek(fin, 1, SEEK_CUR);
   }

   fclose(fin);
   fclose(fout);

   return 0;
}
