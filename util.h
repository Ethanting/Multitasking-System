/****************** util.h file ******************/
#ifndef UTIL_H
#define UTIL_H
#include"type.h"

int get_block(int dev,int blk, char *buf);

int put_block(int dev,int blk,char *buf);

MINODE *iget(int dev,int ino);

int iput(MINODE *mip);

int tokenize(char *pathname);

int search(MINODE *mip,char *name);

int getino(char *pathname);

MINODE *mialloc();

int midalloc(MINODE *mip);

void findmyname(MINODE *parent , u32 myino, char *myname);

int tst_bit(char *buf,int bit);

int set_bit(char *buf,int bit);

int decFreeInode(int idev);

int decFreeBlock(int idev);

int ialloc(int idev);

int balloc(int idev);

int enter_name(MINODE *parent,int inumber,char *basename);

int clr_bit(char *buf,int bit);

int incFreeInodes(int dev);

int idalloc(int dev,int ino);

int incFreeBlocks(int dev);

int bdalloc(int dev,int blk);

int last_entry(char *buf,int del_len);

int rm_child(MINODE *pmip,char *myname);

char * basename(char *pathname);

char * dirname(char *pathname);

#endif	//UTIL_H
