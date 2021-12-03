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
#include <sys/time.h>

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

static void removeSHM(){
	if(shm != NULL){
		if(shmdt(shm) == -1){
			perror("./oss error: shm shmdt");
		}
		shm = NULL;
	}

	if(shmid > 0){
		if(shmctl(shmid, IPC_RMID, NULL) == -1){
			perror("./oss error: shmid shmctl");
		}
		shmid = 0;
	}

	if(queueID > 0){
		if(msgctl(queueID, IPC_RMID, NULL) == -1){
			perror("./oss error: queueID msgctl");
		}
		queueID = -1;
	}
}

static void fillDescriptors(struct descriptor oss[descriptorResources]){
	int shareDescriptors = 4;
	while(shareDescriptors > 0){
		const int descriptorIndex = rand() % 20;

		if (oss[descriptorIndex].shareValue == 0){
			oss[descriptorIndex].shareValue = 1;
			shareDescriptors--;
		}
	}

	int i;
	for (i = 0; i < 20; i++){
		if(oss[i].shareValue == 0){
			oss[i].intValue = 1 + (rand() % 10);
		} else {
			oss[i].intValue = 1;
		}
		oss[i].maxValue = oss[i].intValue;
	}
}

void increaseClock(){
	int timeIncrease;

	timeIncrease = (unsigned int) (rand() % 500000000 + 1);
	ossClockptr->nanoseconds += timeIncrease;
}

int main(void) {
  if(createSHM() < 0){
		return EXIT_FAILURE;
	}

	if(shmTest() < 0){
		return EXIT_FAILURE;
	}

	memset(shm, '\0', sizeof(struct shmem));

	alarm(5);

	fillDescriptors(shm->descripArr);
}