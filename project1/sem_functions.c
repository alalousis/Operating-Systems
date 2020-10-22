#include "semun.h"
#include <sys/sem.h>
#include <stdio.h>

int set_semvalue(int sem_id, int sem_num)
{
	union semun sem_union;
	sem_union.val = 1;
	if (semctl(sem_id, sem_num, SETVAL, sem_union) == -1) return(0);
	return(1);
}

void del_semvalue(int sem_id, int sem_num)
{
	union semun sem_union;
	if (semctl(sem_id, sem_num, IPC_RMID, sem_union) == -1)
	fprintf(stderr, "Failed to delete semaphore\n");
}

int sem_down(int sem_id, int sem_num)
{
	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = -1; 
	sem_b.sem_flg = SEM_UNDO;
	if (semop(sem_id, &sem_b, 1) == -1) {
		fprintf(stderr, "semaphore_up failed\n");
		return(0);
	}
	return(1);
}

int sem_up(int sem_id, int sem_num)
{
	struct sembuf sem_b;
	sem_b.sem_num = sem_num;
	sem_b.sem_op = 1;  
	sem_b.sem_flg = SEM_UNDO;
	if (semop(sem_id, &sem_b, 1) == -1) {
		fprintf(stderr, "semaphore_down failed\n");
		return(0);
	}
	return(1);
}
