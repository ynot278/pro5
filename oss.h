#ifndef OSS_H
#define OSS_H

#define MAX_PROCESS 18
#define descriptorResources 20

enum ftokKeys{
	shmKey = 1000, queueKey		 
};

enum processState{
	RDY,
	BLOCK,
	TERMINATE
};

struct descriptor{
	int shareValue;
	int intValue;
	int maxValue;
};

struct userProcess{
	int id, pid;
	enum processState state;
	struct descriptor descripArr[descriptorResources];
};

typedef struct ossClock{
	unsigned int seconds;
	unsigned int nanoseconds;
} ossClock;

ossClock *ossClockptr;
int ossClockshmid;

static int shmTest(){
	ossClockshmid = shmget(8837, sizeof(ossClock), IPC_CREAT | IPC_EXCL | 0666);
	if(ossClockshmid == -1){
		perror("error: shmget ossClockshmid");
		return -1;
	}

	ossClockptr = shmat(ossClockshmid, NULL, 0);
	if(ossClockptr == (void *) -1){
		perror("error: shmat ossClockptr");
		return -1;
	}
}

struct shmem{
	struct userProcess user[MAX_PROCESS];
	struct descriptor descripArr[descriptorResources];
};

#endif