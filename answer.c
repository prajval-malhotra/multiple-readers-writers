#include "answer.h"

static int insert_counter = 0;
static int remove_counter = 0;

// expect buffer to be malloced?
int get_external_data(char *buffer, int bufferSizeInBytes) {
	// static int i = 0;

	static char ch = 'A';

	// generate differing sizes of buffers
	if((rand() % 10) > BUFFER_SIZE_SKEW) { // generate buffer in small range
		bufferSizeInBytes = rand() % (SMALL_BUFFER_HIGHER + 1 - SMALL_BUFFER_LOWER) + SMALL_BUFFER_LOWER;
	}
	else {
		bufferSizeInBytes = rand() % (BIG_BUFFER_HIGHER + 1 - BIG_BUFFER_LOWER) + BIG_BUFFER_LOWER;
	}

	// bufferSizeInBytes = rand() % (SMALL_BUFFER_HIGHER + 1 - SMALL_BUFFER_LOWER) + SMALL_BUFFER_LOWER;

	// our data is chars in range ['A' - 'z']. Loop around when 'z' reached.
	memset(buffer, ch, sizeof(char) * bufferSizeInBytes);
	ch = (ch < 'z') ? ch + 1 : 'A';

	return bufferSizeInBytes;

}

void process_data(char *buffer, int bufferSizeInBytes) {

}

/**
 * Insert value arg2 into shared buffer pointed to by arg1
 * Return value < 0 on errors
 */
int buffer_insert(struct Buffer* b, char* insertBuf, size_t insertBufSize) {

	for(int i = 0; i < insertBufSize; ++i) { // only allow write when insertBufSize memory available - inefficient
		sem_wait(&b->empty); // wait for a slot to empty up
	}

	// make sure only one thread can write to a spot
	sem_wait(&b->insert_lock);

	unsigned int temp_insert_pos = b->tail;
	if((b->tail + insertBufSize) < BUFFER_SIZE) {
		memcpy(&b->buffer[b->tail], insertBuf, insertBufSize);	
	} else {
		int remaining_space = BUFFER_SIZE - b->tail;
		memcpy(&b->buffer[b->tail], insertBuf, remaining_space);
		memcpy(&b->buffer[0], insertBuf + remaining_space, insertBufSize - remaining_space);
	}
	b->tail = (b->tail + insertBufSize) % BUFFER_SIZE;
	printf("Inserted %ld %c bytes at index %d. New tail: %d\n", insertBufSize, insertBuf[0], temp_insert_pos, b->tail);
	insert_counter += insertBufSize;
	
	sem_post(&b->insert_lock);
	
	for(int i = 0; i < insertBufSize; ++i) {
		sem_post(&b->full);  // indicate a full spot for every byte written, ready to be read
	}

	return insertBufSize;

}

/**
 * Remove value from shared buffer pointed to by arg1. 
 * Value removed is arg2
 * Return value < 0 on errors
 */
void buffer_remove(struct Buffer* b, char* removeBuf, size_t removeBufSize) {

	// wait for a full spot
	sem_wait(&b->full);

	// ensure only one thread reads the value
	sem_wait(&b->remove_lock);

	// memcpy(removeBuf, &b->buffer[b->head], removeBufSize);
	*removeBuf = b->buffer[b->head]; // remove signle byte from shared buffer
	b->head = (b->head + 1) % BUFFER_SIZE;
	printf("Removed 1 byte at index %d. Total bytes removed: %d\n", b->head, remove_counter);
	remove_counter++;
	
	sem_post(&b->remove_lock);

	// indicate a freed up spot in the buffer
	sem_post(&b->empty);
	
}

/**
 * This thread is responsible for pulling data off of the shared data 
 * area and processing it using the process_data() API.
 */
void *reader_thread(void *arg) {
	//TODO: Define set-up required
	printf("Reader\n");
	Buffer_t* b = (Buffer_t *)arg;
	
	while(1) {
		char read_buffer;
		int data_size = 0;
		buffer_remove(b, &read_buffer, data_size);
		if(read_buffer == BUFFER_SENTINEL) {
			printf("Found sentinel value, exiting.\n");
			break;
		}
	}
	
	return NULL;
}


/**
 * This thread is responsible for pulling data from a device using
 * the get_external_data() API and placing it into a shared area
 * for later processing by one of the reader threads.
 */
void *writer_thread(void *arg) {
	//TODO: Define set-up required
	printf("Writer\n");
	Buffer_t* b = (Buffer_t *)arg;

	// malloc max possible input value buffer
	char *write_buffer = (char *)malloc(sizeof(char) * MAX_THREAD_BUFFER_SIZE);

	for(int iter = 0; iter < MAX_WRITER_VALUES; ++iter) {
		unsigned int data_size = get_external_data(write_buffer, 0);
		if(data_size > 0) {
			buffer_insert(b, write_buffer, data_size);
		}
	}

	free(write_buffer);
	
	return NULL;
}


int main(int argc, char **argv) {
	srand(time(NULL)); // truly random values
	int i;
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
			printf("Problem creating reader #%d\n", i);
		}
	}

	for(i = 0; i < NUM_WRITERS; i++) { 
		if(pthread_create(&writer_thids[i], NULL, writer_thread, &b) != 0) {
			printf("Problem creating writer #%d\n", i);
		}
	}


	for(i = 0; i < NUM_WRITERS; ++i) {
		if(pthread_join(writer_thids[i], NULL)) {
			printf("Problem joining writer #%d\n", i);
		}
	}
	printf("All writers exited. Total bytes inserted: %d\n", insert_counter);

	char sentinel_buffer[NUM_READERS];
	for(i = 0; i < NUM_READERS; ++i) {
		sentinel_buffer[i] = BUFFER_SENTINEL;
	}
	buffer_insert(&b, sentinel_buffer, NUM_READERS);

	for(i = 0; i < NUM_READERS; ++i) {
		if(pthread_join(reader_thids[i], NULL)) {
			printf("Problem joining reader #%d\n", i);
		} else {
			printf("%d readers exited\n", i);
		}
	}

	printf("Total inserted bytes: %d, removed bytes: %d.\nBytes Lost: %d\n", insert_counter, remove_counter, insert_counter - remove_counter);

	// perform once all threads finish executing
	sem_destroy(&b.full);
	sem_destroy(&b.empty);
	sem_destroy(&b.insert_lock);
	sem_destroy(&b.remove_lock);

	return 0;	
}