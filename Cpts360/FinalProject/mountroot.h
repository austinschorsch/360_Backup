//
// Created by aschorsch on 11/13/15.
//

#ifndef FINALPROJECT_MOUNTROOT_H
#define FINALPROJECT_MOUNTROOT_H

#include "type.h"

int get_block(int dev, int blk, char *buf);
int put_block(int dev, int blk, char *buf);
void shell();
void reset();
void quit();
void init();
int get_block(int fd, int blk, char buf[BLKSIZE]);
int put_block(int fd, int blk, char buf[ ]);
void mount_root();
void tokenize(char *path);
int getino(int *dev, char *pathname);
int search(int dev, INODE *inodeptr, char *name);
MINODE* iget(int dev, int ino);
int iput(MINODE *mip);
void ls(char *pathname);
void cd(char *pathname);
int tst_bit(char *buf, int bit);
int set_bit(char *buf, int bit);
int clr_bit(char *buf, int bit);
int decFreeInodes(int dev);
int ialloc(int dev);
int balloc(int dev);


#endif //FINALPROJECT_MOUNTROOT_H
