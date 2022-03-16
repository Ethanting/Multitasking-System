/******************** ext2fs func.c file ***************************/
#include<stdio.h>
#include"type.h"
#include<ext2fs/ext2_fs.h>
#include"ext2_func.h"
#include"util.h"
#include"ext2_func.h"

extern int dev;
extern PROC *running;
extern MINODE *root;
extern MTABLE *mtable[NMTABLE];
extern int ninodes,nblocks,bmap,imap,iblock;
extern MINODE minode[NMINODE];         //in memory INODEs
extern OFT oft[NOFT];                  // Opened file instance


extern *name[64];
char gline[256];
int nname;

int chdir(char *pathname){
	int ino;
	MINODE *mip = 0;
	if (pathname[0] == 0){
     		iput(running->cwd);
     		running->cwd = iget(root->dev, 2);
     		return 0;
  	}
	ino = getino(pathname);	//return error i ino = 0
	if(!ino){
		return -1;
	}
	mip = iget(dev,ino);
	if(!S_ISDIR(mip->inode.i_mode)){
		return -1;
	}
	else{
		iput(running->cwd);
		running->cwd = mip;
		return 0;
	}
}

int rpwd(MINODE *wd)
{
  	char buf[BLKSIZE], myname[256], *cp;
  	MINODE *parent, *ip;
  	u32 myino, parentino;
  	DIR   *dp;

  	if (wd == root)
      		return 0;

  	get_block(dev, wd->inode.i_block[0], buf);
  	dp = (DIR *)buf;
  	cp = buf;
  	myino = dp->inode;

  	cp += dp->rec_len;
  	dp = (DIR *)cp;
  	parentino = dp->inode;

  	printf("paretnino = %d ", parentino);
  	//  parentino = findino(wd, &myino);

  	parent = iget(dev, parentino);
  	findmyname(parent, myino, myname);

  	// recursively call rpwd()
  	rpwd(parent);

  	iput(parent);
  	printf("/%s", myname);

  	return 0;
}

void pwd(MINODE *wd)
{
  	if (wd == root){
    		printf("/\n");
    		return 0;
 	}
  	rpwd(wd);
  	printf("\n");
	return 0;
}


int ls(){
	int i;
	char buf[BLKSIZE];
	char temp[256];
	MINODE *mip;
	DIR *dp;
	char *cp;
	mip = running->cwd;
	for(i=0;i<12;i++){
		if(mip->inode.i_block[i] == 0){
			printf("\n");	
			return 0;
		}
		get_block(mip->dev,mip->inode.i_block[i],buf);
		dp = (DIR*)buf;
		cp = buf;
		while(cp < buf + BLKSIZE){
			strncpy(temp,dp->name,dp->name_len);
			temp[dp->name_len] = 0;
			printf(" %s ",temp);
			cp += dp->rec_len;
			dp = (DIR*)cp;
		}
	}
	printf("\n");
	return 0;
}
/////////////////////// mkdir file ////////////////////////////////////////
int kmkdir(MINODE *pmip,char *basename,int pino){
	int i;
	int ino;
	int blk;
	MINODE *mip;
	INODE *ip;

	char buf[BLKSIZE] = {0};

	ino = ialloc(dev);
	blk = balloc(dev);
	mip = iget(dev,ino);

	ip = &mip->inode;
	ip->i_mode = 0x41ED;	//040755: DIR type and permissions
	ip->i_uid = running->uid;	//owner uid
	ip->i_gid = running->gid;	//group id
	ip->i_size = BLKSIZE;		//size in bytes
	ip->i_links_count = 2;
	ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
	ip->i_blocks = 2;
	ip->i_block[0] = blk;
	for(i = 1;i<15;i++)
		ip->i_block[i] = 0;
	//ip->i_block;
	mip->dirty = 1;
	iput(mip);

	//make . entry
	DIR *dp = (DIR*)buf;
	dp->inode = ino;
	dp->rec_len = 12;
	dp->name_len = 1;
	dp->name[0] = '.';
	//make .. entry: pino = parent DIR ino,blk = allocated block
	dp = (char *)dp + 12;
	dp->inode = pino;
	dp->rec_len = BLKSIZE -12;
	dp->name_len = 2;
	dp->name[0] = dp->name[1] = '.';
	put_block(dev,blk,buf);

	enter_name(pmip,ino,basename);

}

int mkdir_d(char *pathname){
	int i,ino;
	MINODE *mip;
	MINODE *pmip;
	if(pathname[0] == 0) return -1;
	if(pathname[0] == '/'){ 
		mip = root;
		ino = 2;
	}
	else 
		mip = running->cwd;
	tokenize(pathname);
	for(i=0;i<nname-1;i++){
		if(!S_ISDIR(mip->inode.i_mode)){
			//printf("%s is not a directory\n",name[i]);
			iput(mip);
			return -1;
		}
		ino = search(mip,name[i]);
		if(!ino){
			//printf("no such component name %s\n",name[i]);
			iput(mip);
			return -1;
		}
		iput(mip);
		mip = iget(dev,ino);
	}
	ino = mip->ino;
	if(S_ISDIR(mip->inode.i_mode))
		if(!search(mip,name[i])) 
			kmkdir(mip,name[i],ino);
	iput(mip);
	return 0;
}
/////////////////////// mkdir file ////////////////////////////////////////
//
////////////////////// create file ///////////////////////////////////////

int kcreate(MINODE *pmip,char *basename,int pino){
	int i;
        int ino;
        int blk;
        MINODE *mip;
        INODE *ip;

        char buf[BLKSIZE] = {0};

        ino = ialloc(dev);
        blk = balloc(dev);
        mip = iget(dev,ino);

        ip = &mip->inode;
        ip->i_mode = 0x81A4;    //040755: DIR type and permissions
        ip->i_uid = running->uid;       //owner uid
        ip->i_gid = running->gid;       //group id
        ip->i_size = 0;           //size in bytes
        ip->i_links_count = 1;
        ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
        ip->i_blocks = 2;
        ip->i_block[0] = blk;
	//ip->i_inode = ino;
        for(i = 1;i<15;i++)
                ip->i_block[i] = 0;
        //ip->i_block;
        mip->dirty = 1;
        iput(mip);

	enter_name(pmip,ino,basename);
}

int create(char *pathname){
	int i,ino;
	MINODE *mip;
	if(pathname[0] == 0) return -1;
	if(pathname[0] == '/'){
		mip = root;
		ino = 2;
	}
	else
		mip = running->cwd;
	tokenize(pathname);
	for(i=0;i<nname-1;i++){
		if(!S_ISDIR(mip->inode.i_mode)){
			printf("%s is not a directory\n",name[i]);
			iput(mip);
			return -1;
		}
		ino = search(mip,name[i]);
		if(!ino){
			printf("no sucn component name %s\n",name[i]);
			iput(mip);
			return -1;
		}
		iput(mip);
		mip = iget(dev,ino);
	}
	if(S_ISDIR(mip->inode.i_mode))
		if(!search(mip,name[i]))
			kcreate(mip,name[i],ino);
	printf("suceed to create!\n");
	iput(mip);
	return 0;
}


///////////////////////////// rmdir file /////////////////////////////////////////////////
int rmdir(char *pathname){
	int i;
	int num;
	char *cp,temp[256],myname[256],buf[BLKSIZE];
	DIR *dp;

	// (1).get in-memory INODE of pathname
	int ino,pino;
	MINODE *mip,*pmip;
	ino = getino(pathname);
	if(!ino){
		printf("no file\n");
		return 0;
	}
	mip = iget(dev,ino);

	// (2).verify INODE is a DIR(by INODE.i_mode field)
	if(mip->refCount == 1){
		num = 0;
		for(i=0;i<12;i++){
			if(mip->inode.i_block[i] == 0)
				break;
			get_block(mip->dev,mip->inode.i_block[i],buf);
			dp = (DIR*)buf;
			cp = buf;
			while(cp < buf + BLKSIZE){
				//findino
				strncpy(temp,dp->name,dp->name_len);
				temp[dp->name_len] = 0;
				printf("\n%s\n",temp);
				if(strcmp("..",temp) == 0){
					printf("found %s : inumber = %d\n","..",dp->inode);
					pino = dp->inode;
					//break;
				}
				if(dp->name_len != 0)
					num++;
				cp += dp->rec_len;
				dp = (DIR*)cp;
			}
		}
	}

	if(num > 2){
		printf("this DIR is not empty\n");
		return 0;
	}
	// (3).get parent's ino and inode
	// findino
	pmip = iget(mip->dev,pino);

	// (4).get name from parent DIR's data block
	findmyname(pmip,ino,myname);

	// (5).remove name from parent directory
	rm_child(pmip,myname);

	// (6).dec parent links_count by 1; mark parent pimp dirty;
	pmip->inode.i_links_count - 1;
	pmip->dirty = 1;
	iput(pmip);

	// (7).deallocate its data blocks and inode
	bdalloc(mip->dev,mip->inode.i_block[0]);
	idalloc(mip->dev,mip->ino);
	iput(mip);
}

///////////////////////// link file ////////////////////////////////////////////
/*
int link(char *old_file,char *new_file){
	// (1). verify old_file exists and is not a DIR
	int i,oino,pino;
	MINODE *omip,*pmip;
	oino = getino(old_file);
	omip = iget(dev,oino);
	if(S_ISDIR(omip->inode.i_mode)){
		printf("old file is dir file\n");
		return 0;
	}

	// (2). new file must not exits yet;
	if(getino(new_file)){
		printf("new file is exit\n");
		return 0;
	}

	// (3).creat new_file with the same inode number of old_file
	
	if(new_file[0] == 0) return -1;
        if(new_file[0] == '/'){
                pmip = root;
                pino = 2;
        }
        else
                pmip = running->cwd;
        
	tokenize(new_file);
        for(i=0;i<nname-1;i++){
                if(!S_ISDIR(pmip->inode.i_mode)){
                        printf("%s is not a directory\n",name[i]);
                        iput(pmip);
                        return -1;
                }
                pino = search(pmip,name[i]);
                if(!pino){
                        printf("no sucn component name %s\n",name[i]);
                        iput(pmip);
                        return -1;
                }
                iput(pmip);
                pmip = iget(dev,pino);
        }
	// creat entry in new parent DIR with same inode number of old_file
	enter_name(pmip,oino,name[i]);

	// (4).put the inode into disk
	omip->inode.i_links_count++;
	omip->dirty = 1;
	iput(omip);
	iput(pmip);
	return 0;
}
*/
////////////////////////////// unlink file //////////////////////////////////////////
/*
int unlink(char *pathname){
	int i;
	int pino,ino;
	MINODE *pmip,*mip;
	char *parent,*child;
	// (1).get filename's minode;

	ino = getino(pathname);
	mip = iget(dev,ino);
	if(S_ISDIR(mip->inode.i_mode)){
		printf("this file is a dir\n");
		return 0;
	}
	// (2).remove name entry from parent DIR's data block;
	parent = dirname(pathname);
	child = basename(pathname);
	pino = getino(parent);
	pmip = iget(dev,pino);
	rm_child(pmip,child);
	pmip->dirty  = 1;
	iput(pmip);
	// (3).decrement INODE's link_count by 1;
	mip->inode.i_links_count--;
	// (4)
	if(mip->inode.i_links_count > 0)
		mip->dirty = 1;
	else{
		bdalloc(mip->dev,mip->inode.i_block[0]);
        	idalloc(mip->dev,mip->ino);
        	iput(mip);


		**
		for(i=1;i<12;i++){
			bdalloc(dev,mip->inode.i_block[i]);
		}**
		//bdalloc(dev,mip);
		//idalloc(dev,ino);
		//bdalloc(int dev,int blk)
		// deallocate all data blocks in INODE;
		// deallocate INODE;
	}
	iput(mip);
}
*/

////////////////////////////// rm file //////////////////////////////////////////////
int rm(char *pathname){
        int i;
        int pino,ino;
        MINODE *pmip,*mip;
        char *parent,*child;
        ino = getino(pathname);
        if(ino == 0)
                mip = running->cwd;
        else
                mip = iget(dev,ino);
        printf("\n%x\n",mip->inode.i_mode);
        if(S_ISDIR(mip->inode.i_mode)){
                printf("this file is a dir\n");
                return 0;
        }
        // (2).remove name entry from parent DIR's data block;
        parent = dirname(pathname);
        child = basename(pathname);
        pino = getino(parent);
        if(pino == 0)
                pmip = running->cwd;
        else
                pmip = iget(dev,pino);
        rm_child(pmip,child);
        pmip->dirty  = 1;
        iput(pmip);
        
        bdalloc(mip->dev,mip->inode.i_block[0]);
        idalloc(mip->dev,mip->ino);
        iput(mip);

	return 0;
}


//////////////////////////////////////////////////////////////////////////////
// The second open,close,lseek,read,write,opendir,readdir
// ///////////////////////////////////////////////////////////////////////////
//////////////////////// open dir ////////////////////////////////////////////
/*
int get_oft(){
	int i;
	for(i = 0;i<NOFT;i++){
		if(oft[i].refCount != 0)
			return i;
	}
	return -1;
}

int open_d(char *filename,int flags){
	int ino;
	int mode;
	int oftt;
	MINODE *mip;
	// (1).get file's minode;
	ino = getino(filename);
	if(ino == 0){
		create(filename);
		ino = getino(filename);
	}
	mip = iget(dev,ino);

	// (2).allocate an openTable entry OFT; initialize OFT entries;
	if((oftt = get_oft()) < 0){
		printf("failed to allocate a oft\n");
		return 0;
	}
	oftt.mode = 0 or 1 or 2 or 3;
	oftt.minodePtr = mip;
	oftt.refCount = 1;
	oftt.offset = 0;

	// (3).search for the first FREE fd[index] entry with the lowest index in PROC
	for(i=0;i<NFD;i++){
		if(running.fd[i] == 0)
			return i;
	}
	return -1;
}

int lseek_d(int fd,int position,int whence){
	
}

int close(int fd){
	if(running.fd[fd] != 0){
		OFT.refCount--;
		if(refCount == 0){
			iput(OFT.minodePtr);
		}
	}
	running.fd[fd] = 0;
}
*/

////////////////////////////////////////////////////////////////////////////
//////////////// Alogrithm of mount ////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


//1.If no parameter,display current mounted file systems;
//2.Check whether filesys is already mounnted:
//  The MOUNT table entries contains mounted file system (device) names
//  and their mounting points.Reject if the device is already mounted.
//  If not, allocate a free MOUNT table entry.
//3.Open the filesys virtual disk (under Linux) for RW;
//  use (Linux ) file descriptor as new dev.Read filesys superblock to verify it is an EXT3 FS
//4.


////////////////////////// menu //////////////////////////////////////////////
int quit(){     //write all modified minodes to disk
        int i;
        MINODE *mip;
        for(i=0;i<NMINODE;i++){
                //MINODES *mip = &minode[i];
                mip = &minode[i];
                if(mip->refCount && mip->dirty){
                        mip->refCount = 1;
                        iput(mip);
                }
        }
        exit(0);
}


int ext2_menu(){     //command menu: to be expanded later
        printf("*******************menu*********************\n");
        printf(" mkdir  rmdir  rm  ls  cd  pwd  quit  create\n");
        printf(" link   unlink\n");
        printf("********************************************\n");
	return 0;
}

