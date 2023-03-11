# Assignement hashmap + shared memory

A Makefile is provided which builds two executables "server" and "client"

## To run

simply run the server executable and then the client executable in a new shell.


## Design

The server spawns a number of threads (as much as the client threads) and creates a shared memory region per thread. Each thread then is synchronized with the corresponding client thread with 2 condition variables (and their associated locks). The client threads can give orders to the server thread which will be handled and forwarded to the hashmap. Once they are done, they can signal the server thread using a special type of order (FINISH). The server thread will then finish.

The HashMap is implemented using an array of linked lists as buckets. A reader-writer lock is used to synchronize access to the hashmap.

Performance wise, I chose to trade memory for speed. Having one shared memory region means that each thread operates independantly at the cost of using more memory. Regarding the condition variables, I think the one the server waits onto is mandatory as the server might be left for long times without orders from the client. On the other side, one could argue that the condition variable the client waits on is less necessary since we could want the client to be tightly coupled to the server. However, I added this other condition variable because I wanted to avoid as much busy-waiting as possible.


