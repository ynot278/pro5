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
static int logLines;

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

	if(ossClockshmid > 0){
		if(shmctl(ossClockshmid, IPC_RMID, NULL) == -1){
			perror("./oss error: ossClockshmid shmctl");
		}
		ossClockptr = 0;
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

void increaseClock(unsigned int min, unsigned int max){
	int timeIncrease;
	int overFlow;

	timeIncrease = (unsigned int) (rand() % max + min);
	if ((ossClockptr->nanoseconds + timeIncrease) > 1000000000){
		overFlow = (ossClockptr->nanoseconds + timeIncrease) - 1000000000;
		ossClockptr->nanoseconds = overFlow;
		ossClockptr->seconds += 1;
	} else ossClockptr->nanoseconds += timeIncrease;
}

static void initProcesses(struct userProcess up[MAX_PROCESS]){
	int i;
	for (i = 0; i < MAX_PROCESS; i++){
		up[i].resource = (rand() % 5);
		up[i].state = RDY;
	}
}

static void askforResources(struct userProcess up[MAX_PROCESS]){
	int i;
	for (i = 0; i < MAX_PROCESS; i++){
		if(up[i].state == RDY || up[i].state == WAIT){
			up[i].resource = (rand() % 5);
			up[i].state = RDY;
		}
	}
}

static void printTable(struct userProcess up[MAX_PROCESS]){
	int i;
	printf("R0\tR1\tR2\tR3\tR4\tR5");
	for (i = 0; i < MAX_PROCESS; i++){
		if(up[i].allowed == 1){
			printf("P%d", i);
		}
	}
}

static void runSim(struct descriptor oss[descriptorResources], struct userProcess up[MAX_PROCESS]){
	int i;
	int shouldTerminate;
	int resourceRelease;
	int granted;
	
	for(i = 0; i < MAX_PROCESS; i++){
		increaseClock(1, 500000000);
		if(up[i].state == RDY){
			printf("Master has detected Proceess P%d requesting R%d at time %d:%d\n", i, up[i].resource, ossClockptr->seconds, ossClockptr->nanoseconds);
			printf("Master running deadlock detection at time %d:%d\n", ossClockptr->seconds, ossClockptr->nanoseconds);
			logLines += 2;

			if (up[i].resource < oss[i].intValue){
				printf("\tSafe state after granting reqeust\n\tMaster granting P%d request R%d at time %d:%d\n", i, up[i].resource, ossClockptr->seconds, ossClockptr->nanoseconds);

				logLines += 1;
			  granted += 1;
				
				oss[i].intValue -= up[i].resource;
				up[i].allowed = 1;

			} else{
				printf("\tProcess P%d deadlocked\n\tUnsafe state after granting request; request not granted\n\tP%d added to wait queue\n", i, i);
				logLines += 1;
				up[i].state = WAIT;
			}
		}

		increaseClock(0, 250000000);
		shouldTerminate = (rand() % 100 + 1);

		if (shouldTerminate > 80){
			printf("Process P%d terminated\n", i);
			logLines += 1;
			up[i].state = TERMINATE;
			shm->userCount -= 1;
		} else{
			printf("Master has acknowledged Process P%d realeasing R%d at time %d:%d\n", i, up[i].resource, ossClockptr->seconds, ossClockptr->nanoseconds);
			resourceRelease = (rand() % oss[i].maxValue);
			printf("\tResources released: R%d\n", resourceRelease);
			logLines += 2;
		}

		if(granted == 20){
			granted = 0;
			//printTable();
		}

	}
}

static void catchSignalHandler(int sig)
{
  removeSHM();
}

int main(void) {
	time_t t;
  srand((unsigned) time(&t));

	signal(SIGINT, catchSignalHandler);
  signal(SIGTERM, catchSignalHandler);
	
  if(createSHM() < 0){
		return EXIT_FAILURE;
	}

	if(shmTest() < 0){
		return EXIT_FAILURE;
	}

	stdout = freopen("logfile.txt", "w,", stdout);
	if(stdout == NULL){
		perror("freopen failed:");
		return -1;
	}

	memset(shm, '\0', sizeof(struct shmem));

	alarm(5);

	fillDescriptors(shm->descripArr);
	initProcesses(shm->user);

	while (shm->userCount < MAX_PROCESS && logLines < 1000){
		runSim(shm->descripArr, shm->user);
		askforResources(shm->user);
		shm->userCount += 1;
	}

	removeSHM();
}