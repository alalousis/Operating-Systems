#include <stdio.h>
#include <stdlib.h>
//#include <sys/types.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <string.h>
//#include <sys/time.h>
#include <time.h>
#include <sys/wait.h>
#include "semun.h"
#include "functions.h"

int main(int argc, char *argv[]){
	int peers, entries, readers, writers, i, j, flag, r, w, iterations;
	float ratio;
	pid_t pid;
	int status, shmid, exit_code, child_pid, segment_size;
	int shm_access, queue_sem, readers_sem;
	struct shmid_ds shmbuffer;
	struct entry firstEntry, secondEntry, *shared_memory;

	if(argc==7){					// Pairnoume ta orismata
		peers = atoi(argv[2]);
		entries = atoi(argv[4]);
		ratio = atof(argv[6]);

	}else{
		printf("Give the number of peers:\n");
		scanf("%d", &peers);
		printf("Give the number of entries:\n");
		scanf("%d", &entries);
		printf("Give the ratio readers/writers:\n");
		scanf("%f", &ratio);
	}
	printf("Peers: %d\tEntries: %d\tReaders/Writers:%f\n", peers, entries, ratio);

	writers = peers/(1+ratio);
	readers = peers-writers;

	printf("Readers: %d\tWriters:%d\n", readers, writers);

	if((shm_access = semget((key_t)1234, 1, 0666 | IPC_CREAT)) == -1){			//Create semaphore for access to shared memory
		printf("Fail in creating semaphore\n");
	}
	if((queue_sem = semget((key_t)5678, 1, 0666 | IPC_CREAT)) == -1){			//Create semaphore for queue access
		printf("Fail in creating semaphore\n");
	}
	if((readers_sem = semget((key_t)6789, 1, 0666 | IPC_CREAT)) == -1){			//Create semaphore for readers counter access
		printf("Fail in creating semaphore\n");
	}

	if ((shmid = shmget(IPC_PRIVATE, (1+entries)*sizeof(struct entry), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR)) == -1){		//Create shared memory
		perror("shmget");
		exit(1);
	}

	set_semvalue(shm_access, 0);
	set_semvalue(queue_sem, 0);
	set_semvalue(readers_sem, 0);

	firstEntry.readings = readers;
	firstEntry.writings = writers;
	secondEntry.readings = 0;
	secondEntry.writings = 0;

	shared_memory = (struct entry*) shmat(shmid, 0, 0);
	memcpy(shared_memory, &firstEntry, sizeof(struct entry));
	memcpy(shared_memory+1, &secondEntry, sizeof(struct entry));
	shmdt (shared_memory);

	for(i=0; i<peers; i++){							//Create peers
		pid = fork();
		sleep(1);
		srand(time(0));

		if(pid==0){
			shared_memory = (struct entry*) shmat(shmid, 0, 0);
			memcpy(&firstEntry, shared_memory, sizeof(struct entry));

			r = firstEntry.readings ;
			w = firstEntry.writings;

			if(r>0 && w>0){
				flag = 2*(rand() / (RAND_MAX + 1.0));
				if (flag == 0){
					r--;
				}
				else if (flag == 1){
					w--;
				}

			}
			else if(pid==0 && r>0 && w==0){
				flag = 0;
				r--;;
			}
			else if(pid==0 && r==0 && w>0){;
				flag = 1;
				w--;
			}

			firstEntry.readings = r;
			firstEntry.writings = w;
			memcpy(shared_memory, &firstEntry, sizeof(struct entry));
			shmdt(shared_memory);
			break;
		}
	}

	if(flag == 0){					//An o peer einai reader
		reader(shmid, entries, shm_access, queue_sem, readers_sem);
	}
	else if(flag == 1){				//An o peer einai writer
		writer(shmid, entries, shm_access, queue_sem);
	}


	if(pid == 0){
		exit(exit_code);
	}
	else if(pid != 0){

		while( (child_pid = wait(&status)>0) ){
			if(WIFEXITED(status)){
				printf("Child exited with code %d\n", WEXITSTATUS(status));
			}
			else{
				printf("Child terminated abnormally\n");
			}
		}

		shared_memory = (struct entry*) shmat(shmid, 0, 0);
		for(i=0; i<entries; i++){
			printf("Entry %d: %d readings, %d writings\n", i, shared_memory[i+2].readings, shared_memory[i+2].writings);
		}

		shmdt(shared_memory);
		del_semvalue(shm_access, 0);
		del_semvalue(queue_sem, 0);
		del_semvalue(readers_sem, 0);
	}

	return 0;
}
