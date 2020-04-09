# Networked-Spell-Checker

### Overview

This program is a spell checking multithreaded server. The program makes use of networking and socket libraries to establish connection between clients and the server. Through the connection, the client sends a word to the server for spell checking, and the server replies with an "OK" if the word is spelled correctly, or "MISSPELLED". The server is able to perform this function by loading a words file named "dictionary.txt", additionally a log file named "log.txt" is generated containing responses that the server sends to its clients. The purpose of this was to learn more about the producer-consumer problem with multithreaded programming and specifically thread synchronization mechanisms such as mutex (binary semaphore) and condition variables.

### To run the server and client:

1. Open terminal in directory, and run 'make' to run makefile.
2. Run the executable "./server" which defaults to port 8080 and the words file "dictionary.txt". Additionally you can provide your own new-line separated words text file along with a specific port to run the server on- "./server [words file][port number]"
3. Run the executable "./client [host][port]" for a client to communicate with the server. If you're running the server locally the host is 127.0.0.1 (localhost, with default port 8080)
   NOTE: There is also a multithreaded client implementation which can be run by "./multi_client [host][port][input request file]". This would read an input file with words to spell check and utilize multiple threads to send parts of the words as requests to the server.

### Program design

The "main.h" file contains default parameter definitions as well as function headers. Some notable parameters include- DEFAULT_PORT (port number on host), NUM_CLIENTS maximum number of clients to add for queue capacity()), NUM_WORKERS (number of worker threads), DEFAULT_LOG_FILE (name of log file to be generated for the server log), DEFAULT_DICTIONARY (name of dictionary file from which words are read to spell check)

The "utility.c" file contains-

- Implementations for ClientQueue and LogQueue which are used for storing the client socket file descriptors and log events respectively. Queues are implemented as a FIFO linked list datastructures.
- Spell checking helpers: These include functions for reading from file to array and search functions to check correctness of requests. There are 2 search functions- a binary search and a linear search.
- Wrapper functions- some functions that help with brevity throughout the program.

"server.c" program flow-

- main():

1. Initializes mutexes and condition variables.
2. Initializes the shared (thread-unsafe) queues for clients and logs.
3. Reads the words file into an array which will be shared by all worker threads.
4. Establishes socket connection through system calls- socket(), bind(), listen().
5. Creates worker threads, and also creates the logger thread which writes to a log file each time a user request is processed.
6. Enters a while loop which keeps accepting incoming connections and enqueueing them to clients queue until queue capacity is reached. The function of the main thread is to distribute processing for each client to a worker thread, therefore the main thread acts as a single producer (writer) and the worker threads are the consumers (readers).

- enqueue_client(): Once a new connection is accepted, threads are notified by a call to pthread_cond_signal(), for which threads that have pthread_cond_wait() then continue to process. The critical section is accessed after locking the mutex with a call to pthread_mutex_lock()

- dequeue_client(): This function returns a client socket file descriptor in a FIFO policy in a thread safe manner. It waits for the clients queue to have an item, and once there is it dequeues the descriptor and returns it.

- worker_thread(): This is the starter/entry function for the worker threads (consumers). It processes the client request by dequeueing a client and handling it through a call to handle_request_response() which serves the client and writes to the logger queue, thus acting as a producer as well. Finally, after the client requests are handled, it closes the dequeued socket file descriptor associated with that client.

- logger_thread(): This thread continuously writes to a log file as log records and events are added to log_queue.

- allocate_thread_arguments(): Creates a struct with relevant arguments to pass to pass to the worker and logger threads.

### Testing plan

I tested the program on my local machine. This involved running the server, and then setting the client hostname parameter to 'localhost' loopback. Testing was an iterative process during program design. I started by designing and testing a basic single-threaded client and server interaction for the overall user interaction. Then I implemented the data structures and client servicing logic for the spell checking functionality. I tried to test these by doing simple inserts and deletions and observing if the FIFO policy is followed for the queues. For the spell checking piece, I tried to do some sample searches which made me realize that the binary_search feature was buggy for a select few words, due to which I implemented a linear search only when the binary search is unsuccessful hoping that this would ease server load without compromising on results. After this, I implemented the synchronization primitives and multithreaded workers. To test these were working as expected I used gdb debugging sessions at points where there were unexpected behaviors. Then I implemented the logging functionality. Once all the features were implemented, I tested the server by running it on my local machine, and then connecting to it with numerous clients to see if it provides the spell checking service and writes all the responses to a log file.
