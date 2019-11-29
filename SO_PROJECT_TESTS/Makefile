#***************************** Projeto 3 de SO *****************************#
#                                                                           #
#                           Trabalho realizado por:                         #
# 																			#
#                            												#
#                           David Miranda ist193697                         #
#                           Pedro Marques ist193746                         #
#																			#
#***************************************************************************#

# Makefile, versao 1
# Sistemas Operativos, DEI/IST/ULisboa 2019-20

CC   = gcc
LD   = gcc
CFLAGS =-Wall -pthread -g -lpthread -lrt -std=gnu99 -I../
LDFLAGS=-lm
MUTEX = -DMUTEX
RWLOCK = -DRWLOCK
NOSYNC = -DNOSYNC

# A phony target is one that is not really the name of a file
# https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
.PHONY: all clean run

server: tecnicofs

client: clientfs

clientfs: client.o 
	$(LD) $(CFLAGS) $(LDFLAGS) -o clientfs client.o 

tecnicofs: main.o inodes.o fs.o hash.o bst.o
	$(LD) $(CFLAGS) $(LDFLAGS) -o tecnicofs main.o inodes.o fs.o hash.o bst.o

main.o: main_server.c
	$(CC) $(CFLAGS) -o main.o -c main_server.c 

inodes.o: lib/inodes.c
	$(CC) $(CFLAGS) -o inodes.o -c lib/inodes.c 

fs.o: fs.c
	$(CC) $(CFLAGS) -o fs.o -c fs.c 
hash.o: lib/hash.c
	$(CC) $(CFLAGS) -o hash.o -c lib/hash.c 

bst.o: lib/bst.c
	$(CC) $(CFLAGS) -o bst.o -c lib/bst.c 


client.o : tecnicofs-client-api.c tecnicofs-client-api.h
	$(CC) $(CFLAGS) -o client.o -c tecnicofs-client-api.c

client2.o: tecnicofs-client2-api.c
	$(CC) $(CFLAGS) $(NOSYNC) -o client2.o -c tecnicofs-client2-api.c

clean:
	@echo Cleaning...
	rm -f lib/*.o *.o *.txt tecnicofs client