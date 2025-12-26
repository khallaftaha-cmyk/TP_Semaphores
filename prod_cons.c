#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define BUFFER_SIZE 5
#define SHM_NAME "/shm_buffer_lr"

#define SEM_MUTEX "/sem_mutex_lr"
#define SEM_FULL "/sem_full_lr"
#define SEM_EMPTY "/sem_empty_lr"

typedef struct {
	int buffer[BUFFER_SIZE];
	int in;
	int out;
}shared_data;

void producteur(int id){
	sem_t *mutex = sem_open(SEM_MUTEX, 0);
	sem_t *full = sem_open(SEM_FULL, 0);
	sem_t *empty = sem_open(SEM_EMPTY, 0);

	int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
	shared_data *data = mmap(NULL, sizeof(shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

	for(int i = 0; i < 5; i++){
		int item = rand() % 100;

		sem_wait(empty);
		sem_wait(mutex);

		data->buffer[data->in] = item;
		printf("Redacteur %d : J'ai écrit %d à l'index %d\n",id , item, data->in);
		data->in = (data->in + 1) % BUFFER_SIZE;

		sem_post(mutex);
		sem_post(full);

		sleep(rand() % 2);
	}

	munmap(data, sizeof(shared_data));
	exit(0);
}

void consommateur(int id){
	sem_t *mutex = sem_open(SEM_MUTEX, 0);
	sem_t *full = sem_open(SEM_FULL, 0);
	sem_t *empty = sem_open(SEM_EMPTY, 0);
	
	int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
	shared_data *data = mmap(NULL, sizeof(shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

	for(int i = 0; i < 5; i++){

		sem_wait(full);
		sem_wait(mutex);

		int item = data->buffer[data->out];
		printf("Consommateur %d : J'ai concommé %d à l'index %d\n",id ,item , data->out);
		data->out = (data->out + 1) % BUFFER_SIZE;

		sem_post(mutex);
		sem_post(empty);

		sleep(rand() % 2);
	}

	munmap(data, sizeof(shared_data));
	exit(0);
}

int main(){
	int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
	ftruncate(shm_fd, sizeof(shared_data));
	shared_data *data = mmap(NULL, sizeof(shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	data->in = 0; data->out = 0; 

	sem_unlink(SEM_MUTEX);
	sem_unlink(SEM_EMPTY);
	sem_unlink(SEM_FULL);

	sem_open(SEM_MUTEX, O_CREAT, 0666, 1);
	sem_open(SEM_EMPTY, O_CREAT, 0666, BUFFER_SIZE);
	sem_open(SEM_MUTEX, O_CREAT, 0666, 0);

	for(int i = 0; i < 2; i++){
		pid_t pid = fork();
		if (pid == 0){
			srand(getpid());
			producteur(i);
		}
	}

	for(int i = 0; i < 2; i++){
		pid_t pid = fork();
		if (pid == 0){
			srand(getpid());
			consommateur(i);
		}
	}


	for(int i = 0; i < 3; i++) wait(NULL);

	munmap(data, sizeof(shared_data));
	shm_unlink(SHM_NAME);
	sem_unlink(SEM_MUTEX);
	sem_unlink(SEM_EMPTY);
	sem_unlink(SEM_MUTEX);

	printf("FIn du programme.\n");
	return 0;
}





