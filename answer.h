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

// uncomment to run using the semaphore or condition variable solution 
// #define SEMAPHORE
#define COND_VAR

#define DEBUG                   1 // uncomment for debug printing
#ifdef DEBUG
    #define DEBUG_PRINT(...) do { fprintf(stderr, __VA_ARGS__); } while (0)
#else
    #define DEBUG_PRINT(...) do { } while (0)
#endif

// macro to get min of two numbers
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// num of readers and writers
#define NUM_WRITERS             20
#define NUM_READERS             35

// amount of bytes readers will try to read at a time
#define READ_SIZE               1024
// num of bytes readers will split their reads into
#define CHUNK_SIZE              1024

// ive noticed a buffer size of about 1/130th the size of total data to be
//  usually sufficient, without long waiting times 
// eg, for cases where im reading/writing a total of 3900kb, a 30k byte buffer does not
//  cause a bottlenck. you may see the timed semwait come into play more and more with 
//  reducing buffer sizes under this ratio - go too low and we start losing data. 
#define BUFFER_SIZE             30000 // in bytes

// this is not a valid data value that we are processing, think of it like a 
//  delimiter to tell the readers when to stop 
#define BUFFER_SENTINEL         '#'

// BIG_BUFFER_HIGHER <= MAX_THREAD_BUFFER_SIZE <= BUFFER_SIZE
#define MAX_THREAD_BUFFER_SIZE  8192

// num of times each writer will loop and produce a random sized insertion value
#define WRITER_ITERATIONS       100

// random size buffer generator config
#define BUFFER_SIZE_SKEW        5   // lower means higher possibility of generating a small sized buffer
#define SMALL_BUFFER_LOWER      1
#define SMALL_BUFFER_HIGHER     128
#define BIG_BUFFER_LOWER        1024
#define BIG_BUFFER_HIGHER       2048

#define CHAR_COUNT_MAP_SIZE     128 // 128 ascii characters, could be smaller

typedef struct Buffer {
    // fixed size ring buffer
    uint32_t head;
    uint32_t tail;
    char buffer[BUFFER_SIZE];

#ifdef SEMAPHORE
    sem_t empty;
    sem_t full;
    sem_t insert_lock;
    sem_t remove_lock;
#endif /* SEMAPHORE */


#ifdef COND_VAR
    pthread_mutex_t mutex;
    pthread_mutex_t insert_mutex;
    pthread_mutex_t remove_mutex;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
#endif /* COND_VAR */

} Buffer_t;

#endif /* _ANSWER_H_ */