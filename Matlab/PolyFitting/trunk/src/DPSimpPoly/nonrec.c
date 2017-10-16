/*  				1-26-94      Jack Snoeyink
                    Non-recursive implementation of the Douglas Peucker line simplification
                    algorithm.
*/
#include <assert.h>
#include "DP.h"
#include "animate.h"

/* Assumes that the polygonal line is in a global array V. 
 * main() assumes also that a global variable n contains the number of
 * points in V.
 */

int stack[MAX_POINTS];		/* recursion stack */
int sp;				/* recursion stack pointer */
int write_last_point = 0;

#define Stack_Push(e)		/* push element onto stack */\
  stack[++sp] = e
#define Stack_Pop()		/* pop element from stack (zero if none) */\
  stack[sp--]
#define Stack_Top()		/* top element on stack  */\
  stack[sp]
#define Stack_EmptyQ()		/* Is stack empty? */\
  (sp < 0)
#define Stack_Init()		/* initialize stack */\
  sp = -1


void Find_Split(int i, int j,  int *split, double *dist) /* linear search for farthest point */
{
   int k;
   HOMOG q;
   double tmp;

   *dist = -1;
   if (i + 1 < j)
   {
      CROSSPROD_2CCH(V[i], V[j], q); /* out of loop portion */ 
      /* of distance computation */
      for (k = i + 1; k < j; k++)
      {
         tmp = DOTPROD_2CH(V[k], q); /* distance computation */
         if (tmp < 0) tmp = - tmp; /* calling fabs() slows us down */
         if (tmp > *dist) 
         {
            *dist = tmp;	/* record the maximum */
            *split = k;
         }
      }
      *dist *= *dist/(q[XX]*q[XX] + q[YY]*q[YY]); /* correction for segment */
   }				   /* length---should be redone if can == 0 */
}



void DPbasic(int i,int j)		/* Basic DP line simplification */
{
   int split; 
   double dist_sq;
  
   Stack_Init();
   Stack_Push(j);
   do
   {
      Find_Split(i, Stack_Top(), &split, &dist_sq);
      if (dist_sq > EPSILON_SQ)
      {
         Stack_Push(split);
      }
      else
      {
         Output(i, Stack_Top()); /* output segment Vi to Vtop */
         i = Stack_Pop();
      }
   }
   while (!Stack_EmptyQ());
}

void set_epsilon(char *value)
{
   //   printf(" original epsilon:%f, ", EPSILON);
   EPSILON = atof(value);
   EPSILON_SQ = EPSILON*EPSILON;
   //   printf("updated to:%f\n", EPSILON);
}

void my_nonrec_DP(char *fn)
{
   char str_line[100];
   FILE *fd = fopen(fn, "r");
   assert(fd);

   int num = 0;
   int x, y;

   Init("DPfast");   
   while (fgets(str_line, 1000, fd))
   {
      sscanf(str_line,"%d %d",&x,&y);
      //printf("X:%d, Y:%d\n", x, y);
      V[num][XX] = (double) x;
      V[num][YY] = (double) y;
      num++;
   }
   n = num;

   // check if this is a closed-form polygon
   if (V[0][XX] == V[num-1][XX] && V[0][YY] == V[num-1][YY] )
   {
      n --;
      write_last_point = 1;
   }
   
   // do the DP algorithm
   for (int i=0; i < 100; i++)	
   {
      outFlag = TRUE;
      num_result = 0;
      DPbasic(0, n - 1);
   }

   Print_Result(TRUE);

   fclose(fd);
}


int main_nonrec(int argc, char **argv)
{
   register int i;

   Parse(argc, argv);
   Init("DPfast");

   do
   {
      Get_Points();

      for (i=0; i < 100; i++)	
      {
         outFlag = TRUE;
         num_result = 0;
         DPbasic(0, n - 1);
      }

      Print_Result(TRUE);
   }
   while (looping);

   return 0;
}

