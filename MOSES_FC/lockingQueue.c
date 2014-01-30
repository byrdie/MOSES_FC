/**
 *Author: Roy Smart
 *MOSES LAB
 **/
#include <sys/time.h>

#include "lockingQueue.h"

void lockingQueue_init(LockingQueue * queue){
    pthread_mutex_init(&queue->lock, NULL);
    pthread_condattr_init(&queue->cattr);
    pthread_cond_init(&queue->cond, &queue->cattr);

	queue->count=0;
}

void enqueue(LockingQueue * queue, Packet * p) {
    
    
    if (queue->first == NULL) {
        queue->first = p;
        queue->last = queue->first;
                
    } else {
        queue->last->next = p;
        queue->last = p;
    }
    queue->count++;
    
             

}

Packet dequeue(LockingQueue * queue) { 
    struct timespec timeToWait;
    struct timeval now;
    
    gettimeofday(&now, NULL);
    timeToWait.tv_sec = now.tv_sec + 2;
    timeToWait.tv_nsec = 0;
    
    
    
    while((queue->count == 0) && ts_alive){
        pthread_cond_timedwait(&queue->cond, &queue->lock, &timeToWait);
    }
    Packet p = *(queue->first);
    queue->first = (Packet *)p.next;
    queue->count--;

    
    return p;
}

void lockingQueue_destroy(LockingQueue * queue){
    pthread_cond_destroy(&queue->cond);
    pthread_mutex_destroy(&queue->lock);
}
