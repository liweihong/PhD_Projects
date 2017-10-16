#include <stdio.h>
#include <stdlib.h>

void main()
{
   FILE *fd = fopen("test.dat", "w+");

   int n = 8;
   
   float value_in[] = {
                    10.0, 10.0,
                    10.0, 20.0,                    
                    10.0, 40.0,
                    20.0, 40.0,                    
                    40.0, 40.0,
                    40.0, 30.0,
                    40.0, 20.0,                    
                    40.0, 10.0 };

   fwrite(&n, sizeof(int), 1, fd);


   fwrite(value_in, sizeof(float), 16, fd);
}
