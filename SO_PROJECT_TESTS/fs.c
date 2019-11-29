/***************************** Projeto 2 de SO ******************************
*                                                                           *
*                           Trabalho realizado por:                         *
*                                                                           *
*                                                                           *
*                           David Miranda ist193697                         * 
*                           Pedro Marques ist193746                         *
****************************************************************************/
#include "fs.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int obtainNewInumber(tecnicofs* fs) {
	int newInumber = ++(fs->nextINumber);
	return newInumber;
}

tecnicofs* new_tecnicofs(int N_buckets){
	int i;
	tecnicofs*fs = malloc(sizeof(tecnicofs));
	if (!fs) {
		perror("failed to allocate tecnicofs");
		exit(EXIT_FAILURE);
		}

	fs -> bstRoot = malloc(sizeof(node)*N_buckets);/*inicializaçao do vetor*/		
	for (i=0; i< N_buckets; i++)
	{
		fs -> bstRoot[i] = NULL;
	}

	fs -> nextINumber = 0;
	
	#ifdef MUTEX/*inicializaçao dos vetores de mutex ou rwlock*/
	fs -> lock = malloc(sizeof(pthread_mutex_t)*N_buckets);	
	for (i=0; i< N_buckets; i++)
	{
		ERROR_VALIDATION(pthread_mutex_init(&(fs -> lock[i]), NULL));	
	}

	#elif RWLOCK
	fs -> lock = malloc(sizeof(pthread_rwlock_t)*N_buckets);	
	for (i=0; i< N_buckets; i++)
	{
		ERROR_VALIDATION(pthread_rwlock_init(&fs -> lock[i], NULL));
	}
	#endif

	return fs;
}

void free_tecnicofs(tecnicofs* fs, int N_buckets){
	int i;
	#ifdef MUTEX/*destroi os mutex e rwlocks do vetor e liberta a memoria alocada pelo vetor*/
	for (i=0; i < N_buckets; i++)
	{
		ERROR_VALIDATION(pthread_mutex_destroy(&fs -> lock[i]));
	}
	free(fs -> lock);
	#elif RWLOCK
	for (i=0; i < N_buckets; i++)
	{
		ERROR_VALIDATION(pthread_rwlock_destroy(&fs -> lock[i]));
	}
	free(fs -> lock);
	#endif
	for (i = 0; i < N_buckets; i++)/*liberta a memoria alocada para a arvore*/
	{
		free_tree(fs->bstRoot[i]);
	}
	free(fs -> bstRoot);
	free(fs);
}

void create(tecnicofs* fs, char *name, int inumber, int N_buckets){
	int h_num = hash(name,N_buckets);

	#ifdef MUTEX/*Utilizamos o trinco do tipo wrlock pois a thread vai escrever na bst um novo node*/
	pthread_mutex_lock(&fs -> lock[h_num]);
	#elif RWLOCK
	pthread_rwlock_wrlock(&fs -> lock[h_num]);
	#endif

	
	fs->bstRoot[h_num] = insert(fs->bstRoot[h_num], name, inumber);

	#ifdef MUTEX
	pthread_mutex_unlock(&fs -> lock[h_num]);
	#elif RWLOCK
	pthread_rwlock_unlock(&fs -> lock[h_num]);
	#endif
}

void renomear(tecnicofs* fs, char *name, char *new_name, int N_buckets){
	int i_number;
	int h_num = hash(name,N_buckets);
	int h_num_new_name = hash(new_name,N_buckets);
	
	#ifdef MUTEX
	int valida_interblocagem = 0;
	while (!valida_interblocagem){
		if (pthread_mutex_trylock(&fs -> lock[h_num]))/*adquisizicao do 1º trinco*/
		{
			if(errno == EBUSY)
			{
				perror("ERRO");
				exit(EXIT_FAILURE);
			}
			if (pthread_mutex_trylock(&fs -> lock[h_num_new_name]))
			{
				if(errno == EBUSY)
			{
				perror("ERRO");
				exit(EXIT_FAILURE);
			}
				
				valida_interblocagem = 1;
			}
			else // adquisição 2º trinco falhou
				pthread_mutex_unlock(&fs -> lock[h_num]); // abre 1ºtrinco e tenta outra vez
		}
	}

	#elif RWLOCK
	int valida_interblocagem = 0;
	while (!valida_interblocagem){
		if (pthread_rwlock_tryrdlock(&fs -> lock[h_num]))/*adquisizicao do 1º trinco*/
		{
			if(errno == EBUSY)
			{
				perror("ERRO");
				exit(EXIT_FAILURE);
			}

			if (pthread_rwlock_tryrdlock(&fs -> lock[h_num_new_name]))
			{
				if(errno == EBUSY)
				{
					perror("ERRO");
					exit(EXIT_FAILURE);
				}
				
				valida_interblocagem = 1;
			}
			else // adquisição 2º trinco falhou
				pthread_rwlock_unlock(&fs -> lock[h_num]); // abre 1ºtrinco e tenta outra vez
		}
	}
	#endif

	node* searchNode = search(fs->bstRoot[h_num], name);
	if (searchNode != NULL)
		i_number = searchNode->inumber;
	else {/*nome n existe*/
		{
			perror("ERRO");
			exit(EXIT_FAILURE);}
	}
	node* searchNode_new = search(fs->bstRoot[h_num_new_name], new_name);

	if (searchNode_new)/*new_name existe*/
	{
		perror("ERRO");
		exit(EXIT_FAILURE);
	}

	fs->bstRoot[h_num] = remove_item(fs->bstRoot[h_num], name);
	fs->bstRoot[h_num_new_name] = insert(fs->bstRoot[h_num_new_name], new_name, i_number);

	#ifdef MUTEX
	pthread_mutex_unlock(&fs -> lock[h_num]);
	pthread_mutex_unlock(&fs -> lock[h_num_new_name]);
	#elif RWLOCK
	pthread_rwlock_unlock(&fs -> lock[h_num]);
	pthread_rwlock_unlock(&fs -> lock[h_num_new_name]);
	#endif
	

}


void delete(tecnicofs* fs, char *name, int N_buckets){
	int h_num = hash(name,N_buckets);

	#ifdef MUTEX/*Utilizamos o trinco do tipo wrlock pois vai remover a bst um node*/
	pthread_mutex_lock(&fs -> lock[h_num]);
	#elif RWLOCK
	pthread_rwlock_wrlock(&fs -> lock[h_num]);
	#endif
	
	fs->bstRoot[h_num] = remove_item(fs->bstRoot[h_num], name);

	#ifdef MUTEX
	pthread_mutex_unlock(&fs -> lock[h_num]);
	#elif RWLOCK
	pthread_rwlock_unlock(&fs -> lock[h_num]);
	#endif
}

int lookup(tecnicofs* fs, char *name, int N_buckets){
	int h_num = hash(name,N_buckets);

	#ifdef MUTEX/*Utilizamos o trinco do tipo wrkock pois vai ler o inumber de uma node se esta existir na bst*/
	pthread_mutex_lock(&fs -> lock[h_num]);
	#elif RWLOCK
	pthread_rwlock_rdlock(&fs -> lock[h_num]);
	#endif

	node* searchNode = search(fs->bstRoot[h_num], name);

	#ifdef MUTEX
	pthread_mutex_unlock(&fs -> lock[h_num]);
	#elif RWLOCK
	pthread_rwlock_unlock(&fs -> lock[h_num]);
	#endif

	if ( searchNode ) return searchNode->inumber;
	return -1;
}

void print_tecnicofs_tree(FILE * fp, tecnicofs *fs, int N_buckets){
	int i;
	for (i= 0; i < N_buckets; i++ ){/*print de todas as arvores*/
		print_tree(fp, fs->bstRoot[i]);
	}
}