/**
  * Austin Schorsch
  * Cpts360 - Lab 3 (sh Simulator)
  *
  * Date: 9/20/15
  *
  * Latest Revision: 9/23/15
  * 				- Removed helper print outs
  *                 - Still may have issue with '>>' redirection. Unclear.   
  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>


// Global Variables
char path[128], home[128];                      /* Contains the path and the home directory */
char dirnames[32][64];                          /* Dirnames contained in PATH */
char input[32], *inputArr[] = {};               /* Contains user input and tokeninized strings */
char *myargv[] = {}, *file;
int pathCount, inputCount;                      /* Keeps track of path counter and input counter */
int rdSymbol = 0; 								/* Contains the redirection symbol */

// Function Declarations
void showPath();
void showHome();
void initialPrompt(char *cpenv[]);
void handle(char *line, char *cpenv[]);
void executeCommand(char *cmd, char *cpenv[]);
void clearInputArr();
 void buildFile(char *line, int iter);
void do_command(char *line, char *cpenv[]); 
int ioredir(char *line); 

main(int argc, char *argv[], char *env[])
{ 
    /** Initialization:
      *     (1). Show the path
      *     (2). Decompose path into dir strings
      *     (3). Show HOME directory
      */
    printf("********** Austin's Shell Simulator **********\n");
    showPath();
    showHome();

    // Wait for the user to press enter to start SHELL, then begins
    char enter = 0;
    printf("\nPress enter to start shell...\n");
    while (enter == 0) { enter = getchar(); }
    system("clear");

    // Loop the shell until the user wants to EXIT
    while (1)
    {
        initialPrompt(env);
    }
}

/**
  * Function: initialPrompt(char *cpenv[]) : void
  * Description: Core display of shell
  */
void initialPrompt(char *cpenv[])
{
    printf("austinschorsch@sh: $ ");
    fgets(input, sizeof(input), stdin);
    
    if (strcmp(input, "\n") == 0) { initialPrompt(cpenv); }              /* Did the user not enter anything? */
    else
    {
      // Remove the \n terminator
      size_t s = strlen(input) - 1;
      if (input[s] == '\n') { input[s] = '\0'; }
      
      /**
        * Input is stored. Handle it appropriately by:
        *     (1). Breaking up all the commands into substrings
        *     (2). Then, handling that command(s)
        */ 
       handle(input, cpenv);
    }
}

/**
  * Function: handle(char *line) : void
  * Description: Based off user's command, reads the input and executes either:
  *                 (a). cd
  *                 (b). exit
  *                 (c). any other command
  * Revisions: None
  */
void handle(char *line, char *cpenv[])
{
    int index = 0;
    char original[64]; 
    strcpy(original, line); 

    // First off, the string needs to be tokenized
    char *token = strtok(line, " ");
    while(token)
    {
        inputArr[index] = token;
        token = strtok(NULL, " ");
        inputCount++; index++;
    }

    /**
      * There are two 'base cases' here:
      *     (1). User enters 'cd ...'; manually change directory
      *     (2). User enters 'exit'; manually exit
      * Otherwise, handle with execve
      */
      if (strcmp(inputArr[0], "cd") == 0)        /* User entered 'cd' */
      {
            // The user entered 'cd': If no other args, go home; else, go to directory
            if (inputCount == 1)
            {
                chdir(home); 
                printf("cd to HOME\n");
            }
            else
            {
                int rv = chdir(inputArr[1]);     /* A return value of -1 means failure */ 
                if (rv == -1) { printf("cd to %s FAILED!\n", inputArr[1]); }
                else
                {
                    printf("cd to %s\n", inputArr[1]);
                }
            }
      }
      else if (strcmp(inputArr[0], "exit") == 0) /* User entered 'exit' */
      {
          exit(-1); 
      }
      else
      {
		  clearInputArr();
          executeCommand(original, cpenv);       /* Send to execute any other command */
      }
       
      // Free array and reset the input counter
      clearInputArr();
      putchar('\n');
}

/**
 * Function: clearInputArr() : void
 * Description: Simply clears the contents of the input array
 */
void clearInputArr() 
{
	int i = 0;
    for (i = 0; i < inputCount; i++)
    {
		if (inputArr[i] != NULL)
        {
			inputArr[i] = NULL;
        }
    } 
    inputCount = 0;
}

/**
  * Function: executeCommand(char *line, char *cpenv[]) : void
  * Description: Handles all other commands excluding cd and exit
  *              Also handles the piping
  */ 
void executeCommand(char *line, char *cpenv[])
{
    int pid, status, pipeArr[2]; 		/* pipe array used for creating creating a pipe and placing file descriptors */
    
    /**
     * strtok the line to get the head and tail
     * Note: if tail is null, then we have NO pipe
     */
    char *head = strtok(line, "|"); 
    char *tail = strtok(NULL, "|");
    //printf("head: %s\ntail: %s\n", head, tail);					                    /* TEST PRINT */
   
	pid = fork();
	//if (pid != 0)											/* Don't print if pid is 0 */
	//	printf("Parent PROC %d forks a child process %d\n", getppid(), pid);
	if (pid)
	{
		//printf("Parent PROC %d waits\n", getppid());                                  /* TEST PRINT */
		pid = wait(&status);
		//printf("Child PROC %d has died : exit status = %04x\n", pid, status);         /* TEST PRINT */
	}
	else
	{
		/**
		 * After the initial fork has been completed, check if we have a pipe or not
		 * Note: This is dependent on whether or not tail is NULL
		 * 
		 * About dup2:
		 * 			int dup2(int fildes, int fildes2); 
		 * 						(1). filedes is the source file descriptor. Remains open after dup2
		 * 						(2). filedes2 is the destination file descriptor. Points to same file as filedes 
		 */
		if (tail != NULL) 
		{						
			/**
			 * We have a pipe. Fork another child. 
			 *      pipeArr[0] -> first integer in array, open for reading
			 *      pipeArr[1] -> second integer in array, open for writing
			 * Note: Parent reads, Child writes
			 */
			pipe(pipeArr);					         /* Establish the file descriptors */
			pid = fork(); 
			if (pid)
			{
				/**
				 * When piping, it is best to move from right to left 
				 * Parent wants to receive data from child -- close end for writing
				 */
				close(pipeArr[1]);					 /* Close end for writing (parent will not write) */
				dup2(pipeArr[0], 0);				 /* Redirect the stdin of this process to the pipe */
				close(pipeArr[0]);					 /* Close the source file descriptor */
				do_command(tail, cpenv);
			}
			else
			{
				/**
				 * Want our child to handle writing 
				 * Child wants to receive data from parents -- close end for reading
				 * Therefore, close the end for reading
				 */ 
				close(pipeArr[0]); 					/* Close end for reading (child will not read) */
				dup2(pipeArr[1], 1);				/* Redirect the stdout of this process to the pipe */
				close(pipeArr[1]); 					/* Close the source file descriptor */
				do_command(head, cpenv);
			}
		}
		else
		{
            // There was no tail, therefore the user entered a simple command (i.e. ls -l)
			do_command(head, cpenv);
			exit(0);
		}
	}
}

/**
  * Function: do_command(char *line) : void
  * Description: Given any command (i.e. ls -l), checks if there are:
  * 				(a). Redirection
  * 				(b). Piping
  * 				(c). None of the above
  * 			 Then, executes the command a
  */
void do_command(char *line, char *cpenv[])
{    
    /** 
      * First, check if the command has a file redirection
      * Then store the type of redirection in rd (Note: 0 -> no redirection)
      */
    rdSymbol = ioredir(line);               
    int rd = rdSymbol; 

    /**
	  * Create our own argv
	  * 		Example: ls -l
	  * 			myargv[0] = ls
	  * 			myargv[1] = -l
	  */
	char *cmd = strtok(line, " ");          /* Break up the user input into strings */
	char *tcmd = cmd; 		
	int i = 0, loc = 0;
	while (tcmd)
	{
		 myargv[i] = tcmd;
		 if ((strcmp(tcmd, ">") == 0) || (strcmp(tcmd, "<") == 0)) { loc = i; } // Locate the redirect symbol
		 tcmd = strtok(NULL, " ");
		 //printf("myargv[%d]=%s\n", i, myargv[i]); 
		 i++;
	}
	
	 if (rd != 0)											/* Handle the file redirection here */
	 {
		 if (myargv[loc+1] == ">") { file = myargv[loc+2]; } 		// '>>' symbol -- Meaning the spot after the location found is '>'
		 else { file = myargv[loc+1]; }
		 
		 /** 
		  * There must be file redirection now. 
		  * 		(1). '<'  : for reading, close end for reading, open file for reading
		  * 		(2). '>'  : for writing, close end for writing, open file for writing
		  * 		(3). '>>' : for writing/appending, close end for writing, open file for reading/writing/appending
		  */ 
		 if (rd == 1) // '<'
		 { 		
				close(0);
				open(file, O_RDONLY); 
		 }
		 else if (rd == 2) // '>'
		 {
				close(1);
				open(file, O_WRONLY|O_CREAT, 0644);
		 }
		 else 	// '>>'
		 {
				close(1);
				open(file, O_RDWR|O_APPEND,0644);
		 }
	 }
	 
	 char temp[128]; 	
	 int iter = 0;
	 for (iter = 0; iter < pathCount; iter++)
	 {
		 /** 
		  * For each path in dirnames, append the command and try executing 
		  */ 
		 strcpy(temp, dirnames[iter]); 
		 strcat(temp, "/"); 
		 strcat(temp, cmd); 					
		 //printf("i=%d	%s\n", iter, temp);				/*  TEST PRINT */
		 execve(temp, myargv, cpenv);
	 } 
	 
	 // Clear out myargv
	 iter = 0;
	 while (*myargv)
	 {
		 if (myargv[iter] != NULL) { myargv[iter] = NULL; }
		 iter++;
	 }		 
}

/**
  * Function: redirect(char *line) : char
  * Description: Helper function that searches through a line and checks for a redirection symbol
  *              Returns that symbol as an integer value
  * 					(1). < is equal to 1
  * 					(2). > is equal to 2
  * 					(3). >> is equal to 3
  */
int ioredir(char *line)
{
	int iter = 0;
	
	// Loop through the line looking for a redirection symbol. If one is found, return an int indicating which one. 
	while (iter < strlen(line))
	{
		if (line[iter] == '<') { return 1; }
		else if (line[iter] == '>')
		{
			if (line[iter+1] == '>') { return 3; }
			else { return 2; }
		}
		
		iter++; 
	}
	
	// No redirects were found, return 0
	return 0;
}

/**
  * Function: showPath(char *env[]) : void
  * Description: Given the env[], finds the string in the array which contains the path
  *              and sets the path in the global variable
  *              Note: ALSO tokenizes the path into path dir names and displays
  * Revisions: None
  */
void showPath()
{
    int index = 0;

    // Simply get the path from the env string
    strcpy(path, getenv("PATH"));
    printf("1. Show PATH:\nPATH=%s\n", path);

    /**
      * Now that the path has been established, tokenize to get the dirnames
      * Secondly, loop through the dirnames and display
      */
    char *token;
    token = strtok(path, ":");

    while(token)
    {
        strcpy(dirnames[index], token);
        token = strtok(NULL, ":");

        index++; pathCount++;
    }

    printf("2. Decompose PATH into dir strings:\n");
    for (index = 0; index < pathCount; index++)
    {
        printf("%s ", dirnames[index]);
    }
    putchar('\n');
}

/**
  * Function: showHome() : void
  * Description: Given the env[], finds the string in the array which contains the home directory
  *                 and sets home in the global variable
  * Revisions: None
  */
void showHome()
{
    strcpy(home, getenv("HOME"));
    printf("3. Show HOME directory: HOME = %s\n", home);
}
