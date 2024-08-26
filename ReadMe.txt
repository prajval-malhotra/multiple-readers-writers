- used semaphores for mutual exclusion and synchronization 
    - all sempahores belong to the buffer structure itself and are only accessed by the buffer insert and remove operations
    - the buffer itself is a fixed size ring buffer
    - a single semaphore value corresponds to an empty or full space on the shared buffer
        - a thread must acquire 1 semaphore per spot of the buffer they want to access
        - at init state, there are BUFFER_SIZE empty semaphores as the buffer is empty, and 0 full semaphores,

To clone and run the program - 

- git clone https://github.com/prajval-malhotra/multiple-readers-writers.git
- cd multiple-readers-writers
- make clean && make
- ./exec


Sample output - 
![alt text](https://drive.google.com/file/d/1h-yePMQecVSKHxF3qS2fB9Tu2_9zduSv/view?usp=sharing)
