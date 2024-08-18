#include "answer.h"


int get_external_data(char *buffer, int bufferSizeInBytes) {

	return 0;
}

void process_data(char *buffer, int bufferSizeInBytes) {

}


/**
 * Insert value arg2 into shared buffer pointed to by arg1
 * Return value < 0 on errors
 */
int buffer_insert(struct Buffer* b, unsigned int value) {

	b->buffer[b->tail] = value;
	b->tail = (b->tail + 1) % BUFFER_SIZE; 
	
	return 0;
}

/**
 * Remove value from shared buffer pointed to by arg1. 
 * Value removed is arg2
 * Return value < 0 on errors
 */
int buffer_remove(struct Buffer* b, unsigned int* value) {

	*value = b->buffer[b->head];
	b->head = (b->head + 1) % BUFFER_SIZE;

	return 0;
}

/**
 * This thread is responsible for pulling data off of the shared data 
 * area and processing it using the process_data() API.
 */
void *reader_thread(void *arg) {
	//TODO: Define set-up required
	printf("Reader\n");
	
	while(1) {
		//TODO: Define data extraction (queue) and processing 
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
	
	while(1) {
		//TODO: Define data extraction (device) and storage 
	}
	
	return NULL;
}


int main(int argc, char **argv) {
	int i;
	Buffer_t b = { .head = 0, .tail = 0 }; // create shared buffer
	pthread_t reader_thids[NUM_READERS];
	pthread_t writer_thids[NUM_WRITERS];
	
	for(i = 0; i < NUM_READERS; i++) { 
		if(pthread_create(&reader_thids[i], NULL, reader_thread, NULL) != 0) {
			printf("Problem creating reader #%d\n", i);
		}
	}

	for(i = 0; i < NUM_WRITERS; i++) { 
		if(pthread_create(&writer_thids[i], NULL, writer_thread, NULL) != 0) {
			printf("Problem creating writer #%d\n", i);
		}
	}

	/*
	for(i = 0; i < NUM_WRITERS; ++i) {
		if(pthread_join(writer_thids[i], NULL)) {
			printf("Problem joining writer #%d\n", i);
		}
	}
	*/

	return 0;	
}