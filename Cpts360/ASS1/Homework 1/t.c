/********** test.c file *****************/
/* Original Source Code from KC Wang
 * Modified by Austin Schorsch
 * 
 * Cpts 360 - Fall 2015
 */ 


#include <stdio.h>
#include <stdlib.h>

int *FP;

main(int argc, char *argv[ ], char *env[ ])
{
  int a,b,c;
  printf("enter main\n");
  
  printf("\n**************************************** Printing contents\n");   
  printf("&argc=%x argv=%x env=%x\n", &argc, argv, env);
  printf("&a=%8x &b=%8x &c=%8x\n", &a, &b, &c);

  a=1; b=2; c=3;
  A(a,b);

  printf("exit main\n");
}

int A(int x, int y)
{
  int d,e,f;
  printf("enter A\n");
  
  // printing the addresses of d, e, f
  printf("&d=%8x &e=%8x &f=%8x\n", &d, &e, &f);

  d=4; e=5; f=6;
  B(d,e);
 
  printf("exit A\n");
}

int B(int x, int y)
{
  int g,h,i;
  printf("enter B\n");
  
  // printing the addresses of g, h, i
  printf("&g=%8x &h=%8x &i=%8x\n", &g, &h, &i);

  g=7; h=8; i=9;
  C(g,h);

  printf("exit B\n");
}

int C(int x, int y)
{
  int u, v, w, i, *p;

  printf("enter C\n");

  // printing the addresses of u, v, w
  printf("&u=%8x &v=%8x &w=%8x\n", &u, &v, &w);

  u=10; v=11; w=12;

  // This line below is setting FP to CPU's ebp register
  asm("movl %ebp, FP"); 
  printf("\n**************************************** Printing stack frame link list\n");
  printf("FP=%8x-->", FP);

  // Loop to print contents of stack frame
  p=FP;
  // while(p!=NULL)  
  do {
    printf("%x",*p);
    if(*p!=0)
    {
      printf("-->");
    }
    p=(*p);  
  } while(p!=NULL);

  printf("\n\n**************************************** Printing the stack contents (from FP-8)\n");

  // reset p to point at the frame pointer
  p=FP-8;
  for(i=0;i<100;i++)
  {
    printf("%d.     %8x,    Contents: %8x\n",i+1,p,*p,*p);
    p++;
  }
  // Note: could display *p (see above) as a hex value 
  printf("exit C\n"); 
}
