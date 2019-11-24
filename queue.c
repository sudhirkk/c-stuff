#include <stdio.h> 
#include <stdlib.h> 
#include <limits.h> 
#include <time.h>

#define BILLION 1000000000L

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
	struct Message* array; 
}; 

Message *empty = NULL;

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
	queue->rear = (queue->rear + 1)%queue->capacity; 
	queue->array[queue->rear] = msg; 
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

	struct Queue* queue = createQueue(1000); 
	for (int i=0; i<10; i++)
	{
		Message *message = (Message *) malloc(sizeof(Message));
		message->msgid = i+1;
		enqueue(queue, *message);
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
	//printf("Size of queue: %ld, size of Message: %ld\n", sizeof(queue), sizeof(struct Message));

	return 0; 
} 

