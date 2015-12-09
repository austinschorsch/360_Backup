/** 
  * Austin Schorsch - Lab 1
  * t.c file
  * Date: 9/7/15
  */

typedef unsigned int u32; 

//#define BASE 10
int BASE = 10; 

char *table = "0123456789ABCDEF"; 

int rpu(u32 x)
{
    char c;
    if(x)
    {
        c = table[x % BASE];
        rpu(x / BASE); 
        putchar(c); 
    }
}

int printu(u32 x)
{
    BASE = 10;
    if(x==0)
        putchar('0'); 
    else
        rpu(x); 
    putchar(' '); 
}

/**
 * prints function - prints a string using putchar while checking for \n character
 */ 
int prints(char *s)
{
    // Update the base
    BASE = 10;
    while(*s)
    {
        if(*s!='\n')
        {
            putchar(*s);
            s++;
        }
        else
        {
            putchar('\n');
            s++;
        }
    }
}

/**
  * printd function - print an integer that can be negative
  */ 
int printd(int x)
{
    // Update the base
    BASE = 10;
    // Check if our value is less than 0
    if(x < 0)
    {
        // Place the negative value, then swap the value to positive
        putchar('-');
        x=x*(-1); 
    }
    else if(x==0)
        putchar('0');
    rpu(x); 
}

/** 
  * printo function - print x in octal
  */
int printo(u32 x) 
{
    // First, change the BASE to octal, so 8.
    // Once completed, simply printu(x); 
    BASE = 8;
    putchar('0');
    rpu(x);
}

/**
  * printx function - print x in hex
  */ 
int printx(u32 x)
{
    // For hex, we need a BASE of 16
    // After that, place the '0x', then call printu(x)
    BASE = 16; 
    putchar('0'); 
    putchar('x'); 
    rpu(x); 
}
    
/**
  * myprintf() function - uses all the print functions created to 
  * make a functional printf function
  */ 
int myprintf(char *fmt, ...) 
{
    // First, initialize the *cp and the *ip
    char *cp=fmt; // points at the format string (* -> value, ~* -> address)
    int *ip; // points at first item on the stack, think of ip as the contents after the , 

    // Since the cp points at the format string, point it at fmt
    // Also, set the *ip to the ebp, then move it over 12
    // Note: converting integer type to int * type
    ip = (int *)(get_ebp() + 12); // Move over to our first value (i.e. 'a'); this is moving from the ebp, past PC (4), past fmt (8)  
    while(*cp)
    {
        if(*cp=='%')
        {
            // Increment one spot over, check to see what character follows
            cp++; 
            if(*cp=='c')
                putchar(*ip);
            else if(*cp=='s')
                prints(*ip);
            else if(*cp=='u')
                printu(*ip);
            else if(*cp=='d')
                printd(*ip);
            else if(*cp=='o')
                printo(*ip);
            else if(*cp=='x')
                printx(*ip);
            
            cp++; // Move cp to the next location (move one address over)
            ip++; // Move to the next item on our stack (move one address over)
        }
        else
        {
            // Check if new line. If not, must be char
            if(*cp=='\n')
            {
                putchar('\n');
                putchar('\r');
            }
            else
                putchar(*cp);
            
           cp++;
        } 
    }
}

/** 
  * mymain - used for testing lab
  * Provided by KC Wang
  */ 
mymain(int argc, char *argv[ ], char *env[ ])
{
  int i;

  myprintf("in mymain(): argc=%d\n", argc);

  for (i=0; i < argc; i++)
    myprintf("argv[%d] = %s\n", i, argv[i]);
    
  // Print the env variables
  printf("====== Environment variables ======\n");

  i=0; //reset i
  while(*env)
  {
    myprintf("ENV[%d]: %s\n", i, *env); 
    env++; 
    i++;
  }

  myprintf("---------- testing YOUR myprintf() ---------\n");
  myprintf("this is a test\n");
  myprintf("testing a=%d b=%x c=%c s=%s\n", 123, 123, 'a', "testing");
  myprintf("string=%s, a=%d  b=%u  c=%o  d=%x\n",
           "testing string", -1024, 1024, 1024, 1024);
  myprintf("mymain() return to main() in assembly\n"); 
}
