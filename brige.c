#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>


typedef union _semun {
		int val;
		struct semid_ds *buf;
		unsigned short *array;
}semun;

static int shmid, semid;

void initshm(int **p){
		shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
		*p = (int *)shmat(shmid, NULL, 0);
		**p = 0;
}

int initsem(){
		semid = semget(IPC_PRIVATE, 2, IPC_CREAT | 0666);
		semun arg;
		arg.val = 1;
		semctl(semid, 0, SETVAL, arg);
		semctl(semid, 1, SETVAL, arg);
		return semid;
}

void remobj(int *Ncount, int *Scount){
		if (shmdt((void *)Ncount) == -1)
				printf("shmdt error\n");
		if (shmctl(shmid, IPC_RMID, NULL) == -1)
				printf("shmctl error/n");
		if (semctl(semid, 0, IPC_RMID, 0) == -1)
				printf("semctl error/n");
}

void P(int semid, int sem_num){
		struct sembuf sb;
		sb.sem_num = sem_num;
		sb.sem_op = -1;
		sb.sem_flg = 0;
		semop(semid, &sb, 1);
}

int V(int semid, int sem_num){
		struct sembuf sb;
		sb.sem_num = sem_num;
		sb.sem_op = 1;
		sb.sem_flg = 0;
		semop(semid, &sb, 1);
}

void NtoS(int *Ncount){
		P(semid, 0);
		P(semid,  1);
		(*Ncount)++;
		printf("A vehicle from the north is crossing the bridge./n");
		sleep(1);
		printf("A vehicle from the north has crossed the bridge./n");
		V(semid, 1);
		V(semid, 0);
}

void StoN(int *Scount){
		P(semid, 1);
		P(semid, 0);
		(*Scount)++;
		printf("A vehicle from the south is crossing the bridge./n");
		sleep(1);
		printf("A veihcle from the south has crossed the brideg./n");
		V(semid, 0);
		V(semid, 1);
}

int main(){
		key_t semkey, shmkey;
		int *Ncount;
		int *Scount;

		Ncount = (int *)shmat(shmid, NULL, 0);
		Scount = (int *)shmat(shmid, NULL, 0);

		initshm(&Ncount);
		initshm(&Scount);

		semid = initsem();

		pid_t pid = fork();

		if (pid == 0){
				while (1){
						NtoS(Ncount);
				}

		} else if (pid > 0) {
				while (1) {
						StoN(Scount);
				}

		} else {
				printf("Fork error/n");
				return 1;
		}

		remobj(Ncount, Scount);

		return 0;
}
