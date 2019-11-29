/***************************** Projeto 3 de SO ******************************
*                                                                           *
*                           Trabalho realizado por:                         *
*                                                                           *
*                                                                           *
*                           David Miranda ist193697                         * 
*                           Pedro Marques ist193746                         *
****************************************************************************/
#define _GNU_SOURCE
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
#include <sys/un.h>
#include "unix.h"
#include <assert.h>
#define MAXSIZE 6

int sockfd, servlen;
struct sockaddr_un serv_addr;
struct ucred ucred;
int a;
long clientuid;
int opened_session = 0;

int error(char e[4]){
int i = atoi(e);
switch(i){
    case -1:
        return TECNICOFS_ERROR_OPEN_SESSION;
    case -2:
        return TECNICOFS_ERROR_NO_OPEN_SESSION;
    case -3:
        return TECNICOFS_ERROR_CONNECTION_ERROR;
    case -4:
        return TECNICOFS_ERROR_FILE_ALREADY_EXISTS;
    case -5:
        return TECNICOFS_ERROR_FILE_NOT_FOUND;
    case -6:
        return TECNICOFS_ERROR_PERMISSION_DENIED;
    case -7:
        return TECNICOFS_ERROR_MAXED_OPEN_FILES;
    case -8:
        return TECNICOFS_ERROR_FILE_NOT_OPEN;
    case -9:
        return TECNICOFS_ERROR_FILE_IS_OPEN;
    case -10:
        return TECNICOFS_ERROR_INVALID_MODE;
    case -11:
        return TECNICOFS_ERROR_OTHER;
    default:
        return i;
    }
}

int tfsMount(char * address){
    socklen_t len;
	/*Cria socket stream */
	if ((sockfd= socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
		perror("client: can't open stream socket");
	/* Primeiro uma limpeza preventiva */
	bzero((char *) &serv_addr, sizeof(serv_addr));
	/* Dados para o socket stream: tipo + nome que identifica o servidor */
	serv_addr.sun_family = AF_UNIX;
	/*strcpy(serv_addr.sun_path, UNIXSTR_PATH);*/
	strcpy(serv_addr.sun_path, address);
	servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

	/* Estabelece uma ligação. Só funciona se o socket tiver sido criado e o nome associado*/

	len = sizeof(struct ucred);
        if(getsockopt(sockfd, SOL_SOCKET, SO_PEERCRED, &ucred, &len) == -1)
            perror("ERRO getsockopt");
    clientuid = (long) ucred.uid;

    if (opened_session == 1)
    {
        perror("Sessao do utilizador ja iniciada");
        return TECNICOFS_ERROR_OPEN_SESSION;
    }
    if(connect(sockfd, (struct sockaddr *) &serv_addr, servlen)< 0)
	{
		perror("client: can't connect to server");
		return TECNICOFS_ERROR_CONNECTION_ERROR;
	}
    opened_session = 1;
	return 0;

}

int tfsUnmount(){
	if(close(sockfd) < 0) {
        perror("Unmount: failed!");
        return TECNICOFS_ERROR_NO_OPEN_SESSION;
    }
    opened_session = 0;
    printf("Unmount\n");
    return 0;
}

int tfsCreate(char *filename, permission ownerPermissions, permission othersPermissions)
{
    if(opened_session == 1)
    {
        char buffer[30],erro[4];
        int n_bytes = 0;

        sprintf(buffer,"c %s %d%d",filename, ownerPermissions, othersPermissions);
        write(sockfd,buffer,strlen(buffer));

        while(n_bytes == 0)
            n_bytes = read(sockfd,erro,4);
        if(n_bytes == -1)
            return TECNICOFS_ERROR_OTHER;
        
        return error(erro);
    }
    return TECNICOFS_ERROR_OPEN_SESSION;
}

int tfsDelete(char *filename)
{
    if(opened_session == 1)
    {
        char buffer[30],erro[4];
        int n_bytes = 0;

        sprintf(buffer,"d %s",filename);
        write(sockfd,buffer,strlen(buffer));

        while(n_bytes == 0)
            n_bytes = read(sockfd,erro,4);
         if(n_bytes == -1)
            return TECNICOFS_ERROR_OTHER;
        
        return error(erro);
    }
    return TECNICOFS_ERROR_OPEN_SESSION;
}

int tfsRename(char *oldfilename, char *newfilename)
{
    if(opened_session == 1)
    {
        char buffer[30],erro[4];
        int n_bytes = 0;

        sprintf(buffer,"r %s %s",oldfilename, newfilename);
        write(sockfd,buffer,strlen(buffer));

        while(n_bytes == 0)
            n_bytes = read(sockfd,erro,4);
        
        return error(erro);/*Retorna a fd (descricao do ficheiro aberto)*/
    }
    return TECNICOFS_ERROR_OPEN_SESSION;
}
int tfsOpen(char *oldfilename, permission mode)
{
    if(opened_session == 1)
    {
        char buffer[30],erro[4];
        int n_bytes = 0;

        sprintf(buffer,"o %s %d",oldfilename, mode);
        write(sockfd,buffer,strlen(buffer));

        while(n_bytes == 0)
            n_bytes = read(sockfd,erro,4);
         if(n_bytes == -1)
            return TECNICOFS_ERROR_OTHER;
        return error(erro);
    }
    return TECNICOFS_ERROR_OPEN_SESSION;
}

int tfsClose(int fd)
{
    if(opened_session == 1)
    {
        char buffer[30],erro[4];
        int n_bytes = 0;

        sprintf(buffer,"x %d",fd);
        write(sockfd,buffer,strlen(buffer));

        while(n_bytes == 0)
            n_bytes = read(sockfd,erro,4);
        if(n_bytes == -1)
            return TECNICOFS_ERROR_OTHER;
        return error(erro);
    }
    return TECNICOFS_ERROR_OPEN_SESSION;
}

int tfsRead(int fd, char *buffer_input, int len)
{
    if(opened_session == 1)
    {
        char buffer[len],output[len + sizeof(int)];/*, bytes_lidos[4];*/
        int n_bytes = 0;

        sprintf(buffer,"l %d %d",fd,len);
        printf("buffer: %s \n",buffer);
        write(sockfd,buffer,strlen(buffer));

        while(n_bytes == 0)
            n_bytes = read(sockfd,output,len + sizeof(int));
        if(n_bytes == -1)
            return TECNICOFS_ERROR_OTHER;

        bzero(buffer,30);
        sscanf(output,"%d %s",&n_bytes, buffer);
        printf("output: %s\n", output);

        return n_bytes;
    }
    return TECNICOFS_ERROR_OPEN_SESSION;
}

int main(int argc, char** argv) {
     if (argc != 2) {
        printf("Usage: %s sock_path\n", argv[0]);
        exit(0);
    }
    int putas;
    char buffer[30];
    tfsMount(argv[1]);
    /*tfsMount(argv[1]);*/
    printf("Test: create file sucess\n");
    assert(tfsCreate("a", RW, READ)==0);
    printf("Test: create file with name that already exists\n");
    assert(tfsCreate("a", RW, READ) == TECNICOFS_ERROR_FILE_ALREADY_EXISTS);
    /*assert(tfsRename("a", "b") == 0);
    assert(tfsDelete("a") == 0);*/
    assert((putas = tfsOpen("a",READ)) > 0);
    assert(tfsRead(putas,buffer,16) > 0);
    assert(tfsClose(putas) == 0);

    tfsUnmount();
    /*Unmount();*/

    return 0;
}
