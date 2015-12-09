/**
* Author : Austin Schorsch
* Description : Cpts 360 Lab 2 - File System in C
* Date : 9/15/15
*
* Old Revisions		: 9/17/15
*						- Added pwd function
*						- Fixed issues with ls, mkdir, cd
*
* Latest Revision		: 9/18/15
*						- Finalized all functions
*						- Heavy testing to ensure correct functionality
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <conio.h>

/**
* Struct: NODE
* Description: Used to represent the nodes within the file system.
* Notes: Each Node has a parent, a child, and a sibling.
*/
typedef struct Node
{
	char name[64];
	char type;
	struct Node *parentPtr,
		*childPtr,
		*siblingPtr;
} NODE;

/**
* Global variables
* Description: Will keep track of things such as our root and the user input
*/
NODE *root, *cwd, *lastUs;                    /* root and CWD pointers */
char line[128], username[64];                 /* user input line */
char command[16], pathname[64];               /* user inputs */
char dirname[64], basename[64];               /* string holders */
char path[128], filename[64];
char dirtype, last[16];						  /* file handling variables */
char *tokenArr[64];							  /* holds the tokens of the given pathname */
int fptrIndex, tokenCount;					  /* holds function ptr index and number of tokens*/
int absolute = 1;    						  /* holds truth value of absolute pathname (init. to false) */
FILE *fp;

/**
* Function declarations
*/
void initialize();
void prompt();
int findCommand(char *command);
int menu(char *pathname);
int mkdir(char *pathname);
int rmdir(char *pathname);
int ls(char *pathname);
int cd(char *pathname);
int pwd(char *pathname);
void rpwd(NODE *currDir);
int creat(char *pathname);
int rm(char *pathname);
int save(char *pathname);
int reload(char *pathname);
int quit(char *pathname);
void breakUp(char *pathname);
void cleanUp();
char * concat(char *path, char *name);
NODE * search(char *pathname);
NODE *createNode(NODE *parent, char *name, char type);
void printToFile(NODE *us);
void frpwd(NODE *currDir);
void checkChildrenOf(NODE *r, char *basename, char type);
void printPaths(NODE *us);
int(*fptr[])(char *) = { (int(*)())menu, mkdir, rmdir, cd, ls, creat, rm, pwd, reload, quit };

/**
* Function: Main() : int
* Description: Simply used to execute the file system
* Notes: None
* Revisions: None
*/
int main()
{
	/**
	* The steps for execution are as follows:
	(1). Start with a / node.
	(2). Prompt the user for a command:
	mkdir, rmdir, cd, ls, pwd, creat, rm, save, reload, quit
	(3). Execute the command, with appropriate tracing messages.
	(4). Repeat (2) until the "quit" command.
	*/
	initialize();
	while (1)
	{
		int r = -1;

		prompt();
		fptrIndex = findCommand(command);
		if (fptrIndex != -1)						/* Ensure the command is valid */
			r = fptr[fptrIndex](pathname);
		else { printf("%s: command not found\n", command); }

		cleanUp();
		putchar('\n');

		// Did the user want to quit? 
		if (r == -2) { break; }
	}

	return 0;
}

/**
* Function: quit(char * command) : int
* Description: Terminates program. Prompts the user to save.
*/
int quit(char *pathname)
{
	save(pathname);

	return -2;
}

/**
* Function: cleanUp() : void
* Description: Simply cleans up key variables
*/
void cleanUp()
{
	strcpy(dirname, "");
	strcpy(basename, "");
	strcpy(pathname, "");
	strcpy(command, "");
	*tokenArr = NULL;
}

/**
* Function: initialize() : void
* Desription: Used to initialize the root node for our file directory
* Revisions: None
*/
void initialize()
{
	int reset = 0;
	/**
	* Create a root node.
	* Note: Every file system must start with a root node denoted by: '\'
	*/
	root = malloc(sizeof(NODE));
	root->type = 'D';
	strcpy(root->name, "/");
	root->parentPtr = NULL;
	root->childPtr = NULL;
	root->siblingPtr = NULL;
	cwd = root;						/* set the cwd to root to start */

	// Show initial prompt and get the user command and pathname
	char enter = 0;
	printf("*** Note: Upon seeing '$', please enter a command ('menu' for help) ***\n");
	printf("Enter username: "); scanf(" %[^\n]s", username);
}

/**
* Function: prompt() : void
* Description: Used to prompt user for command and pathname
* Revisions: None
*/
void prompt()
{
	printf("%s: ", username);
	pwd(pathname);
	printf("$ ");
	scanf(" %[^\n]s", line);

	int tokenCount = 0;
	char *token = strtok(line, " ");
	while (token != NULL)
	{
		// Assign the tokens to both command and pathname
		if (tokenCount == 0) { strcpy(command, token); tokenCount++; }
		else { strcpy(pathname, token); tokenCount++; }
		token = strtok(NULL, " ");
	}
}

/**
* Function: findCommand(char *command) : int
* Description: Returns the fptr index based on the command
* Revisions: None
*/
int findCommand(char *command)
{
	if (strcmp(command, "menu") == 0) { return 0; }
	else if (strcmp(command, "mkdir") == 0) { return 1; }
	else if (strcmp(command, "rmdir") == 0) { return 2; }
	else if (strcmp(command, "cd") == 0) { return 3; }
	else if (strcmp(command, "ls") == 0) { return 4; }
	else if (strcmp(command, "creat") == 0) { return 5; }
	else if (strcmp(command, "rm") == 0) { return 6; }
	else if (strcmp(command, "pwd") == 0) { return 7; }
	//else if (strcmp(command, "save") == 0) { return 9; }
	else if (strcmp(command, "reload") == 0) { return 8; }
	else if (strcmp(command, "quit") == 0) { return 9; }
	else { return -1; }
}

/**
* Function: menu(char *pathname) : int
* Description: Prints the menu of user options when running the system
* Revisions: Nones
*/
int menu(char *pathname)
{
	printf("mkdir  pathname	: make a new directory for the pathname\n");
	printf("rmdir  pathname	: rm the directory, if it is empty.\n");
	printf("cd[pathname]	: change CWD to pathname, or to / if no pathname.\n");
	printf("ls[pathname]	: list the directory contents of pathname or CWD\n");
	printf("pwd		: print the (absolute) pathname of CWD\n");
	printf("creat  pathname	: create a FILE node.\n");
	printf("rm     pathname : rm a FILE node.\n");
	printf("save   filename : save the current file system tree in a file\n");
	printf("reload filename : re - initalize the file system tree from a file\n");
	printf("quit		: save the file system tree, then terminate the program.\n\n");

	return 0;
}

/**
* Function: mkdir(char *pathname) : int
* Description: Make a directory given a specified pathname
* Revisions: Fixed issue when a child of the top is not created
*/
int mkdir(char *pathname)
{
	int valid = 0;

	/**
	* First, clear out previous state of dirname and basename
	* Then, break up the pathname to reinstantiate both dirname and basename
	* See: "/a/b/c/d"
	*				dirname = "/a/b/c/"
	*				basename = "d"
	*
	* Base check for root pathname
	*/
	if (strcmp(pathname, "/") == 1) { breakUp(pathname); }
	else
	{
		printf("mkdir: cannot create directory '%s': File exists\n", pathname);
		valid = 1;
	}

	/**
	* Get the result of the search. Note the possible return values:
	*			(1). NULL NODE, signifying invalid dirname
	*			(2). NODE at the end of token array - 1 (The NODE one before the pathname)
	*/
	if (valid == 0)
	{
		NODE *r = search(dirname);
		if (r == NULL || r->type == 'F')
		{
			char *temp[64];
			strcpy(temp, dirname);
			strcat(temp, basename);
			printf("mkdir: cannot create directory '%s': No such file or directory.\n", temp);
		}
		else
		{
			/**
			* Last, check the children of the node to see if there exists a child equal to the basename
			*/
			checkChildrenOf(r, basename, 'D');
		}
	}

	return 0;
}

/**
* Function: rmdir(char *pathname) : int
* Description: Removes a directory given a specified pathname
* Revisions: None
*/
int rmdir(char *pathname)
{
	// Base check for root pathname
	char path[64];
	strcpy(path, pathname);
	if (strcmp(pathname, "/") == 1) { breakUp(path); }
	else { printf("rmdir: cannot remove '%s' directory\n", pathname); }

	/**
	* Get the result of the search. Note the possible return values:
	*			(1). NULL NODE, signifying invalid pathname
	*			(2). NODE at the end of token array
	*/
	NODE *r = search(pathname);
	if (r == NULL)
	{
		printf("rmdir: cannot remove '%s': No such directory.\n", pathname);
	}
	else
	{
		// Ensure that we have a 'D' type
		if (r->type == 'D')
		{
			if (r->childPtr == NULL)	/* The only way we can remove a directory is if it is empty */
			{
				// Go up to the parent, then search for 'us' and set to NULL
				char *name = r->name, atLocation = 0;
				NODE *last = r->parentPtr->childPtr, *parent = r->parentPtr;

				r = r->parentPtr->childPtr;
				while (atLocation == 0)
				{
					if (strcmp(r->name, name) == 0) { atLocation = 1; }
					else
					{
						last = r;
						r = r->siblingPtr;
					}
				}

				/**
				* Set last's sibling pointer to NULL
				* Note: There exists a case when the first NODE under parent is to be deleted. Reassign parents child ptr
				*/
				if (last == r) { parent->childPtr = last->siblingPtr; }
				else { last->siblingPtr = r->siblingPtr; }
			}
			else { printf("rmdir: cannot remove '%s': Is a directory.\n", pathname); }
		}
		else { printf("rmdir: cannot remove '%s': Not a directory.\n", pathname); }
	}

	return 0;
}

/**
* Function: ls(char *pathname) : int
* Description: Given a pathname, print the contents of the linked-list under the end of that pathname
*			   This works very similar to cd without a permanent change in directory.
* Revisions: Added pathname fix
*/
int ls(char *pathname)
{
	/**
	* ls can work very similar to cd, except without a permanent directory change
	* First, move a temporary directory to the pathname and once set, print children underneath that directory
	*/
	NODE *location = cwd;
	int valid = 1;
	if (strcmp(pathname, "") == 0) { location = cwd; }
	else
	{
		// Search the pathname, then move there
		char path[64];
		strcpy(path, pathname);
		breakUp(path);
		NODE *temp = search(pathname);
		if (temp == NULL)
		{
			printf("ls: %s: No such file or directory.\n", pathname);
			valid = 0;
		}
		else if (temp->type == 'F')
		{
			printf("ls: %s: Not a directory.\n", pathname);
			valid = 0;
		}
		else
		{
			location = temp;
		}
	}

	// A temporary directory is set. Print the linked-list of children under it. 
	if (location->childPtr != NULL && valid == 1)
	{
		NODE *t = location->childPtr;

		while (t != NULL)
		{
			printf("%c: %s/\n", t->type, t->name);
			t = t->siblingPtr;
		}

	}

	return 0;
}

/**
* Function: cd(char *pathname) : int
* Description: Changes the directory to the user's pathname input
* Revisions: Need to fix "/" character at end of strings for search function
*/
int cd(char *pathname)
{
	/**
	* For cd, we have 3 cases:
	*		(1). cd				: This moves the cwd to root
	*		(2). cd ..			: This moves the cwd to parent of 'us'
	*		(3). cd [pathname]	: This moves cwd to pathname
	*/
	if (strcmp(pathname, "") == 0) { cwd = root; }
	else if (strcmp(pathname, "..") == 0)
	{
		if (cwd != root) { cwd = cwd->parentPtr; }
	}
	else
	{
		// Search the pathname, then move there
		char path[64];
		strcpy(path, pathname);
		breakUp(path);
		NODE *temp = search(pathname);
		if (temp == NULL)
		{
			printf("cd: %s: No such file or directory.\n", pathname);
		}
		else if (temp->type == 'F')
		{
			printf("cd: %s: Not a directory.\n", pathname);
		}
		else { cwd = temp; }
	}

	return 0;
}

/**
* Function: pwd(char *pathname) : int
* Description: Function called by user. Bulk of printing the working directory is in function rpwd(NODE *currPtr)
* Revisions: None
*/
int pwd(char *pathname)
{
	/**
	* Create a temporary variable for the current working directory, then simply call the rpwd
	* function to print out the files/directories under that directory
	*/
	NODE *temp = cwd;
	rpwd(temp);
	printf("/");

	putchar('\n');
	return 0;
}

/**
* Function: rpwd(NODE *currDir) : void
* Description: The work behind pwd. Recursively 'climbs' up the directories until root. Once there, prints back down.
* Revisions: Fixed issue with double "/" symbol for root
*/
void rpwd(NODE *currDir)
{
	if (currDir != root)				/* Base case check: Ensure root is not null */
	{
		if (currDir->parentPtr != root)
			rpwd(currDir->parentPtr);

		printf("/%s", currDir->name);
	}
}

/**
* Function: creat(char *pathname) : int
* Description: Same exact code as mkdir, except for print statements and type parameter
* Revisions: None
*/
int creat(char *pathname)
{
	/**
	* First, clear out previous state of dirname and basename
	* Then, break up the pathname to reinstantiate both dirname and basename
	* See: "/a/b/c/d"
	*				dirname = "/a/b/c/"
	*				basename = "d"
	*
	* Base check for root pathname
	*/
	if (strcmp(pathname, "/") == 1) { breakUp(pathname); }
	else { printf("creat: cannot create file '%s': File exists\n", pathname); }

	/**
	* Get the result of the search. Note the possible return values:
	*			(1). NULL NODE, signifying invalid dirname
	*			(2). NODE at the end of token array - 1 (The NODE one before the pathname)
	*/
	NODE *r = search(dirname);
	if (r == NULL)
	{
		char *temp[64];
		strcpy(temp, dirname);
		strcat(temp, basename);
		printf("creat: cannot create file '%s': No such file or directory.\n", temp);
	}
	else if (r->type == 'F') { printf("creat: cannot create file '%s': File exists\n", pathname); }
	else
	{
		/**
		* Last, check the children of the node to see if there exists a child equal to the basename
		*/
		checkChildrenOf(r, basename, 'F');
	}

	return 0;
}

/**
* Function: rm(char *pathname) : int
* Description: Same as rmdir, just change the no children check to making sure we are dealing with a file
* Revisions: None
*/
int rm(char *pathname)
{
	// Base check for root pathname
	char path[64];
	strcpy(path, pathname);
	if (strcmp(pathname, "/") == 1) { breakUp(path); }
	else { printf("rm: cannot remove '%s' directory\n", pathname); }

	/**
	* Get the result of the search. Note the possible return values:
	*			(1). NULL NODE, signifying invalid pathname
	*			(2). NODE at the end of token array
	*/
	NODE *r = search(pathname);
	if (r == NULL)
	{
		printf("rm: cannot remove '%s': No such file.\n", pathname);
	}
	else
	{
		// Ensure that we have a 'F' type
		if (r->type == 'F')
		{
			// Go up to the parent, then search for 'us' and set to NULL
			char *name = r->name, atLocation = 0;
			NODE *last = r->parentPtr->childPtr, *parent = r->parentPtr;

			r = r->parentPtr->childPtr;
			while (atLocation == 0)
			{
				if (strcmp(r->name, name) == 0) { atLocation = 1; }
				else
				{
					last = r;
					r = r->siblingPtr;
				}
			}

			/**
			* Set last's sibling pointer to NULL
			* Note: There exists a case when the first NODE under parent is to be deleted. Reassign parents child ptr
			*/
			if (last == r) { parent->childPtr = last->siblingPtr; }
			else { last->siblingPtr = r->siblingPtr; }
		}
		else { printf("rm: cannot remove '%s': Is a directory.\n", pathname); }
	}

	return 0;
}

/**
* Function: save(char *pathname) : int
* Description: Saves the users file system.
* Revisions: None
* NOTE: By design choice, user can only save when quitting. Otherwise, not an eligible command
*/
int save(char *pathname)
{
	NODE *t = root;

	// The only way we need need to print to file is if the root has 'children'
	if (t->childPtr == NULL) { printf("save: No files to save.\n"); }
	else
	{
		/**
		* After prompting for file information,
		* begin by going down a level
		*/
		printf("save: Please enter a filename: ");
		scanf("%s", filename);
		fp = fopen(filename, "w+");						/* open a FILE stream for WRITE */
		if (fp != NULL)
		{
			printPaths(t->childPtr);
			fclose(fp);
		}
	}

	return 0;
}

/**
* Function: printPaths(NODE *us) : void
* Description: Helper function for save. Does the recursive pre-order traversal through the fd's
* Revisions: Fixed issue with printing more than one layer deep with same dir names
*/
void printPaths(NODE *us)
{
	if (us != NULL)
	{
		dirtype = us->type;
		printPaths(us->childPtr);
		if (us->childPtr == NULL)				/* If us' child NULL, recursive print to file */
		{
			printToFile(us);
		}
		printPaths(us->siblingPtr);
	}

	return;
}

/**
* Function: printToFile(NODE *us) : void
* Description: Essentially same function as pwd but with a file
* Revisions: Fixed too many '/' characters
*/
void printToFile(NODE *us)
{
	char type = us->type;
	fprintf(fp, "%c ", type);
	frpwd(us);
	fprintf(fp, "/\n");
}

/**
* Function: frpwd(NODE *currDir) : void
* Description: Essentially same function as rpwd but with a file
* Revisions: None
*/
void frpwd(NODE *currDir)
{
	if (currDir != root)				/* Base case check: Ensure root is not null */
	{
		if (currDir->parentPtr != root)
			frpwd(currDir->parentPtr);

		fprintf(fp, "/%s", currDir->name);
	}
}

/**
* Function: reload(char *pathname) : int
* Description: Given a filename, reads line by line to produce file directory 'tree'
* Revisions: None
*/
int reload(char *pathname)
{
	/**
	* The first step is to prompt for a filename. Once completed, begin building a tree.
	*/
	printf("reload: Please enter a filename: ");
	scanf("%s", filename);
	fp = fopen(filename, "r");						/* open a FILE stream for WRITE */
	if (fp != NULL)
	{
		char str[100]; char filetype = 0;
		while (fgets(str, 100, fp) != NULL)
		{
			char *tArr[64] = { { 0 } }; int tCount = 0;	int index = 0;


			filetype = str[0];							/* Sets the file 'type' */
			char *token = strtok(str, " ");				/* This line essentially splits the filetype from the rest of the string */
			//*token = strtok(str, " ");
			while (token = strtok(0, "/"))				/* keep calling strtok() with NULL string */
			{
				// Add to the token array
				if (strcmp(token, "\n") == 1)
				{
					tArr[index] = token;
					index++;
				}
			}
			tCount = index;


			/**
			* At this point, the tokens are split up in the array
			* Simply loop through them while making directories (unless last is a file type)
			*/
			int iter = 0;
			NODE *front = root, *back = root;
			char directory[64];
			for (iter = 0; iter < tCount; iter++)
			{
				// Keep building the directory name (i.e. A/ -> A/B/ -> A/B/C)
				if (iter == 0)
					strcpy(directory, tArr[iter]);
				else
					strcat(directory, tArr[iter]);

				strcat(directory, "/");

				// Only check: Make sure we are not at the end
				if (iter == tCount - 1)
				{
					switch (filetype)
					{
					case 'D': mkdir(directory); break;
					case 'F': creat(directory); break;
					}
				}
				else
				{
					char temp[64];								/*	For some reason, was		*/
					strcpy(temp, directory);					/*	losing directory's state	*/
					mkdir(temp);
				}
			}

			strcpy(directory, "");
		}

		printf("reload: Complete. Dismiss errors...\n");
	}
	else
	{
		printf("reload: File '%s' does not exist.", filename);
	}

	return 0;
}

/**
* Function: breakUp(char *pathname) : void
* Description: Splits up the pathname given on a command into the dirname and basename
*				i.e. "/a/b/c/d" -> dirname: a/b/c/   basename: d
* Revisions: Fixed base case of only one command in mkdir (i.e. A)
*/
void breakUp(char *pathname)
{
	int index = 0;
	// Clear out the token array
	for (index = 0; index < tokenCount; index++)
	{
		tokenArr[index] = NULL;
	} index = 0;

	// First step is determine if absolute or not
	if (pathname[0] == '/') { absolute = 1; }
	else { absolute = 0; }

	char *token = strtok(pathname, "/");
	tokenArr[index] = token;
	index++;

	while (token = strtok(0, "/"))				/* keep calling strtok() with NULL string */
	{
		// Add to the token array
		tokenArr[index] = token;
		index++;
	}

	/**
	* Now, cycle through the token array all the way up to the end - 1
	* Note: Append '/' if the absolute is set to true
	*/
	int iter = 0;
	if (absolute == 1 || index == 1) { strcpy(dirname, "/"); }
	for (iter = 0; iter < index - 1; iter++)
	{
		strcat(dirname, tokenArr[iter]);
		strcat(dirname, "/");					/* Set the dirname to all the tokens concatenated */
	}

	tokenCount = index;
	strcpy(basename, tokenArr[index - 1]);		/* Leftover is the basename */
}

/**
* Function: search(char *pathname) : NODE *
* Description: Performs a dirname search. In other words, will search to see if a given dirname exists by traversing the file 'tree'
* Revisions: Keeping tabs on tokenCount and whether iter should go to tC - 1 or just tC. tokenCount seems to be working as is for now.
*/
NODE *search(char *pathname)
{
	// First step, check if the path is absolute
	NODE *r = malloc(sizeof(NODE));
	if (absolute == 1) { r = root; }
	else { r = cwd; }

	/**
	* Start going down the levels of the tree until either:
	*			(1). The pathname is found to be correct
	*			(2). The pathname is found to not exist
	*/
	int iter = 0;
	if (tokenCount > 1)						/* Only perform a search if we have more than 1 layer */
	{
		while ((r->childPtr != NULL) && (iter != tokenCount))// && (r->type != 'F'))
		{
			r = r->childPtr;				/* Move down a level. Begin searching through siblings for token */
			while (strcmp(r->name, tokenArr[iter]) != 0)
			{
				// Can we move to a sibling? If not, invalid pathname 
				if (r->siblingPtr != NULL) { r = r->siblingPtr; }
				else if (iter == tokenCount - 1) { return r->parentPtr; }
				else { return NULL; }
			}
			iter++;
		}

		/* We found the dir, is that an error or good? */

		if (iter == 0) { return NULL; }
		if (strcmp(r->name, tokenArr[tokenCount - 1]) == 0) 
		{
			if (strcmp(command, "ls") != 0) 
			{
				if (strcmp(command, "cd") != 0)
				{
					if ((strcmp(command, "rmdir") != 0) && (strcmp(command, "rm")))
						return NULL;
				}
			}
		}
	}
	else if (strcmp(pathname, "/") != 0)			/* Do we have 1 layer but not root? */
	{
		// Check to see if the pathname exists in the first layer (essentially same idea as checkChildrenOf() function
		if (r->childPtr != NULL)
		{
			r = r->childPtr;
			while (r != NULL)
			{
				if (strcmp(r->name, tokenArr[0]) == 0) { return r; }
				r = r->siblingPtr;
			}
		}
		return NULL;
	}

	return r;
}

/**
* Function: createNode(NODE *parent, char *name, char type) : NODE *
* Description: Simply creates a new NODE with parameters passed in
* Revisions: None
*/
NODE *createNode(NODE *parent, char *name, char type)
{
	NODE *newNode = malloc(sizeof(NODE));
	newNode->childPtr = NULL;
	newNode->siblingPtr = NULL;
	newNode->parentPtr = parent;
	strcpy(newNode->name, name);
	newNode->type = type;

	return newNode;
}

/**
* Function: checkChildrenOf(NODE *r, char *basename) : void
* Description: Once at the last 'level' of a dirname (i.e. "a/b/c/" of "a/b/c/d"), checks to see if the basename exists
* Revisions: None
*/
void checkChildrenOf(NODE *r, char *basename, char type)
{
	int iter = 0, invalid = 0;
	NODE *temp = r, *last = malloc(sizeof(NODE));
	if (temp->childPtr != NULL)
	{
		temp = temp->childPtr;						/* Move down to the child, then start looking right (siblings) */
		while (temp != NULL)
		{
			if (strcmp(temp->name, basename) == 0) // && temp->type == 'D')
			{
				char *temp[64];
				strcpy(temp, dirname);
				strcat(temp, basename);
				printf("mkdir: cannot create directory '%s': File exists.\n", temp);
				invalid = 1;
				break;
			}

			last = temp;
			temp = temp->siblingPtr;
		}
		iter = 1;
	}

	// Given success, place the new NODE in
	if (iter == 0) { r->childPtr = createNode(r, basename, type); }		/* There is nothing in the directory*/
	else
	{
		if (invalid == 0) { last->siblingPtr = createNode(r, basename, type); }
	}
}==
