#include "hashmap_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

// we use an extremely simple algorithm with key mod size of the hashmap
int hashmap_get_hash(int key, int size){ 
    return key%size;
}

void clean_ll(LinkedList_t *ll){
    LinkedList_t *current_list=ll, *tmp_ptr;
    while(current_list!=NULL){
        tmp_ptr=current_list;
        current_list=current_list->tail;
        free(tmp_ptr);
    }
}

HashMap_t* hashmap_init(int size){
    HashMap_t* hm;
    void *ptr;
    if(size==0){
        fprintf(stderr, "Please provide a non-zero size for the hashmap\n");
    }
    ptr=malloc(sizeof(HashMap_t)+size*sizeof(LinkedList_t*));
    if(ptr==NULL){
        fprintf(stderr, "Error when allocating hashmap\n");
        return NULL;
    }
    hm=ptr;
    hm->size=size;
    pthread_rwlock_init(&hm->rw_lock, NULL);
    for(int i=0; i<size; i++){ // initialize the buckets
        hm->buckets[i] = NULL;
    }
    return hm;
}

void hashmap_insert(HashMap_t *hm, int key, char* value){
    int pos = hashmap_get_hash(key, hm->size);
    LinkedList_t *new_ll=malloc(sizeof(LinkedList_t));
    new_ll->key=key;
    new_ll->value=value;
    new_ll->tail=NULL;
    pthread_rwlock_wrlock(&hm->rw_lock);
    LinkedList_t *current_list = hm->buckets[pos], *pred=NULL;
    while(current_list!=NULL){
        pred=current_list;
        current_list=current_list->tail;
    }
    if(pred==NULL){
        hm->buckets[pos]=new_ll;
    }else{
        pred->tail=new_ll;
    }
    pthread_rwlock_unlock(&hm->rw_lock);
}

char* hashmap_get(HashMap_t *hm, int key){
    int pos = hashmap_get_hash(key, hm->size);
    pthread_rwlock_rdlock(&hm->rw_lock);
    LinkedList_t *current_list=hm->buckets[pos];
    while(current_list!=NULL && current_list->key != key){
        current_list=current_list->tail;
    }
    if(current_list==NULL){
        pthread_rwlock_unlock(&hm->rw_lock);
        return NULL;
    }else{
        pthread_rwlock_unlock(&hm->rw_lock);
        return current_list->value;
    }
}

void hashmap_delete(HashMap_t *hm, int key){
    int pos = hashmap_get_hash(key, hm->size);
    pthread_rwlock_wrlock(&hm->rw_lock);
    LinkedList_t *current_list, *pred=NULL;
    for(current_list=hm->buckets[pos]; current_list!=NULL; current_list=current_list->tail){
        if(current_list->key!=key){
            if(pred==NULL){ // beginning of the list
                if(hm->buckets[pos]->tail != NULL){
                    hm->buckets[pos]->tail = hm->buckets[pos]->tail->tail;
                }
                hm->buckets[pos]=NULL;
            }else{
                pred->tail=current_list->tail;
            }
        }
        pred=current_list;
    }
    pthread_rwlock_unlock(&hm->rw_lock);
    free(current_list);
}

void hashmap_clean(HashMap_t *hm){
    for(int i=0; i<hm->size; i++){
        clean_ll(hm->buckets[i]);
    }
}

void hashmap_print(HashMap_t *hm){
    pthread_rwlock_wrlock(&hm->rw_lock);
    printf("HM (size : %i)\n", hm->size);
    LinkedList_t *current_list;
    for(int i=0; i<hm->size; i++){
        if(hm->buckets[i]==NULL){
            printf("\t %i bucket empty\n", i);
        }else{
            current_list=hm->buckets[i];
            printf("\t %i bucket -> \n", i);
            while(current_list!=NULL){
                printf("\t (%i : %s)", current_list->key, current_list->value);
                current_list=current_list->tail;
            }
            printf("\n");
        }
    }
    pthread_rwlock_unlock(&hm->rw_lock);
}
