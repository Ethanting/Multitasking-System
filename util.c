/****************** util.c file ******************/
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include"type.h"
#include"util.h"

extern char *name[64];
extern gline[256];
extern int nname;
extern int dev;
extern MINODE *root;
extern PROC *running;
extern int iblock;
extern MINODE minode[NMINODE];
extern int ninodes,nblocks,bmap,imap,iblock;

MINODE * mialloc();

//get or put the content of block from dev
int get_block(int idev,int blk, char *buf){
	lseek(idev,blk*BLKSIZE,SEEK_SET);
	int n = read(dev,buf,BLKSIZE);
	if(n<0)
		printf("get_block [%d %d] error\n",dev,blk);
}

int put_block(int dev,int blk,char *buf){
	lseek(dev,blk*BLKSIZE,SEEK_SET);
	int n = write(dev,buf,BLKSIZE);
	if(n != BLKSIZE)
		printf("put_block [%d %d] error\n",dev,blk);
}

//get and put ino from dev and ino into minode
MINODE *iget(int idev,int ino){
	MINODE *mip;
	MTABLE *mp;
	INODE *ip;
	int i,block,offset;
	char buf[BLKSIZE];

	//search in-memory minodes first
	for(i = 0;i<NMINODE;i++){
		mip = &minode[i];
		if(mip->refCount && (mip->dev == idev) && (mip->ino == ino)){
			mip->refCount++;
			return mip;
		}
	}

	// needed INODE = (dev,ino) not in memory
	mip = mialloc();		//allocate a FREE minode
	mip->dev = idev;
	mip->ino = ino;
	block = (ino-1)/8 + iblock;
	offset = (ino-1)%8;
	get_block(idev,block,buf);
	ip = (INODE*)buf + offset;
	mip->inode = *ip;
	//initialize minode
	mip->refCount = 1;
	mip->mounted = 0;
	mip->dirty = 0;
	mip->mntPtr = 0;
	return mip;
}

int iput(MINODE *mip){
	INODE *ip;
	int i,block,offset;
	char buf[BLKSIZE];

	if(mip == 0) return ;
	mip->refCount--;		// dec refCount by 1
	if(mip->refCount > 0) return;	// still has user
	if(mip->dirty == 0) return;	// no need to write back

	// write INODE back to disk
	block = (mip->ino - 1)/8 + iblock;// ? iblock :disk block containing this inode
	offset = (mip->ino -1)%8;

	// get block containing this inode
	get_block(mip->dev,block,buf);
	ip = (INODE *)buf + offset;
	*ip = mip->inode;
	put_block(mip->dev,block,buf);
	midalloc(mip);
}

//get ino from pathname
int tokenize(char *pathname){
	char *s;
	strcpy(gline,pathname);
	nname = 0;
	s = strtok(gline,"/");
	while(s){
		name[nname++] = s;
		s = strtok(0,"/");
	}
}

int search(MINODE *mip,char *myname){
	int i;
	char *cp,temp[256],sbuf[BLKSIZE];
	DIR *dp;
	for(i = 0;i<12;i++){
		if(mip->inode.i_block[i] == 0)
			return 0;
		get_block(mip->dev,mip->inode.i_block[i],sbuf);
		dp = (DIR*)sbuf;
		cp = sbuf;
		while(cp < sbuf + BLKSIZE){
			strncpy(temp,dp->name,dp->name_len);
			temp[dp->name_len] = 0;
			printf("%8d%8d%8u %s\n",dp->inode,dp->rec_len,dp->name_len,temp);
			if(strcmp(myname,temp) == 0){
				printf("found %s : inumber = %d\n",myname,dp->inode);
				return dp->inode;
			}
			cp += dp->rec_len;
			dp = (DIR*)cp;
		}
	}
	return 0;
}

int getino(char *pathname){
	MINODE *mip;
	int i,ino;
	if(pathname == NULL)
		return 0;
	if(strcmp(pathname,"/") == 0){
		return 2;	// return root ino = 2
	}
	if(pathname[0] == '/')
		mip = root;
	else
		mip = running->cwd;
	mip->refCount++;

	tokenize(pathname);

	for(i = 0;i<nname;i++){
		if(!S_ISDIR(mip->inode.i_mode)){
			printf("%s is not a directory\n",name[i]);
			iput(mip);
			return 0;
		}
		ino = search(mip,name[i]);
		if(!ino){
			printf("no such component name %s\n",name[i]);
			iput(mip);
			return 0;
		}
		iput(mip);
		mip = iget(dev,ino);
	}
	iput(mip);
	return ino;
}

//allocate and release minode
MINODE* mialloc(){              // allocate a FREE minode for use
        int i;
	MINODE *mp;
        for(i = 0;i<NMINODE;i++){
                mp = &minode[i];
                if(mp->refCount == 0){
                        mp->refCount = 1;
                        return mp;
                }
        }
        printf("FS panic: out of minodes\n");
        return 0;
}

int midalloc(MINODE *mip)       //release a used minode
{
        mip->refCount = 0;
}

//find myname which ino is myino from parent
void findmyname(MINODE *parent , u32 myino, char *myname){
        int i;
        char *cp; 
        //char temp[256];
        char sbuf[BLKSIZE];
        DIR *dp; 
        for(i = 0;i<12;i++){
                if(parent->inode.i_block[i] == 0) return 0;
                get_block(parent->dev,parent->inode.i_block[i],sbuf);
                dp = (DIR*)sbuf;
                cp = sbuf;
                while(cp < sbuf + BLKSIZE){
                        if(dp->inode == myino){
                                strncpy(myname,dp->name,dp->name_len);
                                myname[dp->name_len] = 0; 
                                //temp[dp->name_len] = 0;
                        }
                        cp += dp->rec_len;
                        dp = (DIR*)cp;
                }
        }
        return 0;
}

//allocate and free block of the block
//tst_bit,set_bit functions
int tst_bit(char *buf,int bit){
        return buf[bit/8] & (1 << (bit %8));
}

int set_bit(char *buf,int bit){
        return buf[bit/8] |= (1 << (bit%8));
}

int decFreeInode(int idev){
        char buf[BLKSIZE];
        SUPER *sp; 
        GD *gp; 
        //dec free inodes count in SUPER and GD
        get_block(idev,1,buf);
        sp = (SUPER*)buf;
        sp->s_free_inodes_count--;
        put_block(idev,1,buf);
        get_block(idev,2,buf);
        gp = (GD*)buf;
        gp->bg_free_inodes_count--;
        put_block(idev,2,buf);
}

int decFreeBlock(int idev){
        char buf[BLKSIZE];
        SUPER *sp; 
        GD *gp; 
        get_block(idev,1,buf);
        sp->s_free_blocks_count--;
        put_block(idev,1,buf);
        get_block(idev,2,buf);
        gp = (GD*)buf;
        gp->bg_free_blocks_count--;
        put_block(idev,2,buf);
}

int ialloc(int idev){
        int i;
        int j;
        char buf[BLKSIZE];
        //use imap, niodes in mount table of dev
        //MTABLE *mp = (MTABLE *)get_mtable(idev);
        get_block(idev,imap,buf);
        for(i = 0;i< ninodes;i++){
                j = tst_bit(buf,i);
                if(j == 0){
                        set_bit(buf,i);
                        put_block(dev,imap,buf);
                        //update free inode count in SUPER and GD
                        decFreeInode(idev);
                        return i+1;
                }
        }
        return 0;       //out of FREE inodes
}

int balloc(int idev){
        int i;
        char buf[BLKSIZE];
        //MTABLE *mp = (MTABLE*)get_mtable(idev);
        get_block(idev,bmap,buf);
        for(i = 0;i< nblocks;i++){
                if(tst_bit(buf,i) == 0){
                        set_bit(buf,i);
                        put_block(dev,bmap,buf);
                        decFreeBlock(idev);
                        return i+1;
                }
        }
        return 0;

}

//enter the basename into parent which ino is inumber;
int enter_name(MINODE *parent,int inumber,char *basename){
        int i,j,size,found,count,blk;
        short int ideal_len,need_len,remain;
        DIR *dp;
        char buf[BLKSIZE],temp[256],*cp,c;
        MINODE *mip = parent;
        char ttt[256];
        DIR *ddp;

        size = parent->inode.i_size;
        found = 0;
        count = 0;
        need_len = ((strlen(basename)+8+3)/4)*4;
        printf("need_len for %s = %d\n",basename,need_len);

        // parent dir has up to 12 direct blocks
        for(i = 0;i<12;i++){
                blk = parent->inode.i_block[i];
                if(blk == 0){
                        blk = parent->inode.i_block[i] = balloc(parent->dev);
                        printf("enter_name: allocate a new data block = %d\n",blk);
                        parent->inode.i_size += BLKSIZE;
                        memset(buf,0,1024);
                        dp = (DIR*)buf;
                        dp->inode = inumber;
                        dp->name_len = strlen(basename);
                        dp->rec_len = BLKSIZE;
                        strncpy(dp->name,basename,strlen(basename));
                        break;
                }
		
		printf("parent data blk[%d] = %d\n",i,parent->inode.i_block[i]);
                get_block(parent->dev,parent->inode.i_block[i],buf);

                dp = (DIR*)buf;
                cp = buf;

                while(cp < buf + BLKSIZE){
                        c = dp->name[dp->name_len];
                        dp->name[dp->name_len] = 0;

                        printf("[%d %s] ",dp->rec_len,dp->name);
                        dp->name[dp->name_len] = c;

                        cp += dp->rec_len;
                        dp = (DIR*)cp;
                }
                printf("\n");

                printf("step to LAST entry in data block\n");

                dp = (DIR*)buf;
                cp = buf;
                while(cp + dp->rec_len < buf + BLKSIZE){
                        cp += dp->rec_len;
                        dp = (DIR*)cp;
                }
                printf("\n");
		
		ideal_len = ((8+dp->name_len+3)/4)*4;
                printf("ideal_len = %d rec_len = %d\n",ideal_len,dp->rec_len);
                remain = dp->rec_len - ideal_len;
                if(remain >= need_len){
                        dp->rec_len = ideal_len;
                        cp += dp->rec_len;

                        dp = (DIR*)cp;
                        dp->inode = inumber;
                        dp->name_len = strlen(basename);
                        dp->rec_len = remain;
                        strncpy(dp->name,basename,strlen(basename));
                }
                break;
        }
        printf("writing parent data block back %d %d\n",i,parent->inode.i_block[i]);
        put_block(parent->dev,parent->inode.i_block[i],buf);

        // touch up parent
        parent->inode.i_mtime = parent->inode.i_atime = time(0L);
        parent->dirty = 1;
        printf("----------------- verify results ---------------------\n");
        search(parent,basename);
        return 0;
}
//allocate and ballocate minode of inode
int clr_bit(char *buf,int bit){
        buf[bit/8] &= ~(1<<(bit%8));
}

int incFreeInodes(int dev){
        SUPER *sp;
        GD *gp;
        char buf[BLKSIZE];
        get_block(dev,1,buf);
        sp = (SUPER*)buf;
        sp->s_free_inodes_count++;
        put_block(dev,1,buf);
        get_block(dev,2,buf);
        gp = (GD*)buf;
        gp->bg_free_inodes_count++;
        put_block(dev,2,buf);
}

int idalloc(int dev,int ino){
        int i;
        char buf[BLKSIZE];
        if(ino > ninodes){      // ninodes global
                printf("inumber %d out of range\n",ino);
                return 0;
        }
        get_block(dev,imap,buf);
        clr_bit(buf,ino-1);
        put_block(dev,imap,buf);
        incFreeInodes(dev);
}

int incFreeBlocks(int dev){
        SUPER *sp;
        GD *gp;
        char buf[BLKSIZE];
        get_block(dev,1,buf);
        sp = (SUPER*)buf;
        sp->s_free_blocks_count++;
        put_block(dev,1,buf);
        get_block(dev,2,buf);
        gp = (GD*)buf;
        gp->bg_free_inodes_count++;
        put_block(dev,2,buf);
}

//?
int bdalloc(int dev,int blk){
        int i;
        char buf[BLKSIZE];
        if(blk > nblocks){
                printf("iblock %d out of range\n",blk);
                return 0;
        }
        get_block(dev,bmap,buf);
        clr_bit(buf,blk-1);
        put_block(dev,bmap,buf);
        incFreeBlocks(dev);
}

//rm myname in parent minode
int last_entry(char *buf,int del_len){
        char *cp = buf;
        DIR *dp = (DIR*)cp;

        while(cp + dp->rec_len < buf + BLKSIZE){
                cp += dp->rec_len;
                dp = (DIR *)cp;
        }
        printf("last entrt=[%d %d]",dp->inode,dp->rec_len);
        dp->rec_len += del_len;
        printf("last entry=[%d %d]\n",dp->inode,dp->rec_len);
        return 1;
}

int rm_child(MINODE *pmip,char *myname){
        int i,j,del_len;
        DIR *dp,*pp;
        char *cp,*cq,c;
        char buf[BLKSIZE];

        for(i=0;i<12;i++){
                if(pmip->inode.i_block[i] == 0)
                        break;

                get_block(pmip->dev,pmip->inode.i_block[i],buf);
                pp = dp = (DIR*)buf;
                cp = buf;

                while(cp < buf + BLKSIZE){
                        c = dp->name[dp->name_len] = 0;
                        dp->name[dp->name_len] = 0;

                        if(strcmp(dp->name,myname) == 0){
                                dp->name[dp->name_len] = c;
                                del_len = dp->rec_len;

                                // if first and only entry in block
                                if(pp == dp && dp->rec_len == BLKSIZE){
                                        printf("ONLY entry in block: dp rec_len= %d\n",dp->rec_len);
                                        printf("dealocate %d: blk=%d\n",i,pmip->inode.i_block[i]);
                                        bdalloc(pmip->dev,pmip->inode.i_block[i]);
                                        pmip->inode.i_size -= BLKSIZE;
                                        pmip->dirty = 1;
                                        for(j = i+1;j<12;j++)
                                                pmip->inode.i_block[j-1] = pmip->inode.i_block[j];
                                        return 1;
                                }
				
				//if last entry in block: absorb rec_len into predecessor
                                cq = (char *)dp;
                                if(cq + dp->rec_len >= buf + BLKSIZE){
                                        pp->rec_len += dp->rec_len;
                                }
                                else if(dp->rec_len != BLKSIZE){
                                        last_entry(buf,del_len);
                                        memcpy(cp,cp + dp->rec_len,(&buf[BLKSIZE]-cp));
                                }
                                put_block(dev,pmip->inode.i_block[i],buf);
                                return 1;
                        }
                        dp->name[dp->name_len] = c;
                        pp = dp;
                        cp += dp->rec_len;
                        dp = (DIR*)cp;
                }
                put_block(dev,pmip->inode.i_block[i],buf);
        }
        return -1;
}

//get dirname and basename from the pathname
char * basename(char *pathname){
        int i,j,m;
        //char dname[256];
        char cname[256];
        char *dname;
        dname = malloc(256*sizeof(char));
        char c;
        if(pathname[0] == 0)    return NULL;
        if(pathname[0] != '/'){
                dname[0] = pathname[0];
                i = 1;
                c = pathname[0];
                while(c != '\0'){
                        dname[i] = pathname[i];
                        i++;
                        c = pathname[i];
                }
                return dname;
        }

        else{
                cname[0] = pathname[0];
                j = 0;
                i = 1;
                c = pathname[1];
		while(c != '/'){
                        while(c != '\0' && c != '/'){
                                //cname[i] = pathname[i];
                                i++;
                                c = pathname[i];
                        }
                        cname[i] = pathname[i];
                        if(c == '/')
                                j = i;
                        if(c == '\0'){
                                m = 0;
                                while(i != j){ 
                                        dname[m] = pathname[j+1];
                                        m++;
                                        j++;
                                }
                                break;
                        }
                        i++;
                        c = pathname[i];
                }
        }
        return dname;
}

char * dirname(char *pathname){
        int i,j;
        //char dname[256];
        char cname[256];
        char *dname;
        dname = malloc(256*sizeof(char));
        char c;
        if(pathname[0] == 0)    return NULL;
        if(pathname[0] != '/'){
                return NULL;
        }

        else{
                cname[0] = pathname[0];
                dname[0] = pathname[0];
                i = 1;
                c = pathname[1];
                while(c != '/'){
                        /*
                        dname[i] = pathname[i];
                        i++;
                        c = pathname[i];*/
                        while(c != '\0' && c != '/'){
                                cname[i] = pathname[i];
                                i++;
                                c = pathname[i];
                        }
                        cname[i] = pathname[i];
                        if(c == '/'){
                                j = 0;
                                while(j != i){
                                        dname[j] = cname[j];
                                        j++;
                                }
                        }
                        if(c == '\0')
                                break;
                        i++;
                        c = pathname[i];
                }
        }
        return dname;
}

/*
int decomname(char *pathname,MINODE *pmip,MINODE *mip){
        int i;
        int ino;

        if(pathname[0] == 0) return -1;
        if(pathname[0] == '/'){
                pmip = root;
                ino = 2;
        }
        else
                pmip = running->cwd;

        takenize(pathname);
        for(i=0;i<nname-1;i++){
                if(!S_ISDIR(pmip->inode.i_mode)){
                        printf("%s is not a directory\n",name[i]);
                        iput(pmip);
                        return -1;
                }
                ino = search(pmip,name[i]);
                if(!ino){
                        printf("no sucn component name %s\n",name[i]);
                        iput(pmip);
                        return -1;
                }
                iput(pmip);
                pmip = iget(dev,ino);
        }
        ino = search(pmip,name[i]);
        if(!ino){
                printf("no sucn component name %s\n",name[i]);
                iput(pmip);
                return -1;
        }
        //iput(pmip);
        mip = iget(dev,ino);
        return 0;
        //put(mip);
}

*/

