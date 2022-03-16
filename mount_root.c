/******************************** mount_root.h ************************/
#include"type.h"
#include"util.h"
#include<stdio.h>
#include<stdlib.h>
#include<ext2fs/ext2_fs.h>

extern MINODE *root;
//extern int ninodes,nblocks,bmap;
extern int dev;
extern PROC proc[NPROC];
extern int ninodes;
extern int nblocks;
extern int bmap,imap,iblock;
extern MTABLE mtable[NMTABLE];

int mount_root(char *rootdev){		// mount root file system
	int i;
	MTABLE *mp;
	SUPER *sp;
	GD *gp;
	char buf[BLKSIZE];

	dev = open(rootdev,O_RDWR);
	if(dev < 0){
		printf("panic: can't open root device\n");
		exit(1);
	}

	//get super block of rootdev
	get_block(dev,1,buf);
	sp = (SUPER*)buf;
	//check magic number
	if(sp->s_magic != 0xEF53){
		printf("super magic = %x: %s is not an EXT2 filesys\n",sp->s_magic,rootdev);
		exit(0);
	}
	//file mount table mtable[0] with rootdev information
	mp = &mtable[0];	//use mtable[0]
	mp->dev = dev;
	//copy super block info into mtable[0]
	ninodes = mp->ninodes = sp->s_inodes_count;
	nblocks = mp->nblocks = sp->s_blocks_count;
	strcpy(mp->devName,rootdev);
	strcpy(mp->mntName,"/");
	get_block(dev,2,buf);
	gp = (GD *)buf;
	bmap = mp->bmap = gp->bg_block_bitmap;
	imap = mp->imap = gp->bg_inode_bitmap;
	iblock = mp->iblock = gp->bg_inode_table;
	printf("bmap = %d imap = %d iblock = %d\n",bmap,imap,iblock);

	//call iget(),which inc minode's refCount
	root = iget(dev,2);
	//printf(" root ino :%d\n",root->ino);
	mp->mntDirPtr = root;
	root->mntPtr = mp;
	//set proc CWDs
	for(i = 0;i<NPROC;i++)
		proc[i].cwd = iget(dev,2);
	printf("mount: %s mounted on / \n",rootdev);
	return 0;
}
