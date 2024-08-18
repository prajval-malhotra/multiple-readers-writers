#ifndef _ANSWER_H_
#define _ANSWER_H_

#include <stdio.h>   // printf()
#include <stddef.h> 
#include <string.h>  // memcpy()
#include <stdlib.h>  // rand() 
#include <time.h>    // srand(time()) 
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>

//TODO Define global data structures to be used
#define NUM_WRITERS 10
#define NUM_READERS 20

#define BUFFER_SIZE 65536

// TODO: enforce boundaries
#define MAX_THREAD_BUFFER_SIZE 16384 // BIG_BUFFER_HIGHER <= MAX_THREAD_BUFFER_SIZE <= BUFFER_SIZE

#define MAX_WRITER_VALUES 100

// random number generator config
#define BUFFER_SIZE_SKEW 1 // lower means higher possibility of generating a small sized buffer
#define SMALL_BUFFER_LOWER 1
#define SMALL_BUFFER_HIGHER 128
#define BIG_BUFFER_LOWER 1024
#define BIG_BUFFER_HIGHER 16384

typedef struct Buffer {
    // uint16 -> max buffer size is 65536
    uint32_t head;
    uint32_t tail;
    char buffer[BUFFER_SIZE];

    sem_t empty;
    sem_t full;
    sem_t insert_lock;
    sem_t remove_lock;

    void (*buffer_insert)(struct Buffer* b, char* insertBuf, size_t insertBufSize);
    void (*buffer_remove)(struct Buffer* b, char *removeBuf, size_t removeBufSize);

} Buffer_t;

#endif /* _ANSWER_H_ */