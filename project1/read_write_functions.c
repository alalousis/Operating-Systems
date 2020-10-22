#include <sys/shm.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include "functions.h"
#include "semun.h"

void writer(int shmid, int entries, int shm_access, int queue_sem){
	int entry;
	struct entry temp, *shared_memory;

	srand(time(0));
	entry = entries*(rand() / (RAND_MAX + 1.0));			//entry to be modified

	sem_down(queue_sem, 0);
	sem_down(shm_access, 0);
	sem_up(queue_sem, 0);

	shared_memory = (struct entry*) shmat(shmid, 0, 0);
	memcpy(&temp, shared_memory+2+entry, sizeof(struct entry));		//+2 is because shared_memory[0],shared_memory[1] contain counters

	/*** Perform writing ***/
	temp.writings++;
	memcpy(shared_memory+2+entry, &temp, sizeof(struct entry));

	shmdt(shared_memory);
	sem_up(shm_access, 0);
}


void reader(int shmid, int entries, int shm_access, int queue_sem, int readers_sem){
	int entry;
	struct entry temp, *shared_memory;

	srand(time(0));
	entry = entries*(rand() / (RAND_MAX + 1.0));		//entry to be read

	sem_down(queue_sem, 0);
	sem_down(readers_sem, 0);

	shared_memory = (struct entry*) shmat(shmid, 0, 0);
	memcpy(&temp, shared_memory+1, sizeof(struct entry));
	if(temp.readings == 0){
		sem_down(shm_access, 0);
	}
	temp.readings++;
	memcpy(shared_memory+1, &temp, sizeof(struct entry));

	sem_up(queue_sem, 0);
	sem_up(readers_sem, 0);


	/*** Perofrm reading ***/
	memcpy(&temp, shared_memory+2+entry, sizeof(struct entry));	//+2 is because shared_memory[0],shared_memory[1] contain counters
	temp.readings++;
	memcpy(shared_memory+2+entry, &temp, sizeof(struct entry));

	sem_down(readers_sem, 0);

	memcpy(&temp, shared_memory+1, sizeof(struct entry));
	temp.readings--;
	if(temp.readings == 0){
		sem_up(shm_access, 0);
	}
	memcpy(shared_memory+1, &temp, sizeof(struct entry));

	sem_up(readers_sem, 0);
	shmdt(shared_memory);
}
