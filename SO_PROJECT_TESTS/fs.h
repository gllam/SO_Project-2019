/***************************** Projeto 2 de SO ******************************
*                                                                           *
*                           Trabalho realizado por:                         *
*                                                                           *
*                                                                           *
*                           David Miranda ist193697                         * 
*                           Pedro Marques ist193746                         *
****************************************************************************/
#ifndef FS_H
#define FS_H
#include "lib/bst.h"
#include <pthread.h>
#include <semaphore.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h> 
#include "lib/hash.h"

//macros
#define ERROR_VALIDATION(erro) \
;if (erro != 0){\
 perror("ERRO");\
 exit(EXIT_FAILURE);\
}

#define SEM_DESTROY(SEM)  ERROR_VALIDATION(sem_destroy(&SEM))
#define SEM_INIT(SEM,TICKETS) ERROR_VALIDATION(sem_init(&SEM,0,TICKETS))
#define SEM_WAIT(SEM) ERROR_VALIDATION(sem_wait(&SEM))
#define SEM_POST(SEM) ERROR_VALIDATION(sem_post(&SEM))
#ifdef MUTEX
#define DECLARE_LOCK(L) pthread_mutex_t L
#define INIT_LOCK(L) ERROR_VALIDATION(pthread_mutex_init(&L, NULL))
#define LOCK_DESTROY(L) ERROR_VALIDATION(pthread_mutex_destroy(&L)) /*RWLOCK faz mutex norma. RD lock faz mutex normal*/
#define WRLOCK_LOCK(L) ERROR_VALIDATION(pthread_mutex_lock(&L))
#define RDLOCK_LOCK(L) ERROR_VALIDATION(pthread_mutex_lock(&L))
#define WRLOCK_UNLOCK(L) ERROR_VALIDATION(pthread_mutex_unlock(&L))
#define RDLOCK_UNLOCK(L) ERROR_VALIDATION(pthread_mutex_unlock(&L))

 
#elif RWLOCK
#define DECLARE_LOCK(L) pthread_rwlock_t L
#define INIT_LOCK(L) ERROR_VALIDATION(pthread_rwlock_init(&L, NULL))
#define LOCK_DESTROY(L)  ERROR_VALIDATION(pthread_rwlock_destroy(&L)) /*RWLOCK, vai dar wrlock, RDLOCK vai dar rd lock*/
#define WRLOCK_LOCK(L) ERROR_VALIDATION(pthread_rwlock_wrlock(&L))
#define RDLOCK_LOCK(L) ERROR_VALIDATION(pthread_rwlock_rdlock(&L))
#define WRLOCK_UNLOCK(L) ERROR_VALIDATION(pthread_rwlock_unlock(&L))
#define RDLOCK_UNLOCK(L) ERROR_VALIDATION(pthread_rwlock_unlock(&L))

#else
#define DECLARE_LOCK(L) 
#define INIT_LOCK(L)
#define LOCK_DESTROY(L)
#define WRLOCK_LOCK(L) 
#define RDLOCK_LOCK(L) 
#define WRLOCK_UNLOCK(L) 
#define RDLOCK_UNLOCK(L) 
#endif


typedef struct tecnicofs {
    node** bstRoot; /*vetor de nodes que indicam a raiz de cada bucket*/
    int nextINumber;
    #ifdef MUTEX
    pthread_mutex_t *lock;/*vetor de mutex*/
    #elif RWLOCK
    pthread_rwlock_t *lock;/*vetor de rwlock*/
    #endif
} tecnicofs;



int obtainNewInumber(tecnicofs* fs);
tecnicofs* new_tecnicofs(int N_buckets);
void free_tecnicofs(tecnicofs* fs, int N_buckets);
void create(tecnicofs* fs, char *name, int inumber, int N_buckets);
void renomear(tecnicofs* fs, char *name,char *new_name, int N_buckets);
void delete(tecnicofs* fs, char *name, int N_buckets);
int lookup(tecnicofs* fs, char *name, int N_buckets);
void print_tecnicofs_tree(FILE * fp, tecnicofs *fs, int N_buckets);

#endif /*FS_H*/