#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <errno.h>
#include <signal.h>

#include "oss.h"

static int shmid = -1;
static int queueID = -1;
static struct shmem *shm = NULL;

static int createSHM(){
	const key_t key = ftok("main.c", shmKey);

	if(key == -1){
		perror("./oss error: key ftok shmKey");
		return -1;
	}

	shmid = shmget(key, sizeof(struct shmem), IPC_CREAT | IPC_EXCL | 0666);
	if(shmid == -1){
		perror("./oss error: shmget shmid");
		return -1;
	}

	shm = (struct shmem *)shmat(shmid, NULL, 0);
	if(shm == (void *) -1){
		perror("./oss error: shmat shm");
		return -1;
	}

	key_t msggerKey = ftok("main.c", queueKey);
	if(msggerKey == -1){
		perror("./oss error: mssgerKey ftok queueKey");
		return -1;
	}

	queueID = msgget(msggerKey, IPC_CREAT | IPC_EXCL | 0666);
	if(queueID == -1){
		perror("./oss error: msgget queueID");
		return -1;
	}

	return 0;
}

int main(void) {
  printf("Hello World\n");
  return 0;
}