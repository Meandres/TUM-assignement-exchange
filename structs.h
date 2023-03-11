#ifndef STRUCTS_H
#define STRUCTS_H

#include <pthread.h>

struct LinkedList {
    int key;
    char* value;
    struct LinkedList* tail;
};

struct HashMap {
    int size;
    pthread_rwlock_t rw_lock;
    struct LinkedList *buckets[];  
};

enum OrderType { INSERT, GET, DELETE, ACK, FINISH };

struct Order {
  enum OrderType type;
  int key;
  char value[50];
  pthread_mutex_t serv_mut;
  pthread_cond_t serv_cond;
  pthread_mutex_t client_mut;
  pthread_cond_t client_cond;
};

typedef struct HashMap HashMap_t;
typedef struct LinkedList LinkedList_t;
typedef struct Order Order_t;

#endif
