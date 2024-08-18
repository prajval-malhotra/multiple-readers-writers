#ifndef _ANSWER_H_
#define _ANSWER_H_

#include <stdio.h> 
#include <stddef.h> 
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>

//TODO Define global data structures to be used
#define NUM_WRITERS 10
#define NUM_READERS 20

#define BUFFER_SIZE 1024

#define MAX_WRITER_VALUES 10

typedef struct Buffer {
    // uint16 -> max buffer size is 65536
    uint16_t head;
    uint16_t tail;
    unsigned int buffer[BUFFER_SIZE];

    sem_t empty;
    sem_t full;
    sem_t insert_lock;
    sem_t remove_lock;

    void (*buffer_insert)(struct Buffer* b, unsigned int value);
    void (*buffer_remove)(struct Buffer* b, unsigned int value);

} Buffer_t;

#endif /* _ANSWER_H_ */