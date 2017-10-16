#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#ifdef LINUX
#include <unistd.h>
#endif

char *digit_string(int num, int counter)
{
   char *result = (char *)malloc(num+1);
   char *str_pad = (char *)malloc(num);

   int bigNum = 1;
   int k = 0;
   for (int i = 0 ; i < num-1; i++)
   {
      bigNum *= 10;
      
      if (counter / bigNum == 0)
         str_pad[k++] = '0';

   }
   str_pad[k] = 0;
   
   sprintf(result, "%s%d", str_pad, counter);
   free(str_pad);
   return result;
   
}

bool file_exist(char *fn)
{
   FILE *fd = fopen(fn, "r");

   if (fd == NULL)
   {
      return false;
   } else {
      fclose(fd);
      return true;
   }
   
}

int main()
{
   char fn1[1000];
   char fn2[1000];
   char fn3[1000];

   char *s_str = "./result/point_cloud_images/slices_left_right_600_1024_result/";
   for (int i = 0; i < 1000; i ++)
   {
      char *ds = digit_string(3, i);
      sprintf(fn1, "%saaa_image_slice_1024_392_0%s.tif", s_str, ds);

      if (!file_exist(fn1))
         continue;

      sprintf(fn2, "%sbbb_image_slice_1024_392_0%s.tif", s_str, ds);

      char cmd[1000];
      //sprintf(cmd,"project -B %s -o %s -O \"-r 512 -p 1 -R -t 6 -H\"", fn1, fn2);
      sprintf(cmd,"project -B %s -o %s -O \"-r 256 -p 0.5 -R\"", fn1, fn2);
      printf("%s\n", cmd);
      system(cmd);

      
   }
}

int myproject(int argc, char**argv)
{
   int i = 0;
   if (argc > 1)
   {
      i = atoi(argv[1]);
   }

   printf("i is %d\n", i);
   for (i; i < 12; i ++)
   {
      
      FILE *fd = fopen("./log.txt", "a+");
      assert(fd);
      fprintf(fd, "%d\n", i);
      fflush(fd);
      fclose(fd);

      printf("Processing %d images\n", i);
#ifdef LINUX
      sleep(1);
#endif      

      assert(i != 10 && i != 20 && i != 30);
   }

   return 0;
}

int main1()
{
   int end_no = 999;
   int start_no = 0;
   while (1)
   {
      // construct the command
      char cmd[1000];
      sprintf(cmd,"./project -S -O \"-s %d -e %d\"", start_no, end_no);
      //sprintf(cmd,"./myproject %d", start_no);

      printf("command name is [%s]\n", cmd);
      int ret = system(cmd);
      printf("system command return %d.\n", ret);

      if (ret == 0)
         break;
      
      perror("system:");

      // check the log file
      FILE *fd = fopen("./log.txt", "r");
      char strno[10];
      int no;
      while (fgets(strno, 10, fd)) {
	if (sscanf(strno, "%d", &no) == 1 ) {
      	   printf("No is %d\n", no);
	}
      }
      if (no >= end_no)
         break;
      else
         start_no = no + 1;
	   	
   }

   return 0;
}

#if 0 // This is for Z3 batch run

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <atltime.h>

int main(int argc, char **argv)
{
   char fn1[1000];
   char fn2[1000];
   char cmd[1000];
   time_t ot, ct;

   if (argc < 2)
      exit(0);

   int start = atoi(argv[1]);
   
   //char *str_p1[] = {"1891", "21", "273", "7747", "91"};
   //char *str_p2[] = {"61", "3", "13", "127", "13"};
   char *str_p1[] = {"1891", "21", "273", "91"};
   char *str_p2[] = {"1830", "14", "260", "78"};


   /*
   //for (int i = 0; i < 5; i ++)
   for (int i = 2; i < 4; i ++)
   {
      time(&ot);

      //sprintf(cmd,"e.cmd 1 %s %s", str_p1[i], str_p2[i]);
      sprintf(cmd,"e.cmd 2 %s %s", str_p1[i], str_p2[i]);
      printf("%s\n", cmd);
      system(cmd);
      
      time(&ct);
      printf("\nTotal time is :%d\n", ct-ot);
   }
   */

   /*
   for (int i = 3; i < 10; i ++)
   {
      time(&ot);

      sprintf(cmd,"e.cmd %d 0 0", i);
      printf("%s\n", cmd);
      system(cmd);
      
      time(&ct);
      printf("\nTotal time is :%d\n", ct-ot);
   }

   */
   printf("\n ====== Now the 10th example ============\n");
   //for p in {127759, 2081819, 240157, 273, 7747, 8463}
   char *str_p[] = {"127759", "2081819", "240157", "273", "7747", "8463"};
   for (int i = start; i < 6; i ++)
   {
      time(&ot);

      sprintf(cmd,"e.cmd 10 %s 0", str_p[i]);
      printf("%s\n", cmd);
      system(cmd);
      
      time(&ct);
      printf("\nTotal time is :%d\n", ct-ot);
   }
   
}
#endif
