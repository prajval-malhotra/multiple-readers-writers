#include "answer.h"

static int insert_char_map[CHAR_COUNT_MAP_SIZE] = { 0 };
static int remove_char_map[CHAR_COUNT_MAP_SIZE] = { 0 };

/** 
 *  Fills up the provided buffer with a randomly sized character
 *  	Smaller BUFFER_SIZE_SKEW values give a higher probablility of generating 
 *		smaller sized buffers, vice versa for larger buffers
 *	arg1 is a ptr to a malloced buffer of size MAX_THREAD_BUFFER_SIZE
 * 	arg 2 is unused and set in the function in this psuedo implementation
 */
static int get_external_data(char *buffer, int bufferSizeInBytes) {

	static char ch = 'A';

	// generate differing sizes of buffers
	if((rand() % 10) > BUFFER_SIZE_SKEW) { // generate buffer in small range
		bufferSizeInBytes = rand() % (SMALL_BUFFER_HIGHER + 1 - SMALL_BUFFER_LOWER) + SMALL_BUFFER_LOWER;
	} else { // generate buffer in large range
		bufferSizeInBytes = rand() % (BIG_BUFFER_HIGHER + 1 - BIG_BUFFER_LOWER) + BIG_BUFFER_LOWER;
	}

	DEBUG_PRINT("Insert %u %c bytes\n", bufferSizeInBytes, ch);

	// our data is chars in range ['A' - 'z']. Loop around when 'z' reached.
	memset(buffer, ch, sizeof(char) * bufferSizeInBytes);
	ch = (ch < 'z') ? ch + 1 : 'A';

	return bufferSizeInBytes;

}

/** 
 *	Does some data processing
 */
static void process_data(char *buffer, int bufferSizeInBytes) {
	// do some processing
	DEBUG_PRINT("Remove %d bytes.\n", bufferSizeInBytes);
}

/**
 * Insert value arg2 into shared buffer pointed to by arg1 of size arg3
 * Return value < 0 on errors
 */
static int buffer_insert(struct Buffer* b, char* insertBuf, int totalInsertSize) {

	struct timespec ts;
	if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
        printf("clock_gettime error\n");
		return 0;
    }

	int insertBufSize = totalInsertSize;
	if(totalInsertSize > CHUNK_SIZE) {
		insertBufSize = CHUNK_SIZE;
	}

	// insert in chunks, prevent deadlocks when all 
	//	writers get insertBufSize-1 empties and get stuck
	while(totalInsertSize > 0) {
		if(totalInsertSize < CHUNK_SIZE) {
			insertBufSize = totalInsertSize;
		}

    	ts.tv_sec += 1; // should probably make this smaller - is there a better solution?
		// only allow write when insertBufSize memory available
		bool skip_iteration = false;
		for(int bufIdx = 0; bufIdx < insertBufSize; ++bufIdx) {
			if(sem_timedwait(&b->empty, &ts) != 0) {
				// release all acquired semaphores after waiting for too long
				while(bufIdx >= 0) {
					sem_post(&b->empty);
					--bufIdx;
				}
				skip_iteration = true; // dont insert anything
				break;
			}
		}

		if(skip_iteration == true) {
			continue;
		}
		totalInsertSize -= CHUNK_SIZE;

		// make sure only one thread can write to a spot
		sem_wait(&b->insert_lock);

		// wrap around insert, ensure that we dont write beyond the ring buffers boundary
		if((b->tail + insertBufSize) < BUFFER_SIZE) { // one shot
			memcpy(&b->buffer[b->tail], insertBuf, insertBufSize);	
		} else { // wrap around
			size_t remaining_space = BUFFER_SIZE - b->tail;
			memcpy(&b->buffer[b->tail], insertBuf, remaining_space);
			memcpy(&b->buffer[0], insertBuf + remaining_space, insertBufSize - remaining_space);
		}

		// keep track of all inserted data for error check at the end
		for(int countIdx = 0; countIdx < insertBufSize; ++countIdx) {
			++insert_char_map[insertBuf[countIdx]];
		}
		b->tail = (b->tail + insertBufSize) % BUFFER_SIZE;
		
		sem_post(&b->insert_lock);
		
		// indicate a full spot for every byte written, ready to be read
		for(int i = 0; i < insertBufSize; ++i) {
			sem_post(&b->full);
		}
	
	}

	return insertBufSize;

}

/**
 * Remove value from shared buffer pointed to by arg1. 
 * Value removed is arg2
 * Return value < 0 on errors
 */
static void buffer_remove(struct Buffer* b, char* removeBuf, int totalRemoveSize) {

	struct timespec ts;
	if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
        printf("clock_gettime error\n");
		return;
    }

	int bufferRemoveSize = totalRemoveSize;
	bufferRemoveSize = CHUNK_SIZE;

	// remove in chunks, prevent deadlocks when all 
	//	readers get removeBufSize-1 fulls and get stuck
	while(totalRemoveSize > 0) {
		if(totalRemoveSize < CHUNK_SIZE) {
			bufferRemoveSize = totalRemoveSize;
		}
		ts.tv_sec += 1;
		// wait for a full spot
		bool skip_iteration = false;
		for(int bufIdx = 0; bufIdx < bufferRemoveSize; ++bufIdx) {
			if(sem_timedwait(&b->full, &ts) != 0) {
				// release all acquired semaphores after waiting for too long
				while(bufIdx >= 0) {
					sem_post(&b->full);
					--bufIdx;
				}
				skip_iteration = true;
				break;
			}
		}
		if(skip_iteration == true) {
			continue;
		}
		totalRemoveSize -= CHUNK_SIZE;

		// ensure only one thread reads the value
		sem_wait(&b->remove_lock);

		if((b->head + bufferRemoveSize) < BUFFER_SIZE) {
			memcpy(removeBuf, &b->buffer[b->head], bufferRemoveSize);	
		} else {
			int remaining_space = BUFFER_SIZE - b->head;
			memcpy(removeBuf, &b->buffer[b->head], remaining_space);
			memcpy(removeBuf + remaining_space, &b->buffer[0], bufferRemoveSize - remaining_space);
		}

		// keep track of all removed data for error check at the end
		for(int countIdx = 0; countIdx < bufferRemoveSize; ++countIdx) {
			++remove_char_map[removeBuf[countIdx]];
		}
		b->head = (b->head + bufferRemoveSize) % BUFFER_SIZE;

		sem_post(&b->remove_lock);

		// indicate a freed up spot in the buffer
		for(int i = 0; i < bufferRemoveSize; ++i) {
			sem_post(&b->empty);
		}

	}
	
}

/**
 * This thread is responsible for pulling data off of the shared data 
 * area and processing it using the process_data() API.
 */
void *reader_thread(void *arg) {
	DEBUG_PRINT("Create reader\n");
	Buffer_t* b = (Buffer_t *)arg;
	
	char *read_buffer = (char *)malloc(sizeof(char) * READ_SIZE);
	while(1) {
		buffer_remove(b, read_buffer, READ_SIZE);
		if(read_buffer[0] == BUFFER_SENTINEL) {
			DEBUG_PRINT("Found sentinel value, exiting.\n");
			break;
		} else if(read_buffer[READ_SIZE - 1] == BUFFER_SENTINEL) {
			// process all valid, non sentinel data
			int readBufIdx = 0;
			while(readBufIdx < READ_SIZE && read_buffer[readBufIdx] != BUFFER_SENTINEL) {
				readBufIdx++;
			}
			process_data(read_buffer, readBufIdx + 1);
			break;
		}
		process_data(read_buffer, READ_SIZE);
	}
	
	free(read_buffer);
	return NULL;
}

/**
 * This thread is responsible for pulling data from a device using
 * the get_external_data() API and placing it into a shared area
 * for later processing by one of the reader threads.
 */
void *writer_thread(void *arg) {
	DEBUG_PRINT("Create writer\n");
	Buffer_t* b = (Buffer_t *)arg;

	// malloc max possible input value buffer
	char *write_buffer = (char *)malloc(sizeof(char) * MAX_THREAD_BUFFER_SIZE);

	for(int iter = 0; iter < WRITER_ITERATIONS; ++iter) {
		int data_size = get_external_data(write_buffer, 0);
		if(data_size > 0) {
			buffer_insert(b, write_buffer, data_size);
		}
	}

	free(write_buffer);
	return NULL;
}


/**
 *	print final results
 */
static void print_results() {
	printf("\033[1m==========================================================\033[0m\n");
	printf("\033[33m"); // print in yellow
	printf("Note: '#' is a sentinel value, like a delimiter. \n> 0 '#'s dropped does not imply data was lost\n"); // print in yellow
	printf("\033[0m"); // print in blue
	printf("\033[1m----------------------------------------------------------\033[0m\n");
	printf("\033[34m"); // print in blue
	printf("\033[1mCharacter\tInsertions\tRemovals\tDifference\n");
	printf("\033[0m"); // reset
	printf("\033[1m----------------------------------------------------------\033[0m\n");
	int total_char_types = 0;
	int total_insertions = 0;
	int total_removals   = 0;
	int char_difference  = 0;
	for(int i = 0; i < CHAR_COUNT_MAP_SIZE; ++i) {
		total_insertions += insert_char_map[i];
		total_removals   += remove_char_map[i];
		char_difference  += insert_char_map[i] - remove_char_map[i];

		if(insert_char_map[i] != 0 && remove_char_map[i] != 0) {
			++total_char_types;
			(insert_char_map[i] - remove_char_map[i]) ? printf("\033[31m") : printf("\033[32m");
			printf("%c:\t\t%d\t\t%d\t\t%d\t\n", 
				i, insert_char_map[i], remove_char_map[i],
				insert_char_map[i] - remove_char_map[i]);
			printf("\033[0m"); // reset special formatting
			
		}
	}
	printf("\033[1m----------------------------------------------------------\n");
	printf("\033[34m");
	printf("\033[1m%u\t\t%u\t\t%u\t\t%u\t\033[0m\n", 
			total_char_types, total_insertions, total_removals, char_difference);
	printf("\033[1m----------------------------------------------------------\033[0m\n");
	int total_rw = total_insertions + total_removals;
	printf("\033[1m %d kilobytes read/written\n", (total_rw > 0) ? total_rw / 1000 : 0);
	printf("\033[1m %d bytes read/written\n", total_rw);
	printf("\033[0m");
	printf("\033[1m==========================================================\033[0m\n");
}


int main(int argc, char **argv) {
	int i;
	srand(time(NULL)); // set time based seed for truly random values
	Buffer_t b = { .head = 0, .tail = 0 }; // create shared buffer
	
	// initialize semaphores
	sem_init(&b.full, 0, 0);
	sem_init(&b.empty, 0, BUFFER_SIZE);
	sem_init(&b.insert_lock, 0, 1);
	sem_init(&b.remove_lock, 0, 1);

	pthread_t reader_thids[NUM_READERS];
	pthread_t writer_thids[NUM_WRITERS];
	
	for(i = 0; i < NUM_READERS; i++) { 
		if(pthread_create(&reader_thids[i], NULL, reader_thread, &b) != 0) {
			DEBUG_PRINT("Problem creating reader #%u\n", i);
		}
	}

	for(i = 0; i < NUM_WRITERS; i++) { 
		if(pthread_create(&writer_thids[i], NULL, writer_thread, &b) != 0) {
			DEBUG_PRINT("Problem creating writer #%u\n", i);
		}
	}

	// wait for all writers to finish first
	for(i = 0; i < NUM_WRITERS; ++i) {
		if(pthread_join(writer_thids[i], NULL)) {
			DEBUG_PRINT("Problem joining writer #%u\n", i);
		}
	}
	DEBUG_PRINT("All writers exited.\n");

	const int num_sentinels = NUM_READERS * READ_SIZE; 
	char sentinel_buffer[num_sentinels];
	memset(sentinel_buffer, BUFFER_SENTINEL, num_sentinels);
	buffer_insert(&b, sentinel_buffer, num_sentinels);

	// wait for readers to read all data in the buffer
	for(i = 0; i < NUM_READERS; ++i) {
		if(pthread_join(reader_thids[i], NULL)) {
			DEBUG_PRINT("Problem joining reader #%u\n", i);
		} else {
			DEBUG_PRINT("%u readers exited\n", i);
		}
	}

	print_results();

	// perform once all threads finish executing
	sem_destroy(&b.full);
	sem_destroy(&b.empty);
	sem_destroy(&b.insert_lock);
	sem_destroy(&b.remove_lock);

	return 0;	
}