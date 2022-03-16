///////////////// global main.c file //////////////////////////////////////////////
#include<stdio.h>
#include<stdlib.h>
#include"type.h"
#include"queue.h"
#include"proc_func.h"
#include"util.h"
#include"mount_root.h"

//global variables
PROC proc[NPROC];       //NPROC PROCs
PROC *sleepList;        //sleepList of PROCs
PROC *zombieList;
PROC *freeList;         //freeList of PROCs
PROC *readyQueue;       //priority queue of READY procs
PROC *running;          //current running proc pointer
MINODE minode[NMINODE];         //in memory INODEs
MTABLE mtable[NMTABLE];         // mount tables
OFT oft[NOFT];                  // Opened file instance

MINODE *root;
int ninodes,nblocks,bmap,imap,iblock;
int dev;
char gline[25],*name[16];       //tokenized component string strings
int nname;                      // number of component strings
char *rootdev = "mydisk";       //default root_device

//global functions
int init(){
        int i,j;
        PROC *p;
        for(i = 0;i<NPROC;i++){         //PID = 0 to NRPOC-1
                p = &proc[i];
                p->pid = i;
		p->uid = 0;
		p->gid = 0;
                p->status = FREE;
                p->priority = 0;
                p->next = p+1;
		p->icwd = 0;
		for(j = 0;j<NFD;j++)
                        proc[i].fd[j] = 0;
        }
        proc[NPROC-1].next = 0;
        freeList = &proc[0];
        readyQueue = 0;
	sleepList = 0;
	zombieList = 0;

        //create  P0 as the initial running process
        p = running = dequeue(&freeList);       //use proc[0]
        p->status = READY;
        p->ppid = 0;

	for(i = 0; i<NMINODE;i++)       // initialize all minodes as FREE
                minode[i].refCount = 0;
	for(i = 0; i<NMTABLE;i++)       // initialize mtables as FREE
                mtable[i].dev = 0;
	for(i = 0; i<NOFT;i++)          // initialize ofts as FREE
                oft[i].refCount = 0;
	

        printf("init complete: P0 running\n");
}

int init_body(){
	MINODE * ip;
        char line[64],cmd[16],pathname[64];
	//if(argc > 1)
        //        rootdev = argv[1];
	mount_root(rootdev);
        ip = mialloc();
        ip = iget(dev,2);
        //printf("task %d start\n",running->pid);
        while(1){
                proc_menu();
		ext2_menu();
                printf("task %d is running\n",running->pid);
                printf("enter a command line: ");
                fgets(line,64,stdin);
                line[strlen(line) - 1] = 0;     //kill \n at end of line
		sscanf(line,"%s %s",cmd,pathname);
                //sscanf(line,"%s",cmd);
                //if(strcmp(cmd,"create") == 0)
                        //create a thread in the proc
                //        ;
                if(strcmp(cmd,"fork") == 0)
                        //fork a new proc;
                        do_kfork();
                else if(strcmp(cmd,"switch") == 0)
                        //switch the proc
                        do_switch();
                else if(strcmp(cmd,"exit") == 0)
                        //exit the system
                        do_exit(0);
                else if(strcmp(cmd,"kill") == 0)
                        //kill the proc
                        do_kill();
                else if(strcmp(cmd,"ps") == 0)
                        do_ps();
                else if(strcmp(cmd,"shutdown") == 0)
                        exit(0);


		else if(strcmp(cmd,"mkdir") == 0){
                        if(!mkdir_d(pathname))
				printf("suceed to mkdir!\n");
			else
				printf("failed to mkdir!\n");
		}
		else if(!strcmp(cmd,"rmdir"))
                        rmdir(pathname);
		
		else if(!strcmp(cmd,"rm"))
                        rm(pathname);
		else if(!strcmp(cmd,"ls"))
                        ls();
		else if(strcmp(cmd,"cd") == 0){
                        if(!chdir(pathname))
				printf("suceed to change directory!\n");
			else
				printf("failed to change directory!\n");
		}
		else if(!strcmp(cmd,"pwd"))
                        pwd(running->cwd);
		//else if(!strcmp(cmd,"quit"))
                //        quit();
		else if(strcmp(cmd,"create") == 0)
                        create("mmm");
		/*
		else if(strcmp(cmd,"link") == 0)
                        link("mmm","abclink2");
		else if(strcmp(cmd,"unlink") == 0)
                        unlink("/abclink2");
		*/
                else
                        printf("no this command!\n");
        }
}

int main(int argc,char **argv){
	printf("Welcome to the MT Multitasking System\n");
        //tmp = 0;
        //tmp2 = 0;
        init(); //initialize system ; create and run P0
	//mount_root(rootdev);
        kfork(init_body);       //kfork P1 into readyQueue
        while(1){
                printf("P0: switch process\n");
                if(readyQueue){
                        tswitch();
                }
        }
}

int scheduler(){
	PROC *p;
        printf("proc %d in scheduler()\n",running->pid);
	p = running;
        if(running->status == READY)
                enqueue(&readyQueue,running);
	if(running->status == ZOMBIE)
		enqueue(&zombieList,running);
        //printList("readyQueue",readyQueue);
        running = dequeue(&readyQueue);
	if(p->icwd != running->icwd)
		do_mount();
        //printf("next running  = %d\n",running->pid);
}

