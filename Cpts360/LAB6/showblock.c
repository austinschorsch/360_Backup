/**
 * Author : Austin Schorsch
 * Date   : 11/1/15
 * Description : Lab 06, Cpts 360 - Inode Introduction
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include <sys/stat.h>
#include <ext2fs/ext2_fs.h>

typedef unsigned int   u32;

// define shorter TYPES, save typing efforts
typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;    // need this for new version of e2fs

GD    *gp;
SUPER *sp;
INODE *ip;
DIR   *dp;

#define BLKSIZE 1024

// Global Variables
int fd, pathcount, divisor;             // divisor = BLKSIZE / sp->s_inode_size
int inodebeginblock;

int i, j;
unsigned int buf1[256];
unsigned int buf2[256];

char buf[1024];
char path[128], *pathnames[64];

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
 *
    The layout of an EXT2 file system on a virtual disk image is

     1KB      0    1    2    3    4  | 5 . ........... 27| 28 ..................|
    BLOCK: |Boot|Super| Gd |Bmap|Imap|Inodes blocks .....|....  data blocks ....|
                                     |    INODEs         |
                                  ino|1,2,3,4 .......... |
 *
 * Function: super() : void
 * Description: Reads in the superblock to ensure that we have an ext2 filesystem
 */
void super()
{
    // read SUPER block
    get_block(fd, 1, buf);
    sp = (SUPER *)buf;

    printf("************  super block info: *************\n");

    // Ensure that the file type is an ext2 filesystem
    // by checking for the magic number (EF53)
    if (sp->s_magic != 0xEF53){
        printf("Usage : show  /dev/XYZ  pathname\n");
        exit(1);
    }

    printf("inode_count\t\t\t%d\n", sp->s_inodes_count);
    printf("blocks_count\t\t\t%d\n", sp->s_blocks_count);
    printf("r_blocks_count\t\t\t%d\n", sp->s_r_blocks_count);
    printf("free_blocks_count\t\t%d\n", sp->s_free_blocks_count);
    printf("free_inodes_count\t\t%d\n", sp->s_free_inodes_count);
    printf("log_blk_size\t\t\t%d\n", sp->s_log_block_size);
    printf("first_data_block\t\t%d\n", sp->s_first_data_block);

    printf("magic number = %x\n", sp->s_magic);

    printf("rev_level\t\t\t%d\n", sp->s_rev_level);
    printf("inode_size\t\t\t%d\n", sp->s_inode_size);
    printf("block_group_nr\t\t\t0\n");
    printf("blksize\t\t\t\t1024\n");
    printf("inode_per_group\t\t\t%d\n", sp->s_inodes_per_group);

    printf("---------------------------------------------\n");
    printf("desc_per_block\t\t\t%d\n", sp->s_inode_size / 4);
    printf("inodes_per_block\t\t%d\n", BLKSIZE/sp->s_inode_size);
    printf("inode_size_ratio\t\t%d\n", 1);

    // Set divisor
    divisor = BLKSIZE/sp->s_inode_size;
}

/**
 * Function: groupblock() : void
 * Description: This function displays the block information
 * in the group descriptor block
 */
void groupblock()
{
    // read GD block
    get_block(fd, 2, buf);
    gp = (GD *)buf;

    printf("************  group 0 info ******************\n");

    printf("Blocks bitmap block\t\t%d\n", gp->bg_block_bitmap);
    printf("Inodes bitmap block\t\t%d\n", gp->bg_inode_bitmap);
    printf("Inodes table block\t\t%d\n", gp->bg_inode_table);
    printf("Free blocks count\t\t%d\n", gp->bg_free_blocks_count);
    printf("Free inodes count\t\t%d\n", gp->bg_free_inodes_count);
    printf("Directories count\t\t%d\n", gp->bg_used_dirs_count);

    // Get inode start block number
    inodebeginblock = gp->bg_inode_table;
    printf("inodes start\t\t\t%d\n", inodebeginblock);
}

/**
 * Function: rootinfo() : void
 * Description: This function displays the information for the root inode
 */
void rootinfo()
{

    printf("***********  root inode info ***************\n");

    // get inode start block
    get_block(fd, inodebeginblock, buf);
    ip = (INODE *)buf + 1;         // ip points at 2nd INODE

    printf("File mode\t\t\t%4x\n", ip->i_mode);
    printf("Size in bytes\t\t\t%d\n", ip->i_size);
    printf("Blocks count\t\t\t%d\n", ip->i_blocks);
}

/**
 * Function: parse_path(char *path) : void
 * Description: This function takes the pathname entered by the user
 *              and parses it into pathnames
 */
void parse_path(char *path)
{
    char *token;
    int i = 0;

    // Ensure that we start one value over from the root
    token = strtok(&path[1], "/");
    while (token)
    {
        pathnames[i] = token;

        token = strtok(NULL, "/");
        i++;
    } pathcount = i;
}

/**
 * Function: search(INODE *inodeptr, char *name) : int
 * Description: Searches a given inode for a name (most likely referencing a DIR)
 *              and either:
 *                          (a) returns that DIR's inumber
 *                          (b) DIR doesn't exist, return 0
 */
int search(INODE *inodeptr, char *name)
{
    printf("   i_number rec_len name_len    name\n");

    // Get your current block into the buffer
    get_block(fd, inodeptr->i_block[0], buf);

    DIR  *dp = (DIR *)buf;       // access buf[] as DIR entries
    char *cp = buf;              // char pointer pointing at buf[ ]

    while(cp < buf + BLKSIZE)       // Loop until out of the block
    {
        // Print out our information for each directory entry
        printf("%8d %8d %7d       %-s\n", dp->inode, dp->rec_len, dp->name_len, dp->name);

        // Return case : we located the directory ( by name ) that we were looking for
        if (strcmp(name, dp->name) == 0)
            return dp->inode;

        cp += dp->rec_len;         // advance cp by rlen in bytes
        dp = (DIR *)cp;            // pull dp to the next DIR entry
    }

    return 0;
}

/** HELPER FUNCTIONS **/
void displayInitial()
{
    super();
    groupblock();
    rootinfo();
}

void wait()
{
    printf("Hit any key to continue : ");
    getchar();
}
/** END HELPER FUNCTIONS **/

/**
 * Function: main(int argc, char *argv[])
 */
char *device = "my disk";
main(int argc, char *argv[])
{
    /**
     * Check to make sure arguments were passed in.
     * Assuming they were, set device to the first argument then
     * open the file for read
     */
    if (argc > 2) {
        // Set the device and the path variables
        device = argv[1];
        strcpy(path, argv[2]);
    }

    // Send the path to be parsed
    parse_path(path);

    fd = open(device, O_RDONLY);
    if (fd < 0) {
        printf("Usage : show  /dev/XYZ  pathname\n");
        exit(1);
    }

    // Display the basic info: pathcount and pathnames
    printf("n = %d  ", pathcount);
    for (i = 0; i < pathcount; i++)
    {
        printf("%s ", pathnames[i]);
    } putchar('\n');

    /**
     * The next steps are as follows ( via displayInitial ):
     *      (1) Call super to display super block info
     *      (2) Call groupblock to display group descriptor info
     *      (3) Call rootinfo to display the info of the root inode
     *
     *      Then, wait for the user to move on ...
     */
    displayInitial();
    wait();

    /****************************************************
    * Print out Root dir information before searching
    ****************************************************/
    printf("block[0] = %d\n", ip->i_block[0]);
    printf("********* root dir entries ***********\n");
    printf("block = %d\n", ip->i_block[0]);
    printf("   i_number rec_len name_len    name\n");

    // Get your root block into the buffer
    get_block(fd, ip->i_block[0], buf);

    DIR  *dp = (DIR *)buf;       // access buf[] as DIR entries
    char *cp = buf;            // char pointer pointing at buf[ ]
    int blocknum, inodenum;

    // Root Info loop -- displays root dir entries
    while(cp < buf + BLKSIZE)
    {
        // Print out our info
        printf("%8d %8d %7d       %-s\n", dp->inode, dp->rec_len, dp->name_len, dp->name);

        cp += dp->rec_len;         // advance cp by rlen in bytes
        dp = (DIR *)cp;       // pull dp to the next DIR entry
    }
    /*****************************************************
    * Finished root inode info
    *****************************************************/

    wait();

    // Reset our block
    get_block(fd, inodebeginblock, buf);
    ip = (INODE *)buf + 1;
    printf("mode=%4x uid=%d gid=%d\n", ip->i_mode, ip->i_uid, ip->i_gid);

    // Begin search process for pathname
    int inumber = 0;
    for (i = 0; i < pathcount; i++)
    {
        printf("===========================================\n");
        printf("i=%d name[%d]=%s\n", i, i, pathnames[i]);
        printf("search for %s\n", pathnames[i]);
        printf("block[0]=%d\n\n", ip->i_block[0]);

        inumber = search(ip, pathnames[i]);

        // Check if the DIR was found
        if (inumber == 0) {
            printf("name %s does not exist\n", pathnames[i]);
        }
        else {
            /**
             *  Display information
             *      found [name] : ino = [num]
             *      group=0 inumber=13
             *      blk=11   disp=5
             */
            printf("\nfound %s : ino = %d\n", pathnames[i], inumber);
            printf("group=%d inumber=%d\n", ip->i_gid, inumber - 1);

            // Use mailman's algorithm to get the block number and inode number
            // of the returned inumber from the search function
            inodenum = (inumber - 1) % divisor;
            blocknum = ((inumber - 1) / divisor) + inodebeginblock;

            printf("blk=%d disp=%d\n", blocknum, inodenum);

            // Make sure we are not trying to go into a regular file
            if (S_ISREG(ip->i_mode) && i < pathcount - 1) {
                printf("%s is not a directory. Cannot go further\n");
                break;
            }

            // Update the block and the inode
            get_block(fd, blocknum, buf);
            ip = (INODE *)buf + inodenum;

            getchar();
        }
    } // End Search functionality

    /**
     *  Begin displaying the blocks
     **************************************
     * The fields i_block[15] record the disk blocks (numbers) of a file, which are

            DIRECT blocks : i_block[0] to i_block[11], which point to direct blocks.
            INDIRECT block: I_block[12] points to a block, which contians 256 (ulong)
                            block numbers.
            DOUBLE INDIRECT block:
                            I_block[13] points to a block, which points to 256 blocks,
                            each of which point to 256 blocks.
            TRIPLE INDIRECT block:
                            I_block[14] points to a block, which points to 256 blocks,
                            each of which points to 256 blocks, each of which points to
                            256 blocks.

      */

    int blocks = ceil(ip->i_size / 1024.0);
    printf("\nsize=%d blocks=%d\n", ip->i_size, blocks);

    printf("****************  DISK BLOCKS  *******************\n");

        /**
          * There are up to 15 Disk blocks, loop through them
          * Note: block[12] -> indirect
          *       block[13] -> double indirect
          */
        for(i = 0; i < 15; i++)
        {
            // Ensure that the block is not zero
            if(ip->i_block[i] != 0)
            {
                printf("i_block[%d] = %d\n", i, ip->i_block[i]);
            }
        }

    printf("\n================ Direct Blocks ===================\n");

        /**
         * Loop through the 12 direct blocks,
         * At the end, decrement remaining blocks
         */
        for (i = 0; i < 12; i++)
        {
            // Ensure that the block is not zero
            if (ip->i_block[i] != 0)
            {
                // Tab check
                if(i == 9) putchar('\n');

                printf("%d ", ip->i_block[i]);
            }
        }

        // Update remaining blocks
        if (blocks > 12) blocks -= 12;
        else blocks = 0;

        printf("\nblocks remain = %d", blocks);

    // Check for Indirect block ( located in i_block[12] )
    if (ip->i_block[12])
    {
        printf("\n===============  Indirect blocks   ===============\n");

        // Get our 12th block (which holds 256 blocks) and
        // display them as indirects
        get_block(fd, ip->i_block[12], (char*)buf1);

        // Each indirect block potentially has 256 blocks, loop through them
        // while checking to see if they still exist
        for (i = 0; i < 256; i++)
        {
            // Ensure the buffer is not referring to 0, if so, print
            if (buf1[i])
            {
                // Tab block
                if ((i != 0) && (i % 10 == 0)) putchar('\n');

                printf("%d ", buf1[i]);
            }
        }

        // Update remaining blocks
        if (blocks > 256) blocks -= 256;
        else blocks = 0;

        printf("\nblocks remain = %d", blocks);
    }
    // Check for Double Indirect block ( located in i_block[13] )
    if (ip->i_block[13])
    {
        printf("\n===========  Double Indirect blocks   ============\n");

        // Get our 13th block (which holds 256 blocks) and
        // display them as indirects
        printf("%d\n*******************************************\n", ip->i_block[13]);

        // Get our block into our first buffer
        get_block(fd, ip->i_block[13], (char*)buf1);

        /**
         * Description: Loop through each blocks indirect block (each has 256)
         *              However, we must loop through the 256 indirect blocks already
         *              (hence double indirect).
         */
        for (i = 0; i < 256; i++)
        {
            /**
             * For info on inode pointer structure:
             *      https://en.wikipedia.org/wiki/Inode_pointer_structure
             */
            if (buf1[i])
            {
                // Get the block for the current block (double indirect)
                get_block(fd, buf1[i], (char*)buf2);
                for (j = 0; j < 256; j++)
                {
                    if (buf2[j])
                    {
                        if ((j != 0) && (j % 10 == 0)) putchar('\n');

                        printf("%d ", buf2[j]);
                    }
                }
            }
        }

        printf("\nblocks remain = 0\n\n");
    }
}


