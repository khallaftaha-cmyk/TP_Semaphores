#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#define MSG_SIZE 100

char mail_box[MSG_SIZE];

sem_t empty;
sem_t full;
pthread_mutex_t mutex;

void* sender(void* arg){
	char* messages[] = {
		"bonjour",
		"message systéme",
		"alerte",
		"fin"
	};

	for(int i = 0;i < 4;i++){
		sem_wait(&empty);
		pthread_mutex_lock(&mutex);

		strcpy(mail_box, messages[i]);
		printf("[Emetteur] envoie : %s\n", mail_box);
		fflush(stdout);

		pthread_mutex_unlock(&mutex);
		sem_post(&full);
	}

	return NULL;
}

void* reciever(void* arg){
	char buffer[MSG_SIZE];

	for(int i = 0; i < 4; i++){
		sem_wait(&full);
		pthread_mutex_lock(&mutex);

		strcpy(buffer, mail_box);
		printf("[Recepteur] reçoit : %s\n", buffer);
		fflush(stdout);

		pthread_mutex_unlock(&mutex);
		sem_post(&empty);
	}

	return NULL;
}

int main(){
	pthread_t t1, t2;

	sem_init(&empty, 0, 1);
	sem_init(&full, 0, 0);
	pthread_mutex_init(&mutex, NULL);

	printf("Demarrage du systéme de messagerie\n");

	pthread_create(&t1, NULL, sender, NULL);
	pthread_create(&t2, NULL, reciever, NULL);

	sem_destroy(&empty);
	sem_destroy(&full);
	pthread_mutex_destroy(&mutex);

	printf("Systéme arreté\n");

	return 0;
}







