/////////////////////////// type.h file/////////////////////////////////////////////
#ifndef TYPE_H
#define TYPE_H
#include<fcntl.h>
#include<ext2fs/ext2_fs.h>
#include<libgen.h>
#include<string.h>
#include<sys/stat.h>

typedef unsigned int u32;

// define shorter TYPES for convenience
typedef struct ext2_group_desc GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode INODE;
typedef struct ext2_dir_entry_2 DIR;
#define BLKSIZE 1024

// Block number of EXT2 FS on FD
#define SUPERBLOCK 1
#define GDBLOCK 2
#define ROOT_INODE 3

// Default dir and regular file modes
#define DIR_MODE        0x41ED
#define FILE_MODE       0x81AE
#define SUPER_MAGIC     0xEF53
#define SUPER_USER      0

// file system table sizes
#define NMINODE         100
#define NMTABLE         10
#define NFD             10
#define NOFT            40
#define NPROC 10
#define SSIZE 1024

// PROC status
#define FREE 0
#define READY 1
#define SLEEP 2
#define ZOMBIE 3

//In-memory inodoes structures
typedef struct minode{
        INODE inode;    //disk inode
        int dev,ino;
        int refCount;
        int dirty;
        int mounted;
        struct mount *mntPtr;
        //int lock;
}MINODE;


//open File Table
typedef struct oft{
        int mode;
        int refCount;
        struct minode *minodePtr;
        int offset;
}OFT;

// Mount Table structure
typedef struct mtable{
        int dev;
        int ninodes;
        int nblocks;
        int free_blocks;
        int free_inodes;
        int bmap;
        int imap;
        int iblock;
        MINODE *mntDirPtr;
        char devName[64];
        char mntName[64];
}MTABLE;

//PROC structure
typedef struct proc{
        struct proc *next;              //next proc pointer
        int *ksp;                       //saved stack pointer
        int pid;                        //pid = 0 to NPROC-1
	int uid;
	int gid;
        int ppid;                       //parent pid
        int status;                     //PROC status
	int event;
	int exitStatus;			//0 for normal,-1 for error
        int priority;                   //scheduling priority
        struct proc *child;
        struct proc *sibling;
        struct proc *parent;
        int kstack[SSIZE];              //process stack
	struct minode *cwd;
	int icwd;
	OFT *fd[NFD];
}PROC;

#endif
