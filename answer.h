#ifndef _ANSWER_H_
#define _ANSWER_H_

#include <stdio.h>   // printf()
#include <stddef.h>
#include <stdbool.h> 
#include <string.h>  // memcpy()
#include <stdlib.h>  // rand() 
#include <time.h>    // srand(time()) 
#include <stdint.h>
#include <pthread.h>
#include <sched.h>
#include <semaphore.h>

//TODO Define global data structures to be used
#define NUM_WRITERS             10
#define NUM_READERS             20

#define READ_SIZE               100

// #define BUFFER_SIZE 65536
#define BUFFER_SIZE             32000
// #define BUFFER_SIZE             12000
#define BUFFER_SENTINEL         '#'

// TODO: enforce boundaries
#define MAX_THREAD_BUFFER_SIZE  8192 // BIG_BUFFER_HIGHER <= MAX_THREAD_BUFFER_SIZE <= BUFFER_SIZE

#define WRITER_ITERATIONS       10

// random size buffer generator config
#define BUFFER_SIZE_SKEW        10   // lower means higher possibility of generating a small sized buffer
#define SMALL_BUFFER_LOWER      1
#define SMALL_BUFFER_HIGHER     128
#define BIG_BUFFER_LOWER        1024
#define BIG_BUFFER_HIGHER       2048

#define CHAR_COUNT_MAP_SIZE     128 // 128 ascii characters, could be smaller

typedef struct Buffer {
    uint32_t head;
    uint32_t tail;
    char buffer[BUFFER_SIZE];

    sem_t empty;
    sem_t full;
    sem_t insert_lock;
    sem_t remove_lock;
    sem_t sentinel_lock;

    void (*buffer_insert)(struct Buffer* b, char* insertBuf, size_t insertBufSize);
    void (*buffer_remove)(struct Buffer* b, char *removeBuf, size_t removeBufSize);

} Buffer_t;

typedef enum OperationType {
    HEAD = 0,
    TAIL,
} OperationType;

#endif /* _ANSWER_H_ */