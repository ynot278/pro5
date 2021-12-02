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

struct shmem{
	struct timeval time;
	struct userProcess user[MAX_PROCESS];
	struct descriptor descripArr[descriptorResources];
};

#endif