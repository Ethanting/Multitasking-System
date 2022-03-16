/////////////////////////////// proc function file ///////////////////////////////
#include<stdio.h>
#include<stdlib.h>
#include"type.h"
#include"queue.h"

extern PROC *zombieList;
extern PROC *sleepList;        //sleepList of PROCs
extern PROC proc[NPROC];       //NPROC PROCs
extern PROC *freeList;         //freeList of PROCs
extern PROC *readyQueue;       //priority queue of READY procs
extern PROC *running;          //current running proc pointer
extern int init_body();

int kexit(int value){
	PROC *p,*q;
	p = running->child;

	if(p->pid == 1){
		printf("you can't exit P1 process!\n");
		return 0;
	}
	while(p){
		p->parent = &proc[1];
		p->ppid = 1;
		p = p->sibling;
	}
	p = running->parent;
	q = p->child;
	p = running;
	if(p->pid == q->pid){
		p->parent->child = p->sibling;
	}
	else{
		while(q->sibling->pid == p->pid){
			q->sibling = p->sibling;
		}
	}
	p = running;
	p->exitStatus = value;
	p->status = ZOMBIE;
	tswitch();
}
/*
int kexit(int value){
        int i;
        PROC *p,*q;
        printf("give away children to P1\n");
        p = running->child;

        while(p){
                p->parent = &proc[1];
                p->ppid = 1;
                p = p->sibling;
        }
        p = running->child;
        running->child = 0;

	p = running->sibling;
        q = proc[1].child;
        if(q == 0){
                proc[1].child = p;
        }
        else{
                while(q->sibling)
                        q = q->sibling;
                q->sibling = p;
        }

        running->exitStatus = value;
        running->status = ZOMBIE;
        kwakeup(running->parent);
        tswitch();
}
*/
/*
int kwait(int *status){
        int i,pid,child;
        PROC *p,*q;
        PROC *temp = 0;

        p = running->child;
        if(p == 0){
                printf("wait error: no child\n");
                return -1;
        }

        while(1){
                p = running->child;
                q = p;
                while(p){
                        if(p->status == ZOMBIE){
                                pid = p->pid;
                                *status = p->exitCode;
                                p->status = FREE;p->priority = 0;

                                if(p == running->child)
                                        running->child = p->sibling;
                                else
                                        q->sibling = p->sibling;
                                enqueue(&freeList,p);
                                return pid;
                        }
                        q = p;
                        p = p->sibling;
                }
                ksleep(running);
        }
}
*/
int ksleep(int event){
        running->event = event; //event ?
        running->status = SLEEP;
        enqueue(&sleepList,running);
        tswitch();
//      return 0;
}


int kwakeup(int event){
        PROC *p = sleepList;
        while(p){
                if(p->event == event){
                        p = Delete_List(&sleepList,p);
                        printf("wake up %d\n",p->pid);
                        p->status = READY;
                        enqueue(&readyQueue,p);
                }
                else
                        p = p->next;
        }
}

int kfork(){
        int i;
        PROC *p = dequeue(&freeList);
        PROC *q = NULL;
        if(!p){
                printf("no more proc\n");
                return -1;
        }

        /* initialize the new proc and its stack */
        p->status = READY;
        p->priority = 1;                // ALL PROCs priority = 1;except P0
        p->ppid = running->pid;
	p->uid = 1;
	p->gid = 1;

        /************** new task initial stack contents******************
         *kstack contains: |retPC|eax|ebx|ecx|edx|ebp|esi|edi|eflag
                            -1    -2  -3  -4  -5  -6  -7  -8  -9
         ****************************************************************/
        for(i = 1;i<10;i++)
                p->kstack[SSIZE -1] = 0;
        p->kstack[SSIZE-1] = (int)init_body; //retPC -> body()
        p->ksp = &(p->kstack[SSIZE-9]); // PROC.ksp -> saved eflag

        //handling PROC->child,sibling,parent;
        p->child = 0;
        q = running;
        if(!q->child)
                running->child = p;
        else{
                while(q->sibling)
                        q = q->sibling;
        }
        q->sibling = p;
        p->parent = running;
        enqueue(&readyQueue,p);         //enter p into readyQueue
        return p->pid;
}

//////////////////////////////////////////////////////////////////////////////////////////
//
int do_kfork(){
        int child = kfork();
        if(child < 0)
                printf("kfork failed\n");
        else{
                printf("proc %d kforked a child = %d\n",running->pid,child);
                //printList("readyQueue",readyQueue);
        }
        return child;
}

int do_switch(){
        tswitch();
}

int do_kill(){
	;
}

int do_exit(){
        kexit(0);
}

int do_ps(){
        printList("readyList",readyQueue);
        printList("freeList",freeList);
	printList("sleepList",sleepList);
	printList("zombieList",zombieList);
        return 0;
}
/*
int sort(PROC *p,int *num){
	if(p->child){
		*num++;
		sort(p->child);
	}
	if(p->sibling){
		*num++;
		sort(p->sibling);
	}
}

int do_ptree(){
	PROC *p,*q,*r;
	int n,m,sum;
	int length;
	int locat[10];
	length = n = sum = 0;
	//      10010010010010010010010010010010
	//      P0 P1 P2 P3 P4 P4 P5 P6 P7 P8 P9
	printf("               P0\n"); 15 * ' '
	printf("               |\n");
	printf("               P1\n");15 * ' '
	printf("               |\n");
	p = &proc[1];
	r = q = p->child;
	m = 1;
	while(r){
		while(q){
			p = q;	
			while(p){
				n++;
				p = p->sibling;
			}
			length = (n/2)*3;
			locat[m] = 15-length;
			m++;
			q = q->child;
		}
		r = r->sibling;
	}
}
*/
int do_mount(){
	printf("mount\n");
	return 0;
}

int proc_menu(){     //command menu: to be expanded later
        printf("\n*******************menu*********************\n");
        printf("  fork  switch  exit  ps  [stop]  [kill]\n");
	printf(" shutdown\n");
        printf("********************************************\n");
}

