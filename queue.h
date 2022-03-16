//queue.h file
#ifndef QUEUE_H
#define QUEUE_H
#include "type.h"
int enqueue(PROC **queue,PROC *p);
PROC *dequeue(PROC **queue);
int printList(char *name,PROC *p);
#endif //QUEUE_H
