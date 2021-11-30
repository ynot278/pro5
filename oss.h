#ifndef OSS_H
#define OSS_H

enum ftokKeys{
	shmKey = 1000, queueKey		 
};

struct shmem{
	struct timespec clock;
	struct userProcess user[MAX_PROCESS];
};
