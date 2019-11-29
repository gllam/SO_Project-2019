#define _GNU_SOURCE
#define MAX_SIZE_ARG 1000
#define MAX_SIZE_BUFFER 2000
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <pthread.h>
#include "tecnicofs-api-constants.h"
#include "tecnicofs-client-api.h"
#include "lib/inodes.h"
#include "lib/bst.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h> 
#include <fcntl.h>
#include "fs.h"
#include "unix.h"

tecnicofs* fs;
int N_buckets = 1;

int checkPermissions(int permissions, uid_t owner,uid_t uid,permission getOwnerPermissions,permission getOtherPermissions)
{
    if((long)owner == (long)uid)
    {
        if(permissions == 3 && (long)getOwnerPermissions != 0)
            return getOwnerPermissions;/* True*/
        else if((long)getOwnerPermissions == 3 && permissions != 0 )
            return permissions;/* True*/
        else if((permissions == (long)getOwnerPermissions) && (permissions != 0))
            return permissions;
        return 0;
            
    }
    else
    {
        if(permissions == 3 && (long)getOtherPermissions != 0)
            return getOtherPermissions;/* True*/
        else if((long)getOtherPermissions == 3 && permissions != 0 )
            return permissions;/* True*/
        else if((permissions == (long)getOtherPermissions) && (permissions != 0))
            return permissions;
        return 0;
    }
}

/****************************** applyCommands  *******************************
*                                                                            *
*   input -> none                                                            *
*   output -> void                                                           *
*                                                                            *
*   Descricao:                                                               * 
*       Funcao que aplica os comandos vindos do input vetor de comandos                                                   *
*****************************************************************************/

void *applyCommands(void *socket){

    struct ucred ucred;
    int success;
    char buffer[MAX_SIZE_BUFFER] = {0},token,arg1[MAX_SIZE_ARG],arg2[MAX_SIZE_ARG],fd_buffer[4];
    int ownerPermissions,otherPermissions,iNumber;
    socklen_t len;
    int newsockfd = *((int *) socket);
    int n_bytes = 0; 

    len = sizeof(struct ucred);
    if(getsockopt(newsockfd, SOL_SOCKET, SO_PEERCRED, &ucred, &len) == -1)
            perror("ERRO getsockopt");
    uid_t owner;
    permission getOwnerPermissions;
    permission getOtherPermissions;
    char* arrayOpenFiles[5];
    int permission,i,access_granted = 0;
    int fd,fd_given,fd_file, caracteres_lidos,lenfd,acabou=0;

    for(i=0;i<5;i++)
    {
        arrayOpenFiles[i] = malloc(MAX_SIZE_ARG + 3);/*Positive integer plus a blank space*/
        memset(arrayOpenFiles[i],0,MAX_SIZE_ARG + 3);
    }



    /*memset(arrayOpenFiles,0,5*sizeof(arrayOpenFiles[0]));*/


    while(1){

        bzero(buffer,MAX_SIZE_BUFFER); 
        printf("Conectou gostoso\n");
        n_bytes = 0;
        success = 0;

        while(n_bytes == 0)
            n_bytes = read(newsockfd,buffer, 30);
        if(n_bytes == -1)
            return NULL;

        sscanf(buffer,"%c %s %s", &token, arg1, arg2);
        printf("%s\n", buffer);

        switch (token) {
            case 'c':
                ownerPermissions = atoi(arg2) / 10;
                otherPermissions = atoi(arg2) % 10;

                iNumber = lookup(fs,arg1,N_buckets);
                if(iNumber != -1) {
                    write(newsockfd,"-4",3);
                    break;
                }
                
                iNumber = inode_create(ucred.uid,ownerPermissions,otherPermissions);

                if (iNumber == -1){
                    write(newsockfd,"-11",4);
                    break;
                }

                success = inode_set(iNumber, arg1, strlen(arg1));

                if (success != 0){
                    write(newsockfd,"-11",4);
                    break;
                }

                create(fs, arg1, iNumber,N_buckets);
                write(newsockfd,"0",2);
                break;

            case 'd':

                iNumber = lookup(fs,arg1,N_buckets);
                if(iNumber == -1) {
                    write(newsockfd,"-5",3);
                    break;
                }

                inode_get(iNumber,&owner,NULL,NULL,NULL,0);

                if((long)owner != (long)ucred.uid)
                {
                    write(newsockfd,"-6",3);
                    break;
                }

                success = inode_delete(iNumber);

                if (success != 0 ){
                    write(newsockfd,"-11",3);
                    break;
                }

                delete(fs, arg1, N_buckets);
                write(newsockfd,"0",2);
                break;

            case 'r':
                iNumber = lookup(fs,arg2,N_buckets);
                if(iNumber != -1) {
                    write(newsockfd,"-4",3);
                    break;
                }

                iNumber = lookup(fs,arg1,N_buckets);
                if(iNumber == -1) {
                    write(newsockfd,"-5",3);
                    break;
                }

                inode_get(iNumber,&owner,NULL,NULL,NULL,0);

                if((long)owner != (long)ucred.uid)
                {
                    write(newsockfd,"-6",3);
                    break;
                }

                if(inode_set(iNumber,arg2,strlen(arg2)) != 0)
                {   
                    write(newsockfd,"-11",4);
                    break;
                }                
                renomear(fs, arg1, arg2, N_buckets);
                write(newsockfd,"0",2);
                break;
        
            case 'o':
                acabou = 0;
                permission = atoi(arg2);
                if(strcmp(arrayOpenFiles[4],"")!= 0)
                {
                    write(newsockfd,"-7",3);
                    break;
                }

                iNumber = lookup(fs,arg1,N_buckets);
                if(iNumber == -1) {
                    write(newsockfd,"-5",3);
                    break;
                }

                inode_get(iNumber,&owner,&getOwnerPermissions,&getOtherPermissions,NULL,0);
                
                access_granted = checkPermissions(permission,owner,ucred.uid,getOwnerPermissions,getOtherPermissions);                  
                
                if (access_granted != 0)/* True*/
                    {
                    switch(access_granted){
                        case(1):
                            fd = open(arg1, O_WRONLY | O_CREAT );
                            break;
                        case(2):
                            fd = open(arg1, O_RDONLY | O_CREAT);
                            break;
                        case(3):
                            fd = open(arg1, O_RDWR | O_CREAT );
                            break;
                        default:{
                            write(newsockfd,"-11",4);
                            acabou = 1;
                            break;
                            }
                        }
                        if (acabou == 1)
                            break;
                        printf("fd: %d\n",fd);
                        for(i=0;i<5;i++)
                        {      
                            if(strcmp(arrayOpenFiles[i],"")== 0){
                             sprintf(arrayOpenFiles[i],"%d %d %s",fd,permission,arg1);
                                break; /*end the for loop*/
                            }
                        }
                        sprintf(fd_buffer,"%d",fd);
                        write(newsockfd,fd_buffer,2);/*Tamanho de inteiros positivos*/
                        break;
                    }
                write(newsockfd,"-6",3);/*ERRO DONT HAVE PERMISSION*/
                break;

            case 'x':
                acabou = 0;
                fd_given = atoi(arg1);
                for(i=0;i<5;i++)
                {
                    sscanf(arrayOpenFiles[i],"%d",&fd_file);
                    if(fd_file == fd_given)
                    {
                        close(fd_file);
                        strcpy(arrayOpenFiles[i],"");
                        printf("cona\n");
                        write(newsockfd,"0",2);
                        acabou = 1;
                        break;
                    }
                }
                if(acabou == 1)
                    break;
                printf("i: %d\n",i );
                write(newsockfd,"-8",3);
                break;

            case 'l':
                acabou = 0;
                bzero(buffer,MAX_SIZE_BUFFER);
                lenfd = atoi(arg2) - 1;
                printf("lenfd: %d",lenfd);
                fd_given = atoi(arg1);
                for(i=0;i<5;i++)
                {
                    sscanf(arrayOpenFiles[i],"%d %d",&fd_file,&permission);
                    printf("permission: %d\n",permission);
                    if(fd_file == fd_given)
                    {
                        if(permission >= 2)
                        {
                            printf("%d\n", fd_file);
                            caracteres_lidos = read(fd_file,buffer,lenfd);
                            printf("%s\n", buffer);
                            printf("caracterres lidos: %d\n", caracteres_lidos);
                            sprintf(fd_buffer,"%d ", caracteres_lidos);/* reutilizacao de memoria*/

                            strcat(fd_buffer,buffer);
                            printf("fd_buffer: %s\n", fd_buffer );
                            lenfd += sizeof(int);
                            write(newsockfd,fd_buffer,lenfd);
                            acabou = 1;
                            break;
                        }
                    }
                }
                if(acabou == 1)
                    break;
                write(newsockfd,"-8",3);
                break;
            default:{

                exit(EXIT_FAILURE);
            }
        }
    }
    return NULL;
}
int main(int argc, char* argv[])
 {
    int sockfd, newsockfd,servlen, N_threads,i;
    struct sockaddr_un cli_addr, serv_addr;
    pthread_t tid[6];
    socklen_t clilen;
    N_threads = 0;
    fs = new_tecnicofs(N_buckets);
    int *arg = malloc(sizeof(*arg));
    inode_table_init();

    /* Cria socket stream */
    if ((sockfd = socket(AF_UNIX,SOCK_STREAM,0) ) < 0)
        perror("server: can't open stream socket");
    /* Elimina o nome, para o caso de já existir.*/
        unlink(argv[1]);
    /* O nome serve para que os clientes possam identificar o servidor */
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, argv[1]);
    servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, servlen) < 0)
        perror("server, can't bind local address");
    listen(sockfd,5);

    for(;;)
    {
        clilen = sizeof(cli_addr);
       
        /* Accept actual connection from the client */
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0) {
          perror("server: accept error");
          exit(EXIT_FAILURE);
        }

        *arg = newsockfd;
        pthread_create(&tid[N_threads], 0, (void *)applyCommands, arg);
        N_threads ++;
        
    }
        for(i=0; i < N_threads ; i++)
        {
            pthread_join(tid[i], NULL); 
        }
        free(arg);
        free_tecnicofs(fs,N_buckets);
        printf("Acabei o programa\n");

    return 0;
 }
