#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#include "oss.h"

static int shmid = -1;
static int qid = -1;
static struct shmem *shm = NULL;
static struct userProcess *user = NULL;

static struct shmem *createSHM(){
	struct shmem *shm = NULL;

	key_t key = ftok("main.c", shmKey);
	if (key == -1){
		perror("./user.c error: key ftok shmKey");
		return NULL;
	}

	shmid = shmget(key, sizeof(struct shmem), 0);
	if (shmid == -1){
		perror("./user.c error: shmget");
		return NULL;
	}

	shm = (struct shmem *)shmat(shmid, NULL, 0);
	if (shm == (void *)-1){
		perror("./user.c error: shmat");
		return NULL;
	}

	key = ftok("main.c", queueKey);
	if (key == -1){
		perror("./user.c error: queue ftok key queue");
		return NULL;
	}

	qid = msgget(key, 0);
	if (qid == -1){
		perror("./user.c error: msgget");
		return NULL;
	}

	return shm;
}

static int removeSHM(struct shmem *shm){
	if(shmdt(shm) == -1){
		perror("./user.c error: shmdt shm");
		return -1;
	}
	return 0;
}

int main(const int argc, char *const argv[]){
	if (argc != 2){
		perror("user.c error: Args: ./user\n");
		return EXIT_FAILURE;
	}

	if(createSHM() < 0){
		return EXIT_FAILURE;
	}

















}