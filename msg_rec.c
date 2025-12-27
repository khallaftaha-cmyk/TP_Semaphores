#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define MAX_MESSAGES 5
#define MSG_SIZE 128

typedef struct{
	char text[MSG_SIZE];
	int priority;
}message_t;

typedef struct{
	int value;
	pthread_mutex_t lock;
	pthread_cond_t wait_queue;
}semaphore_t;

void sem_init_custom(semaphore_t *s, int init_val){
	s->value = init_val;
	pthread_mutex_init(&s->lock,NULL);
	pthread_cond_init(&s->wait_queue, NULL);
}

void down(semaphore_t *s){
	pthread_mutex_lock(&s->lock);
	s->value--;
	if(s->value < 0){
		pthread_cond_wait(&s->wait_queue,&s->lock);
	}
	pthread_mutex_unlock(&s->lock);
}

void up(semaphore_t *s){
	pthread_mutex_lock(&s->lock);
	s->value++;
	if(s->value <= 0){
		pthread_cond_signal(&s->wait_queue);
	}
	pthread_mutex_unlock(&s->lock);
}

typedef struct{
	message_t queue[MAX_MESSAGES];
	int head;
	int tail;
	semaphore_t empty;
	semaphore_t full;
	semaphore_t mutex;
}mailbox_t;

void send_message(mailbox_t *box, const char* content){
	down(&box->empty);
	down(&box->mutex);

	strncpy(box->queue[box->tail].text, content, MSG_SIZE);
	printf("[Expéditeur]Message envoyé:%s\n",content);
	box->tail = (box->tail + 1) % MAX_MESSAGES;

	up(&box->mutex);
	up(&box->full);
}

void sem_destroy_custom(semaphore_t *s){
	pthread_mutex_destroy(&s->lock);
	pthread_cond_destroy(&s->wait_queue);
}

void recieve_message(mailbox_t *box, char *buffer){
	down(&box->full);
	down(&box->mutex);

	strncpy(buffer, box->queue[box->head].text, MSG_SIZE);
	printf("[Destinataire]Message reçue:%s\n",buffer);
	box->head = (box->head + 1) % MAX_MESSAGES;

	up(&box->mutex);
	up(&box->empty);
}

mailbox_t my_mailbox;

void* sender_thread(void *arg){
	char* messages[] = {"bonjour", "comment ça va", "alertes sytéme", "fin de transmission"};
	for(int i = 0; i < 4; i++){
		send_message(&my_mailbox, messages[i]);
		sleep(1);
	}
	return NULL;
}

void* reciever_thread(void* arg){
	char buffer[MSG_SIZE];
	for(int i = 0; i < 4; i++){
		recieve_message(&my_mailbox, buffer);
		sleep(2);
	}
	return NULL;
}

int main(){
	pthread_t t1,t2;

	my_mailbox.head = 0;
	my_mailbox.tail = 0;

	sem_init_custom(&my_mailbox.full, 0);
	sem_init_custom(&my_mailbox.empty, MAX_MESSAGES);
	sem_init_custom(&my_mailbox.mutex, 1);
	
	printf("deamarrage de systéme e méssagerie...\n");
	
	pthread_create(&t1, NULL, sender_thread, NULL);
	pthread_create(&t2, NULL, reciever_thread, NULL);

	pthread_join(1,NULL);
	pthread_join(2,NULL);

	sem_destroy(&my_mailbox.full);
	sem_destroy(&my_mailbox.empty);
	sem_destroy(&my_mailbox.mutex);

	printf("\nsytéme arrete\n");
	return 0;
}







