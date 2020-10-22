#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include "structs.h"
#include "functions.h"

int main(int argc, char **argv){
    int i, j, c, M, n, *matrix, counter, readers;
    pid_t pid, childID;
    int status, shmid, child_pid, shm_access, readers_sem;
    struct entry *shared_memory, tmpEntry;
    time_t t, timestamp, curTime, start, end, total_time;
    struct timeval tv;
    float *runningAverage;
    char path[32], id[8];
    FILE *fd;

    M = atoi(argv[1]);
    n = atoi(argv[2]);

    runningAverage = malloc(n*sizeof(float));
    for(i=0; i<n; i++){
        runningAverage[i] = 0;
    }

    if ((shmid = shmget(IPC_PRIVATE, sizeof(struct entry), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR)) == -1){		//Create shared memory
		perror("shmget");
		exit(1);
	}

    if((shm_access = semget((key_t)1234, 1, 0666 | IPC_CREAT)) == -1){			//Create semaphore for access in shared memory
		printf("Fail in creating semaphore\n");
	}
    if((readers_sem = semget((key_t)1234, 1, 0666 | IPC_CREAT)) == -1){			//Create semaphore for readers queue
		printf("Fail in creating semaphore\n");
	}
    set_semvalue(shm_access, 0, 1);
    set_semvalue(readers_sem, 0, 1);

    readers = 0;
    counter = 0;
    for(c=0; c<n; c++){
        if( (pid=fork()) <= 0){
            break;
        }
    }

    if(pid == 0){    //child
        strcpy(path, "output");
        childID = getpid();
        sprintf(id, "%d", childID);
        strncat(path, id, strlen(id));
        strncat(path, ".txt", 4);

        fd = fopen(path, "w");
        sleep(1);
        while(counter < M){
        	sem_down(readers_sem, 0);
            readers++;
            if(readers == 1){           //first reader
                set_semvalue(shm_access, 0, 1);
                sem_down(shm_access, 0);
                counter++;
            }
        	sem_up(readers_sem, 0);

            shared_memory = (struct entry *)shmat(shmid, 0, 0);          //perform reading
            memcpy(&tmpEntry, shared_memory, sizeof(struct entry));
            gettimeofday(&tv, NULL);
            curTime = tv.tv_sec;
            runningAverage[c] += curTime - tmpEntry.timestamp;
            fprintf(fd, "%d\n", tmpEntry.element);
            shmdt(shared_memory);

            sem_down(readers_sem, 0);
            readers--;
            if(readers == 0){       //last reader
                sem_up(shm_access, 0);
                sleep(2);
            }
            sem_up(readers_sem, 0);
        }
        runningAverage[c] = runningAverage[c] / M;
        fprintf(fd, "Running Average: %.3fsec\n", runningAverage[c]);
        fprintf(fd, "PID: %d\n", childID);
        printf("PID:%d\tRunning Average:%.3fsec\n", childID, runningAverage[c]);
        exit(1);
    }
    else{       //parent
        matrix = malloc(sizeof(int));

        srand((unsigned) time(&t));
        for(i=0; i<M; i++){
            matrix[i] = rand() % 10000;
        }

		for(j=0; j<M; j++){
        	sem_down(shm_access, 0);

            shared_memory = (struct entry *)shmat(shmid, 0, 0);          //perform  writing
            gettimeofday(&tv, NULL);
            timestamp = tv.tv_sec;

            tmpEntry.element = matrix[j];
            tmpEntry.timestamp = tv.tv_sec;
            memcpy(shared_memory, &tmpEntry, sizeof(struct entry));
            shmdt(shared_memory);

            sem_up(shm_access, 0);
            sleep(2);
        }

        while( (child_pid = wait(&status)>0) ){
			if(WIFEXITED(status)){
				printf("Child exited with code %d\n", WEXITSTATUS(status));
			}
			else{
				printf("Child terminated abnormally\n");
			}
		}

        shmctl(shmid, IPC_RMID, 0);
        del_semvalue(shm_access, 0);
        del_semvalue(readers_sem, 0);
        free(matrix);
        exit(1);
    }

}
