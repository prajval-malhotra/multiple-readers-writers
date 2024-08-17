//TODO Define global data structures to be used

/**
 * This thread is responsible for pulling data off of the shared data 
 * area and processing it using the process_data() API.
 */
void *reader_thread(void *arg) {
	//TODO: Define set-up required
	
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
	
	while(1) {
		//TODO: Define data extraction (device) and storage 
	}
	
	return NULL;
}


#define M 10
#define N 20
int main(int argc, char **argv) {
	int i;
	
	for(i = 0; i < N; i++) { 
		pthread_create(NULL, NULL, reader_thread, NULL);
	}

	for(i = 0; i < M; i++) { 
		pthread_create(NULL, NULL, writer_thread, NULL);
	}
	

	return 0;	
}