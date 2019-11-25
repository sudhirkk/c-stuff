#include <stdio.h> 
#include <stdlib.h> 
#include <limits.h> 
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define BILLION 1000000000L

pthread_mutex_t mutex     = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condition = PTHREAD_COND_INITIALIZER;

typedef struct Message
{
	long msgid;
	char data[4000];
	long timestamp;
} Message;

struct Queue 
{ 
	int front, rear, size; 
	unsigned capacity; 
	Message* array; 
}; 

Message *empty = NULL;

void createMmapFiles()
{
	int arraySize = sizeof(Message) * 10000;
	int queueSize = 8;
	FILE *fp = fopen("messages.data", "w");
	fseek(fp, arraySize, SEEK_SET);
	fputc('\0', fp);
	fclose(fp);

	fp = fopen("queue.data", "w");
	fseek(fp, queueSize, SEEK_SET);
	fputc('\0', fp);
	fclose(fp);

}

/*
 * Get an artitrary time in microseconds accuracy
 * Use it to find time differences in perf measurements.
 */
long getMicroTime()
{
	struct timespec tm;
	clock_gettime(CLOCK_MONOTONIC, &tm);
	return tm.tv_sec*1000000 + tm.tv_nsec/1000;
}

/*
 * Pass the capacity and create the queue structure which can hold Message structures.
 */
struct Queue* createQueue(unsigned capacity) 
{ 
	struct Queue* queue = (struct Queue*) malloc(sizeof(struct Queue)); 
	queue->capacity = capacity; 
	queue->front = queue->size = 0; 
	queue->rear = capacity - 1; // This is important, see the enqueue 
	queue->array = (Message*) malloc(queue->capacity * sizeof(Message)); 
	//Message *msgs  = (Message*) malloc(queue->capacity * sizeof(Message)); 
	printf("Q0 %d\n", queue->rear);

	int size = queue->capacity * sizeof(Message);
	int fd = open("messages.data", O_RDWR | O_CREAT | O_TRUNC, 0600);

  	Message *addr = (Message *)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  	printf("Mapped at %p\n", addr);

  	if (addr == (void*) -1 ) exit(-1);

	addr = queue->array;
	//queue->array = addr;
	return queue; 
} 

// Queue is full when size becomes equal to the capacity 
int isFull(struct Queue* queue) 
{ return (queue->size == queue->capacity); } 

// Queue is empty when size is 0 
int isEmpty(struct Queue* queue) 
{ return (queue->size == 0); } 

/* 
 * Function to add an item to the queue. 
 * It changes rear and size 
 */
void enqueue(struct Queue* queue, Message msg) 
{ 
	if (isFull(queue)) 
		return; 
	printf("Q1 %d %d\n", queue->rear, queue->capacity);
	queue->rear = (queue->rear + 1)%queue->capacity; 
	printf("Q2 %d\n", queue->rear);
	queue->array[queue->rear] = msg; 
	printf("Q3\n");
	queue->size = queue->size + 1; 
	printf("%ld enqueued to queue\n", msg.msgid); 
} 

/* 
 * Function to remove an item from queue. 
 * It changes front and size 
 */
Message dequeue(struct Queue* queue) 
{ 
	if (isEmpty(queue)) 
	{
		if (empty == NULL)
			empty = (Message *)malloc(sizeof(Message));
		empty->msgid = -1;
		return *empty; 
	}

	Message msg = queue->array[queue->front]; 
	queue->front = (queue->front + 1)%queue->capacity; 
	queue->size = queue->size - 1; 
	return msg; 
} 

// Function to get front of queue 
Message front(struct Queue* queue) 
{ 
	if (isEmpty(queue)) 
	{
		if (empty == NULL)
			empty = (Message *)malloc(sizeof(Message));
		empty->msgid = -1;
		return *empty; 
	}
	return queue->array[queue->front]; 
} 

// Function to get rear of queue 
Message rear(struct Queue* queue) 
{ 
	if (isEmpty(queue)) 
	{
		if (empty == NULL)
			empty = (Message *)malloc(sizeof(Message));
		empty->msgid = -1;
		return *empty; 
	}
	return queue->array[queue->rear]; 
} 

int main() 
{ 

	createMmapFiles();
	struct Queue* queue = createQueue(100); 
	for (int i=0; i<10; i++)
	{
		printf("1\n");
		Message *message = (Message *) malloc(sizeof(Message));
		printf("2\n");
		message->msgid = i+1;
		printf("3\n");
		enqueue(queue, *message);
		printf("4\n");
	}

	printf("Front item is %ld\n", front(queue).msgid); 
	printf("Rear item is %ld\n", rear(queue).msgid); 

	for (int i=0; i<10; i++)
	{
		printf("%ld dequeued from queue\n", dequeue(queue).msgid); 
	}



	//long ts = getMicroTime();
	//sleep(1);
	//long ts1 = getMicroTime();
	//printf("Time: %ld\n", ts1-ts);
	printf("Size of queue: %ld, size of Message: %ld\n", sizeof(queue), sizeof(struct Message));

	return 0; 
} 

