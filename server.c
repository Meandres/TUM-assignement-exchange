#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <error.h>
#include <sys/mman.h>
#include <pthread.h>
#include "structs.h"
#include "hashmap_utils.h"

#define SHM_KEY "/shm_order_"
#define NB_THREADS 4
#define SIZE_SHM sizeof(Order_t)

HashMap_t *hm;

void *server_thread(void *param){
    int tid = (int)(intptr_t)param;
    int fd_shm;
    pthread_mutexattr_t serv_attr_mutex;
    pthread_condattr_t serv_attr_cond;
    pthread_mutexattr_t client_attr_mutex;
    pthread_condattr_t client_attr_cond;
    char shm_name[20];
    char tmp[50];
    Order_t *order;
    sprintf(shm_name, "%s%i", SHM_KEY, tid);
    printf("%s\n", shm_name);
    if ((fd_shm = shm_open(shm_name, O_RDWR | O_CREAT | O_TRUNC, 0660)) == -1)
        perror("shm_open");

    if (ftruncate(fd_shm, SIZE_SHM) == -1)
       perror("ftruncate");
    
    if ((order = (Order_t*)mmap(NULL, SIZE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0)) == MAP_FAILED)
       perror ("mmap");
    
    pthread_mutexattr_init(&serv_attr_mutex);
    pthread_mutexattr_setpshared(&serv_attr_mutex, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&order->serv_mut, &serv_attr_mutex);
    pthread_condattr_init(&serv_attr_cond);
    pthread_condattr_setpshared(&serv_attr_cond, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(&order->serv_cond, &serv_attr_cond);
    
    pthread_mutexattr_init(&client_attr_mutex);
    pthread_mutexattr_setpshared(&client_attr_mutex, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&order->client_mut, &client_attr_mutex);
    pthread_condattr_init(&client_attr_cond);
    pthread_condattr_setpshared(&client_attr_cond, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(&order->client_cond, &client_attr_cond);
    
    printf("Thread %i initialised\n", tid);

    int keep=1;
    int l_key=-1;
    char *l_value=NULL;
    order->type=ACK;
    int op=0;
    while(keep==1){
        pthread_mutex_lock(&order->serv_mut);
        hashmap_print(hm);
        while(order->type==ACK){
            pthread_cond_wait(&order->serv_cond, &order->serv_mut);
        }
        pthread_mutex_unlock(&order->serv_mut);
        printf("Thread %i : op %i\n", tid, op);
        pthread_mutex_lock(&order->client_mut);
        switch(order->type){
            case INSERT:
                strcpy(tmp, order->value);
                hashmap_insert(hm, order->key, tmp);
                l_key=-1;
                l_value=NULL;
                break;
            case GET:
                l_value=hashmap_get(hm, order->key);
                l_key=-1;
                break;
            case DELETE:
                hashmap_delete(hm, order->key);
                l_key=-1;
                l_value=NULL;
                break;
            case FINISH:
                keep=0;
                break;
        }
        order->key=l_key;
        if(l_value!=NULL){
            strcpy(order->value, l_value);
        }else{
            order->value[0]='\0';
        }
        order->type=ACK;
        op++;
        pthread_cond_signal(&order->client_cond);
        pthread_mutex_unlock(&order->client_mut);
    }
    return 0;
}

int main(int argc, char **argv)
{
    int hm_size=10;
    pthread_t threads[NB_THREADS];
    
    hm=hashmap_init(hm_size);

    
    for(int i=0; i<NB_THREADS; i++){
        pthread_create(&threads[i], NULL, server_thread, (void*)(intptr_t)i);
    }

    for(int i=0; i<NB_THREADS; i++){
        pthread_join(threads[i], NULL);
    }
}
