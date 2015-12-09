/**
 * Author: Austin Schorsch
 * Final Project
 *
 * Collaborated with: Trevor Mozingo
 *
 * Last Revision: 11/12/15
 */

#include "type.h"

MINODE minode[NMINODE];
MINODE *root;
PROC   proc[NPROC], *running;
MOUNT  mounttab[5];

char names[64][128],*name[64];
int fd, dev, n;
int nblocks, ninodes, bmap, imap, inode_start, iblock;

char pathname[256], parameter[256], command[32];
char usrname[32];

char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";

char buf[1024], buf2[32];
char *pathnames[64], path[128];

int DEBUG;

int get_block(int dev, int blk, char *buf);
int put_block(int dev, int blk, char *buf);

/* Function Pointer
char *functionStrings[2] = { "ls", "quit" };
int (*functions[2])() = { ls, quit };
*/

// Global Variables
int i = 0, pathcount = 0, inodebeginblock = 0,
    divisor = 0;

/**
 * Function:
 * Description:
 */
void shell()
{
    char *line;

    printf("sh@%s $ ", usrname);
    scanf(" %[^\n]s", line);

    // Tokenize the command and pathname
    int tokenCount = 0;
    char *token = strtok(line, " ");
    while (token != NULL) {
        // Assign the tokens to both command and pathname
        if (tokenCount == 0) { strcpy(command, token); tokenCount++; }
        else { strcpy(pathname, token); tokenCount++; }
        token = strtok(NULL, " ");
    }

    // Run the command given
    if (strcmp(command, "ls") == 0) {
        ls(pathname);
    }
    else if (strcmp(command, "cd") == 0) {
        cd(pathname);
    }
    else if (strcmp(command, "quit") == 0)
        quit();
    else {
        printf("%s: Command does not exist\n", command);
    }

    reset();
    putchar('\n');
}

/**
 * Function: reset() : void
 * Description: Clears out variables
 */
void reset()
{
    bzero(command, sizeof(command));
    bzero(pathname, sizeof(pathname));
}

/**
 * Function: quit() : void
 * Description: Quits the program and writes back all dirty MINODEs
 */
void quit()
{
     for (i = 0; i < NMINODE; i++) {
        if (minode[i].refCount > 0 || minode[i].dirty == 1) {
            minode[i].refCount = 1;
            minode[i].dirty = 0;
            iput(&(minode[i]));
        }
    }

    exit(0);
}

/**
 * Function: findFunction(char *input) : void
 * Description: Uses the function pointer table to locate the function
 *              based off what the user inputted
 *
void findFunction(char *input)
{
    while (functionStrings[i])
    {
        if (strcmp(functionStrings[i], input) == 0) {
            // Function was found, call it
            functions[i];
            return;
        } i++;
    }

    printf("%s: Command does not exist\n", input);
}*/

/**
 * Function: init() : void
 * Description: Initializes the proc structure and the minode ref count
 */
void init()
{
    /**
     * Initialize the 2 PROCs, which includes:
     *      (a) Setting PROC 0 uid = 0
     *      (b) Setting PROC 1 uid = 1
     *      (c) Setting all cwd's to 0
     */
    proc[0].uid = 0;
    proc[0].pid = 1;
    proc[0].cwd = 0;

    proc[1].uid = 1;
    proc[1].pid = 2;
    proc[1].cwd = 0;

    // Set all the minodes ref counts to 0
    for (i = 0; i < 100; i++) {
        minode[i].refCount = 0;
    }

    // Allocate our running
    running = &proc[0];

    // Last step is to set the MINODE root to 0
    root = 0;
}

/**
 * Function: get_block(int fd, int blk, char buf[BLKSIZE]) : int
 * Description: Get_block function provided by KC Wang
 */
int get_block(int fd, int blk, char buf[BLKSIZE])
{
    lseek(fd, (long)(blk*BLKSIZE), 0);
    read(fd, buf, BLKSIZE);
}

/**
 * Function: put_block(int fd, int blk, char buf[]) : int
 * Description: Put_block function provided by KC Wang
 */
int put_block(int fd, int blk, char buf[ ])
{
    lseek(fd, (long)blk*BLKSIZE, 0);
    write(fd, buf, BLKSIZE);
}

/**
 * Function: mount_root() : void
 * Description: Initializes program by mounting the root
 */
void mount_root()
{
    // The first step of mounting the root is to get the device name from the user
    printf("Enter root device name : ");
    scanf("%s", &buf);

    dev = open(buf, O_RDWR);

    if (dev < 0) {
        printf("Not an EXT2 File System\n");
        quit();
    }

    // read SUPER block
    get_block(dev, 1, buf);
    sp = (SUPER *)buf;

    // Set the divisor for mailman's
    divisor = BLKSIZE/sp->s_inode_size;

    // Ensure that the file type is an ext2 filesystem
    // by checking for the magic number (EF53)
    if (sp->s_magic != 0xEF53){
        printf("Not an EXT2 File System\n");
        exit(1);
    }

    // read GD block
    get_block(dev, 2, buf);
    gp = (GD *)buf;

    // Get inode start block number
    inodebeginblock = gp->bg_inode_table;

    // Lastly, copy the root inode from the ext2 and set
    // the pointer to point at the root in the MINODE array
    // iget returns a minode pointer
    root = iget(dev, 2);    /* get root inode */
    printf("imode=%4x\n", root->INODE.i_mode);

    // Let cwd of both P0 and P1 point at the root minode (refCount=3)
    proc[0].cwd = iget(dev, 2);
    proc[1].cwd = iget(dev, 2);
}

/**
 * Function: tokenize(char *path) : void
 * Description: This function takes the pathname entered by the user
 *              and parses it into pathnames
 */
void tokenize(char *path)
{
    char *token;
    int i = 0;

    // Check if our path starts with a root
    if (path[0] == '/')
        strcpy(buf2, &path[1]);
    else
        strcpy(buf2, path);

    // Ensure that we start one value over from the root
    token = strtok(buf2, "/");
    while (token)
    {
        pathnames[i] = token;

        token = strtok(NULL, "/");
        i++;
    } pathcount = i;
}

/**
 * Function: getino(int *&dev, char *pathname) : int
 * Description: Retrieves the inode for the pathname given by the user
 */
int getino(int *dev, char *pathname)
{
    // Create our INODE pointer
    INODE *ip;
    int inumber = 0, blocknum = 0, inodenum = 0;

    // Check the beginning of the the pathname
    // Note: We are looking for root at the beginning of the string
    if (strcmp(pathname, "/") == 0) { return root->ino; }
    else if (pathname[0] == '/') {
       ip = &(root->INODE);
    }
    else {
        ip = &(running->cwd->INODE);
    }

    // Split up the pathname and loop to find each directory within the path
    tokenize(pathname);
    for (i = 0; i < pathcount; i++) {
        printf("\n*************** inode info ***************\n");
        printf("Searching for pathname: %s\n", pathnames[i]);
        char *t = pathnames[i];

        inumber = search(*dev, ip, pathnames[i]);

        // Check if the DIR was found
        if (inumber == 0) {
            printf("name %s does not exist\n", pathnames[i]);
            return 0;           // No dir was found, return 0
        }
        else {
            // Use mailman's algorithm to get the block number and inode number
            // of the returned inumber from the search function
            inodenum = (inumber - 1) % divisor;
            blocknum = ((inumber - 1) / divisor) + inodebeginblock;

            // Make sure we are not trying to go into a regular file
            if (S_ISREG(ip->i_mode) && i < pathcount - 1) {
                printf("%s is not a directory. Cannot go further\n");
                break;
            }

            // Update the block and the inode
            printf("\nFound %s\nblocknum: %d inodenum: %d\n", t, blocknum, inodenum);
            get_block(dev, blocknum, buf);

            ip = (INODE *)buf + inodenum;
        }
    }

    // If get to this point, we have a inumber that is valid.
    // Return that inumber
    printf("************ end inode info **************\n");

    return inumber;
}

/**
 * Function: search(int dev, INODE *inodeptr, char *name) : int
 * Description:
 */
int search(int dev, INODE *inodeptr, char *name)
{
    printf("   i_number rec_len name_len    name\n");

    // Loop through the 12 direct blocks
    for (i = 0; i < 12; i++) {
        if (!inodeptr->i_block[i]) {
            // Get your current block into the buffer
            get_block(dev, inodeptr->i_block[0], buf);

            DIR *dp = (DIR *) buf;       // access buf[] as DIR entries
            char *cp = buf;              // char pointer pointing at buf[ ]

            while (cp < buf + BLKSIZE)       // Loop until out of the block
            {
                // Print out our information for each directory entry
                printf("%8d %8d %7d       %-s\n", dp->inode, dp->rec_len, dp->name_len, dp->name);

                // Return case : we located the directory ( by name ) that we were looking for
                if (strcmp(name, dp->name) == 0)
                    return dp->inode;

                cp += dp->rec_len;         // advance cp by rlen in bytes
                dp = (DIR *) cp;            // pull dp to the next DIR entry
            }
        }
    }

    return 0;
}

/**
 * Function: iget(int dev, int ino) : MINODE *
 * Description: Get a MINODE by checking if it is being referenced or not
 */
MINODE* iget(int dev, int ino)
{
    int blocknum, inodenum;
    INODE *ip;

    // Check if the inode number is already in the MINODE array (in the cache)
    for (i = 0; i < 100; i++)
    {
        // Does that minode exist in the minode array
        if (minode[i].ino == ino)
        {
            // Increment our reference count at that location and return
            minode[i].refCount++;
            return &minode[i];
        }
    }

    // In the case that it is not :
    // Use mailman's algorithm to get the block number and inode number
    // of the returned inumber from the search function
    inodenum = (ino - 1) % divisor;
    blocknum = ((ino - 1) / divisor) + inodebeginblock;

    // The item is located, get that block
    get_block(dev, blocknum, buf);
    ip = (INODE *)buf + inodenum;

    // Find somewhere to store the operation
    for (i = 0; i < 100; i++)
    {
        if (minode[i].refCount == 0) {
            // Move the inode into the table
            memcpy(&(minode[i].INODE), ip, sizeof(INODE));

            // Update the MINODE properties
            minode[i].refCount++;
            minode[i].dev = dev;
            minode[i].ino = ino;

            /** INODE PRINTS */
            printf("minode[i] dev: %d\n", minode[i].dev);
            printf("minode[i] ino: %d\n", minode[i].ino );
            printf("******************************************\n");

            // Now return this MINODE to the user
            return &minode[i];
        }
    }
}

/**
 * Function: iput(MINODE *mip) : int
 * Description: Releases a MINODE
 */
int iput(MINODE *mip)
{
    int inodenum, blocknum;

    /**
        This function releases a Minode[]. Since an Minode[]'s refCount indicates
        the number of users on this Minode[], releasing is done as follows:

        First, dec the refCount by 1.
        If (after dec) refCount > 0 ==> return;
        if Minode[].dirty == 0 ==> no need to write back, so return;
        --------------------------------------------------------------
                Otherwise, (dirty==1) ==> must write the INODE back to disk.
     */

    mip->refCount--;
    if (mip->refCount > 0 || mip->dirty == 0) return 0;
    else {
        // The MINODE is dirty, we must write back to this disk

        // Use mailman's algorithm to get the block number and inode number
        // of the returned inumber from the search function
        inodenum = (mip->ino - 1) % divisor;
        blocknum = ((mip->ino - 1) / divisor) + inodebeginblock;

        get_block(mip->dev, blocknum, buf);

        // Write the block back to the disk
        INODE *ip = (INODE*)buf + inodenum;
        memcpy(ip, &(mip->INODE), sizeof(INODE));

        put_block(mip->dev, blocknum, buf);
    }
}

/**
 * Function: ls(char *pathname) : void
 * Description: Gets the inode number and the MINODE from a path
 *              and lists the directories under that path
 */
void ls(char *pathname)
{
   // printf("pathname : %s\n", pathname);
    int ino, dev = running->cwd->dev;
    MINODE *mip = running->cwd;

    if (strcmp(pathname, "") != 0) {   // ls pathname:
        if (pathname[0]=='/')
            dev = root->dev;

        printf("pathname: %s\n", pathname);
        ino = getino(&dev, pathname);
        mip = iget(dev, ino);
    }

    //printf("mip->ino = %d\n", mip->ino);
    //printf("ino = %d\n", ino);

    // Check to ensure the pathname is a directory
    if (!S_ISDIR(mip->INODE.i_mode)) {
        printf("%s: Not a directory\n", pathname);
        return;
    }

    // Loop through the direct blocks
    for (i = 0; i < 12; i++) {
        if (mip->INODE.i_block[i] != 0) {
            // After ensure the block is not 0, get the block
            get_block(dev, mip->INODE.i_block[i], buf);

            DIR  *dp = (DIR *)buf;       // access buf[] as DIR entries
            char *cp = buf;              // char pointer pointing at buf[ ]

            while(cp < buf + BLKSIZE)       // Loop until out of the block
            {
                //ino = dp->inode;
                //mip = iget(dev, ino);

                // Print out our information for each directory entry
                printf("%s\n", dp->name);

                /*
                if (S_ISREG(mip->INODE.i_mode))     // is reg
                    printf("%c",'-');
                if (S_ISDIR(mip->INODE.i_mode))     // is dir
                    printf("%c",'d');
                //if ((mip->INODE.i_mode & 0xF000) == 0xA000)
                //    printf("%c",'l');

                for (i=8; i >= 0; i--){
                    if (mip->INODE.i_mode & (1 << i))
                        printf("%c", t1[i]);
                    else
                        printf("%c", t2[i]);
                }

                printf("%4d ",mip->INODE.i_links_count);
                printf("%4d ",mip->INODE.i_gid);
                printf("%4d ",mip->INODE.i_uid);

                strncpy(name, dp->name, dp->name_len);
                printf(" %s", name);
                bzero(name, sizeof(name));

                printf("%8d ",mip->INODE.i_size);

                // print time
                char ftime[64];
                strcpy(ftime, ctime(&mip->INODE.i_ctime));
                ftime[strlen(ftime)-1] = 0;
                printf("%s  ",ftime);
                printf("HERE!\n");
                */

                cp += dp->rec_len;         // advance cp by rlen in bytes
                dp = (DIR *)cp;            // pull dp to the next DIR entry
            }
        }
    }
}

/**
 * Function: cd(char *pathname) : void
 * Description: Changes the current directory to the directory at given pathname
 */
void cd(char *pathname)
{
    int ino, dev = running->cwd->dev;
    MINODE *mip;

    // Check if no pathname was specified; if so, change to root
    if (strcmp(pathname, "") == 0)
    {
        printf("Changed directory to root\n");
        running->cwd = root;
        return;
    }
    else if (path[0] == '/') {
        dev = root->dev;
    }

    // Get the inode number of the pathname, then set the cwd
    ino = getino(&dev, pathname);
    mip = iget(dev, ino);

    // Check if the minode is a directory
    if (S_ISDIR(mip->INODE.i_mode))
        running->cwd = mip;
    else
        printf("%s: Not a directory\n", pathname);
}

/**
 * Functions: tst_bit, set_bit, clr_bit(char *buf, int bit) : int
 * Description: Bit operations
 */
int tst_bit(char *buf, int bit)
{
    int i, j;
    i = bit/8; j=bit%8;
    if (buf[i] & (1 << j))
        return 1;
    return 0;
}

int set_bit(char *buf, int bit)
{
    int i, j;
    i = bit/8; j=bit%8;
    buf[i] |= (1 << j);
}

int clr_bit(char *buf, int bit)
{
    int i, j;
    i = bit/8; j=bit%8;
    buf[i] &= ~(1 << j);
}

/**
 * Function: decFreeInodes(int dev) : int
 * Description: Decrement the free inodes
 */
int decFreeInodes(int dev)
{
    char buf[BLKSIZE];

    // dec free inodes count in SUPER and GD
    get_block(dev, 1, buf);
    sp = (SUPER *)buf;
    sp->s_free_inodes_count--;
    put_block(dev, 1, buf);

    get_block(dev, 2, buf);
    gp = (GD *)buf;
    gp->bg_free_inodes_count--;
    put_block(dev, 2, buf);
}

/**
 * Function: ialloc(int dev) : int
 * Description: Allocates for an inode
 */
int ialloc(int dev)
{
    int  i;
    char buf[BLKSIZE];

    // read inode_bitmap block
    get_block(dev, imap, buf);

    for (i=0; i < ninodes; i++){
        if (tst_bit(buf, i)==0){
            set_bit(buf,i);
            decFreeInodes(dev);

            put_block(dev, imap, buf);

            return i+1;
        }
    }
    printf("ialloc(): no more free inodes\n");
    return 0;
}

/**
 * Function: balloc(int dev) : int
 * Description: Allocates for an block node
 */
int balloc(int dev)
{
    int  i;
    char buf[BLKSIZE];

    // read inode_bitmap block
    get_block(dev, bmap, buf);

    for (i=0; i < ninodes; i++){
        if (tst_bit(buf, i)==0){
            set_bit(buf,i);
            decFreeInodes(dev);

            put_block(dev, bmap, buf);

            return i+1;
        }
    }
    printf("balloc(): no more free inodes\n");
    return 0;
}

main()
{
    // Initialize with mounting the root
    init();
    mount_root();

    // Get the username
    printf("Enter username : ");
    scanf(" %[^\n]s", usrname);

    while (1)
        shell();
    //printf("result: %d\n", getino(&dev, "/X/"));

    return;
}
