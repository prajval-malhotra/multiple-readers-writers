#include "answer.h"


int get_external_data(char *buffer, int bufferSizeInBytes) {
	static int i = 0;

	return i++;
}

void process_data(char *buffer, int bufferSizeInBytes) {

}


/**
 * Insert value arg2 into shared buffer pointed to by arg1
 * Return value < 0 on errors
 */
void buffer_insert(struct Buffer* b, unsigned int value) {

	sem_wait(&b->empty); // wait for a slot to empty up

	// make sure only one thread can write to a spot
	sem_wait(&b->insert_lock);
	b->buffer[b->tail] = value;
	b->tail = (b->tail + 1) % BUFFER_SIZE;
	printf("Inserted %d\n", value);
	sem_post(&b->insert_lock);
	
	sem_post(&b->full); // indicate a full spot, ready to be read

}

/**
 * Remove value from shared buffer pointed to by arg1. 
 * Value removed is arg2
 * Return value < 0 on errors
 */
void buffer_remove(struct Buffer* b, unsigned int* value) {

	// wait for a full spot
	sem_wait(&b->full);

	// ensure only one thread reads the value
	sem_wait(&b->remove_lock);
	*value = b->buffer[b->head];
	b->head = (b->head + 1) % BUFFER_SIZE;
	printf("Removed %d\n", *value);
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
		unsigned int value = 0;
		buffer_remove(b, &value);
		if(value == -1) {
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
	
	while(1) {
		unsigned int value = get_external_data(NULL, 1);
		if(value >= 100) {
			break;
		}
		buffer_insert(b, value);
	}
	
	return NULL;
}


int main(int argc, char **argv) {
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

	for(i = 0; i < NUM_READERS; ++i) {
		buffer_insert(&b, -1);
	}

	for(i = 0; i < NUM_READERS; ++i) {
		if(pthread_join(reader_thids[i], NULL)) {
			printf("Problem joining reader #%d\n", i);
		}
	}

	// perform once all threads finish executing
	// sem_destroy(&b.full);
	// sem_destroy(&b.empty);
	// sem_destroy(&b.insert_lock);
	// sem_destroy(&b.remove_lock);

	return 0;	
}