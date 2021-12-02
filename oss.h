#ifndef OSS_H
#define OSS_H

#define MAX_PROCESS 18

enum ftokKeys{
	shmKey = 1000, queueKey		 
};

enum processState{
	RDY,
	BLOCK,
	TERMINATE
};

struct userProcess{
	int id, pid;
	enum processState state;
};

struct shmem{
	struct timeval time;
	struct userProcess user[MAX_PROCESS];
};

#endif