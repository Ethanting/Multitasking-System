//queue.c file
#include "queue.h"

int enqueue(PROC **queue,PROC *p){
	PROC *q = *queue;
	if(q == 0 || p->priority > q->priority){
		*queue = p;
		p->next = q;
	}
	else{
		while(q->next && p->priority <= q->next->priority)
			q = q->next;
		p->next = q->next;
		q->next = p;
	}
}
//return the first element of the queue
//   in-> **|**|**|**|**|**|**| ->out
PROC *dequeue(PROC **queue){
	PROC *p = *queue;
	if(p)
		*queue = (*queue)->next;
	return p;
}

PROC *Insert_List(PROC **list,PROC *p){
	;
}
//tem -> q -> next
//with head pointer
PROC *Delete_List(PROC **list,PROC *p){
	PROC *q = *list;
	PROC *tem = q;
	while(q){
		if(q->pid == p->pid){
			tem->next = q->next;
			break;
		}
		else{
			tem = q;
			q = q->next;
		}
	}
	return q;
}

int printList(char *name,PROC *p){
	printf("%s = ",name);
	while(p){
		printf("[%d %d]->",p->pid,p->priority);
		p = p->next;
	}
	printf("NULL\n");
}
