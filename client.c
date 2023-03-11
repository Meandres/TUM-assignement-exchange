#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <semaphore.h>
#include <error.h>
#include <sys/mman.h>
#include "structs.h"
#include "hashmap_utils.h"

#define SHM_KEY "/shm_order_"
#define NB_THREADS 4
#define SIZE_SHM sizeof(Order_t)
#define NB_OP 5

int things_to_do[NB_OP] = {INSERT, INSERT, GET, DELETE, INSERT};

void *client_thread(void *param){
    int tid = (int)(intptr_t)param;
    int fd_shm;
    char thread_name[5], shm_name[20], tmp[5];
    sprintf(thread_name, "T%i", tid);
    sprintf(shm_name, "%s%i", SHM_KEY, tid);
    Order_t *order;
    
    if ((fd_shm = shm_open (shm_name, O_RDWR, 0)) == -1)
        perror("shm_open");

    if ((order = mmap (NULL, SIZE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0)) == MAP_FAILED)
       perror ("mmap");
    
    printf("%s initialised\n", thread_name);
    for(int i=0; i<NB_OP+1; i++){
        pthread_mutex_lock(&order->serv_mut);
        if(i==NB_OP){
            order->type=FINISH;
        }else{
            switch(things_to_do[i]){
                case INSERT:
                    order->key=i*10+tid;
                    strcpy(order->value, thread_name);
                    order->type=INSERT;
                    break;
                case GET:
                    order->key=(i-2)*10+tid;
                    order->type=GET;
                    break;
                case DELETE:
                    order->key=(i-2)*10+tid;
                    order->type=DELETE;
                    break;
            }
        }
        pthread_cond_signal(&order->serv_cond);
        pthread_mutex_unlock(&order->serv_mut);
        pthread_mutex_lock(&order->client_mut);
        while(order->type!=ACK){
            pthread_cond_wait(&order->client_cond, &order->client_mut);
        }
        pthread_mutex_unlock(&order->client_mut);
        if(things_to_do[i]==GET){
            strcpy(tmp, order->value);
            printf("Thread %i : Read %s @ %i\n", tid, tmp, (i-2)*10+tid);
        }
    }
    return 0;
}

int main (int argc, char **argv)
{
    pthread_t threads[NB_THREADS];
    
    for(int i=0; i<NB_THREADS; i++){
        pthread_create(&threads[i], NULL, client_thread, (void*)(intptr_t)i);
    }

    for(int i=0; i<NB_THREADS; i++){
        pthread_join(threads[i], NULL);
    }       
}
