/***************************** Projeto 3 de SO ******************************
*                                                                           *
*                           Trabalho realizado por:                         *
*                                                                           *
*                                                                           *
*                           David Miranda ist193697                         * 
*                           Pedro Marques ist193746                         *
****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <semaphore.h>
#include "tecnicofs-api-constants.h"
#include "tecnicofs-client-api.h"
#include <sys/types.h>
#include <sys/socket.h>
#include "unix.h"

int sockfd, servlen;
struct sockaddr_un serv_addr;
int tfsMount(char * address){
char* message = "MENSAGEM DO CLIENTE";
	/*Cria socket stream */
	if ((sockfd= socket(AF_UNIX, SOCK_STREAM, 0) ) < 0)
		perror("client: can't open stream socket");
	/* Primeiro uma limpeza preventiva */
	bzero((char *) &serv_addr, sizeof(serv_addr));
	/* Dados para o socket stream: tipo + nome que identifica o servidor */
	serv_addr.sun_family = AF_UNIX;
	/*strcpy(serv_addr.sun_path, UNIXSTR_PATH);*/
	strcpy(serv_addr.sun_path, address);
	servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

	/* Estabelece uma ligação. Só funciona se o socket tiver sido criado e o nome associado*/
	if(connect(sockfd, (struct sockaddr *) &serv_addr, servlen) < 0)
		perror("client: can't connect to server");
	printf("%s\n", message );
	send(sockfd , message , strlen(message) , 0);
	/* Envia as linhas lidas do teclado para o socket */
	/*str_cli(stdin, sockfd);*//* Isto esta errado BOIIS*/
	/* Fecha o socket e termina 
	close(sockfd);
	exit(0);*/
	return 0;
}


/*if(connect(sockfd, (struct sockaddr *) &address, servlen) < 0)
		err_dump("client: can't connect to server");*/


int tfsUnmount(){
	/* se calhar chamar shutdown(socket, int how)
	Mas como n receber argumentos n sei*/
}
