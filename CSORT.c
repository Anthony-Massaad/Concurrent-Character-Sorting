#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <ctype.h>

#include "CSORT.h"

static int set_sem1value(void);
static void del_sem1value(void);
static int semaphore1_p(void);
static int semaphore1_v(void);
static int set_sem2value(void);
static void del_sem2value(void);
static int semaphore2_p(void);
static int semaphore2_v(void);

static int sem1_id;
static int sem2_id;


int main(int argc, char **argv)
{

	char temp;
	srand((unsigned int)getpid());

	sem1_id = semget((key_t)123, 1, 0666 | IPC_CREAT);
	sem2_id = semget((key_t)124, 1, 0666 | IPC_CREAT);
	
	void *shared_memory = (void *)0;
	struct shared_use_st *shared_stuff;

	int shmid;
	shmid = shmget((key_t)1234, sizeof(int), 0666 | IPC_CREAT);

	if (shmid == -1) {
		fprintf(stderr, "shmget failed\n");
		exit(EXIT_FAILURE);
	}

	shared_memory = shmat(shmid, (void *)0, 0);
	if (shared_memory == (void *)-1) {
		fprintf(stderr, "shmmat failed\n");
		exit(EXIT_FAILURE);
	}

	if (!set_sem1value()) {
		fprintf(stderr, "Failed to initialize semaphore\n");
		exit(EXIT_FAILURE);
	}
	
	if (!set_sem2value()) {
		fprintf(stderr, "Failed to initialize semaphore\n");
		exit(EXIT_FAILURE);
	}

	//Prompt user for 7 letters (TO DO)
	int debug = 0; 
	char answer;

	while (1){
		printf("Would you like debug mode? (y/n): ");
		scanf("%c", &answer);
		answer = tolower(answer);
		if (answer == 'y' || answer == 'n'){
			if (answer == 'y'){
				debug = 1;
			}
			break;
		}
		else{
			printf("Invalid input\n");
		}
	}


	shared_stuff = (struct shared_use_st *)shared_memory;
	printf("Enter 7 chars (no spaces and unique characters please): ");
	scanf("%s", &shared_stuff->AR);
	int size = sizeof(shared_stuff->AR) / sizeof(shared_stuff->AR[0]);
	for (int i = 0; i < size; i++){
		shared_stuff->AR[i] = tolower(shared_stuff->AR[i]);
	}

	
	shared_stuff->ordered[0] = 0;
	shared_stuff->ordered[1] = 0;
	shared_stuff->ordered[2] = 0;

	shared_stuff->complete = 0;

	pid_t pid;
	pid = fork();

	switch(pid){
	case -1:
		perror("fork failed");
		exit(1);

	case 0:
		if (shmid == -1) {
			fprintf(stderr, "shmget failed\n");
			exit(EXIT_FAILURE);
		}

		shared_memory = shmat(shmid, (void *)0, 0);
		if (shared_memory == (void *)-1) {
			fprintf(stderr, "shmmat failed\n");
			exit(EXIT_FAILURE);
		}

		shared_stuff = (struct shared_use_st *)shared_memory;
		
		while(!shared_stuff->ordered[0] || !shared_stuff->ordered[1] || !shared_stuff->ordered[2]){
			int swapped = 0;
			
			if(shared_stuff->AR[0] > shared_stuff->AR[1]){
				temp = shared_stuff->AR[0];
				shared_stuff->AR[0] = shared_stuff->AR[1];
				shared_stuff->AR[1] = temp;
				swapped = 1;
				if (debug){
					printf("Prcoess 1 swaps %c and %c\n", shared_stuff->AR[0], shared_stuff->AR[1]);
				}
			}

			if(shared_stuff->AR[1] > shared_stuff->AR[2]){
				if (!semaphore1_p()) exit(EXIT_FAILURE);
				temp = shared_stuff->AR[1];
				shared_stuff->AR[1] = shared_stuff->AR[2];
				shared_stuff->AR[2] = temp;
				swapped = 1;
				if (debug){
					printf("Prcoess 1 swaps %c and %c\n", shared_stuff->AR[1], shared_stuff->AR[2]);
				}
				shared_stuff->ordered[1] = 0;
				if (!semaphore1_v()) exit(EXIT_FAILURE);
			}


			if (debug){
				if (!swapped){
					printf("Process 1 did not swap\n");
				}
			}

			
			if (!semaphore1_p()) exit(EXIT_FAILURE);
			if(shared_stuff->AR[0] < shared_stuff->AR[1] && shared_stuff->AR[0] < shared_stuff->AR[2] && shared_stuff->AR[1] < shared_stuff->AR[2]){
				shared_stuff->ordered[0] = 1;
			}
			if (!semaphore1_v()) exit(EXIT_FAILURE);

		}
		printf("final results for process 1 [%c %c %c %c %c %c %c]\n", shared_stuff->AR[0], shared_stuff->AR[1], shared_stuff->AR[2], shared_stuff->AR[3], shared_stuff->AR[4], shared_stuff->AR[5], shared_stuff->AR[6]);

		shared_stuff->complete++;

		if(shared_stuff->complete == 3){
			del_sem1value();
			del_sem2value();
		}

		break;

	//Parent after P1
	default:
		
		pid = fork();

		switch(pid){
			case -1:
				perror("fork failed");
				exit(1);

			case 0:
				if (shmid == -1) {
					fprintf(stderr, "shmget failed\n");
					exit(EXIT_FAILURE);
				}

				shared_memory = shmat(shmid, (void *)0, 0);
				if (shared_memory == (void *)-1) {
					fprintf(stderr, "shmmat failed\n");
					exit(EXIT_FAILURE);
				}

				shared_stuff = (struct shared_use_st *)shared_memory;	

				while(!shared_stuff->ordered[0] || !shared_stuff->ordered[1] || !shared_stuff->ordered[2]){
					int swapped = 0; 
					if(shared_stuff->AR[2] > shared_stuff->AR[3]){
						if (!semaphore1_p()) exit(EXIT_FAILURE);
						temp = shared_stuff->AR[2];
						shared_stuff->AR[2] = shared_stuff->AR[3];
						shared_stuff->AR[3] = temp;
						swapped = 1;
						if (debug){
							printf("Prcoess 2 swaps %c and %c\n", shared_stuff->AR[2], shared_stuff->AR[3]);
						}
						shared_stuff->ordered[0] = 0;
						if (!semaphore1_v()) exit(EXIT_FAILURE);
					}

					if(shared_stuff->AR[3] > shared_stuff->AR[4]){
						if (!semaphore2_p()) exit(EXIT_FAILURE);
						temp = shared_stuff->AR[3];
						shared_stuff->AR[3] = shared_stuff->AR[4];
						shared_stuff->AR[4] = temp;
						swapped = 1;
						if (debug){
							printf("Prcoess 2 swaps %c and %c\n", shared_stuff->AR[3], shared_stuff->AR[4]);
						}
						shared_stuff->ordered[2] = 0;
						if (!semaphore2_v()) exit(EXIT_FAILURE);
					}

					if (debug){
						if (!swapped){
							printf("process 2 did not swap\n");
						}
					}
					
					if (!semaphore1_p()) exit(EXIT_FAILURE);
					if (!semaphore2_p()) exit(EXIT_FAILURE);
					if(shared_stuff->AR[2] < shared_stuff->AR[3] && shared_stuff->AR[2] < shared_stuff->AR[4] && shared_stuff->AR[3] < shared_stuff->AR[4]){
						shared_stuff->ordered[1] = 1;
					}
					if (!semaphore1_v()) exit(EXIT_FAILURE);
					if (!semaphore2_v()) exit(EXIT_FAILURE);

				}	
				printf("final results for process 2 [%c %c %c %c %c %c %c]\n", shared_stuff->AR[0], shared_stuff->AR[1], shared_stuff->AR[2], shared_stuff->AR[3], shared_stuff->AR[4], shared_stuff->AR[5], shared_stuff->AR[6]);

				shared_stuff->complete++;

				if(shared_stuff->complete == 3){
					del_sem1value();
					del_sem2value();
				}

				break;

			//P3
			default:
				pid = fork();

				switch(pid){
					case -1:
						perror("fork failed");
						exit(1);
					case 0:
						if (shmid == -1) {
							fprintf(stderr, "shmget failed\n");
							exit(EXIT_FAILURE);
						}

						shared_memory = shmat(shmid, (void *)0, 0);
						if (shared_memory == (void *)-1) {
							fprintf(stderr, "shmmat failed\n");
							exit(EXIT_FAILURE);
						}

						shared_stuff = (struct shared_use_st *)shared_memory;

						while(!shared_stuff->ordered[0] || !shared_stuff->ordered[1] || !shared_stuff->ordered[2]){
							int swapped = 0;

							if(shared_stuff->AR[4] > shared_stuff->AR[5]){
								if (!semaphore2_p()) exit(EXIT_FAILURE);
								temp = shared_stuff->AR[4];
								shared_stuff->AR[4] = shared_stuff->AR[5];
								shared_stuff->AR[5] = temp;
								swapped = 1; 
								if (debug){
									printf("Prcoess 3 swaps %c and %c\n", shared_stuff->AR[4], shared_stuff->AR[5]);
								}
								shared_stuff->ordered[1] = 0;
								if (!semaphore2_v()) exit(EXIT_FAILURE);
							}

							if(shared_stuff->AR[5] > shared_stuff->AR[6]){
								temp = shared_stuff->AR[5];
								shared_stuff->AR[5] = shared_stuff->AR[6];
								shared_stuff->AR[6] = temp;
								swapped = 1;
								if (debug){
									printf("Prcoess 3 swaps %c and %c\n", shared_stuff->AR[5], shared_stuff->AR[6]);
								}
							}

							if (debug){
								if (!swapped){
									printf("Process 3 did not swap\n");
								}
							}
							
							if (!semaphore2_p()) exit(EXIT_FAILURE);
							if(shared_stuff->AR[4] < shared_stuff->AR[5] && shared_stuff->AR[4] < shared_stuff->AR[6] && shared_stuff->AR[5] < shared_stuff->AR[6]){
								shared_stuff->ordered[2] = 1;
							}
							if (!semaphore2_v()) exit(EXIT_FAILURE);
					
						}
						printf("final results for process 3 [%c %c %c %c %c %c %c]\n", shared_stuff->AR[0], shared_stuff->AR[1], shared_stuff->AR[2], shared_stuff->AR[3], shared_stuff->AR[4], shared_stuff->AR[5], shared_stuff->AR[6]);

						shared_stuff->complete++;

						if(shared_stuff->complete == 3){
							del_sem1value();
							del_sem2value();
						}
						break;

					default:
						break;
				}
				break;
			}
			break;
		}
	
	if (shmdt(shared_memory) == -1){
		fprintf(stderr, "shmdt failed\n");
		exit(EXIT_FAILURE);
	}

	if (shmctl(shmid, IPC_RMID, 0) == -1){
		fprintf(stderr, "shmctl failed");
		exit(EXIT_FAILURE);
	}

	exit(0);

}

static int set_sem1value(void)
{
	union semun sem1_union;

	sem1_union.val = 1;
	if (semctl(sem1_id, 0, SETVAL, sem1_union) == -1) return(0);
	return(1);
}

static void del_sem1value(void)
{
	union semun sem1_union;

	if (semctl(sem1_id, 0, IPC_RMID, sem1_union) == -1)
		fprintf(stderr, "Failed to deleted semaphore1\n");
}

static int semaphore1_p(void)
{
	struct sembuf sem_b1;

	sem_b1.sem_num = 0;
	sem_b1.sem_op = -1; /* P() */
	sem_b1.sem_flg = SEM_UNDO;
	if (semop(sem1_id, &sem_b1, 1) == -1) {
		fprintf(stderr, "semaphore1_p failed\n");
		return(0);
	}
	return(1);
}

static int semaphore1_v(void)
{
	struct sembuf sem_b1;

	sem_b1.sem_num = 0;
	sem_b1.sem_op = 1; /* V() */
	sem_b1.sem_flg = SEM_UNDO;
	if (semop(sem1_id, &sem_b1, 1) == -1) {
		fprintf(stderr, "semaphore1_v failed\n");
		return(0);
	}
	return(1);
}

static int set_sem2value(void)
{
	union semun sem2_union;

	sem2_union.val = 1;
	if (semctl(sem2_id, 0, SETVAL, sem2_union) == -1) return(0);
	return(1);
}

static void del_sem2value(void)
{
	union semun sem2_union;

	if (semctl(sem2_id, 0, IPC_RMID, sem2_union) == -1)
		fprintf(stderr, "Failed to deleted semaphore2\n");
}

static int semaphore2_p(void)
{
	struct sembuf sem2_b;

	sem2_b.sem_num = 0;
	sem2_b.sem_op = -1; /* P() */
	sem2_b.sem_flg = SEM_UNDO;
	if (semop(sem2_id, &sem2_b, 1) == -1) {
		fprintf(stderr, "semaphore2_p failed\n");
		return(0);
	}
	return(1);
}

static int semaphore2_v(void)
{
	struct sembuf sem2_b;

	sem2_b.sem_num = 0;
	sem2_b.sem_op = 1; /* V() */
	sem2_b.sem_flg = SEM_UNDO;
	if (semop(sem2_id, &sem2_b, 1) == -1) {
		fprintf(stderr, "semaphore2_v failed\n");
		return(0);
	}
	return(1);
}
