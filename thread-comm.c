#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t count_mutex     = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condition_var   = PTHREAD_COND_INITIALIZER;

typedef struct Message
{
	long msgid;
	char data[4000];
	long timestamp;
} Message;

Message *messages = NULL;

void *produceMessages();
void *consumeMessages();
int  count = 0;
int start = 0; 

#define MSG_COUNT  10000


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

int main()
{
   pthread_t producer, consumer;
   messages = (Message *)malloc(sizeof(Message)*MSG_COUNT);

   pthread_create( &producer, NULL, &produceMessages, NULL);
   pthread_create( &consumer, NULL, &consumeMessages, NULL);

   pthread_join( producer, NULL);
   pthread_join( consumer, NULL);

   exit(EXIT_SUCCESS);
}

// Write numbers 1-3 and 8-10 as permitted by functionCount2()

void sleepMillis(int milisec)
{
    struct timespec res;
    res.tv_sec = milisec/1000;
    res.tv_nsec = (milisec*1000000) % 1000000000;
    clock_nanosleep(CLOCK_MONOTONIC, 0, &res, NULL);
}

void *consumeMessages()
{
   for(;;)
   {
      // Lock mutex and then wait for signal to relase mutex
      pthread_mutex_lock( &count_mutex );

      // Wait while functionCount2() operates on count
      // mutex unlocked if condition varialbe in functionCount2() signaled.
      pthread_cond_wait( &condition_var, &count_mutex );
      Message msg = messages[count];
      long ts = getMicroTime();

      printf("Received message: %ld (%ld)\n", msg.msgid, ts - msg.timestamp);

      pthread_mutex_unlock( &count_mutex );

      if(count >= MSG_COUNT) return(NULL);
    }
}

void *produceMessages()
{
    for(;;)
    {
       pthread_mutex_lock( &count_mutex );

      Message *msg = (Message *)malloc(sizeof(Message));
      count++;
      msg->msgid = count;
      msg->timestamp = getMicroTime();
      messages[count] = *msg;
      pthread_cond_signal( &condition_var );
      printf("Sent message: %ld\n", msg->msgid);

       pthread_mutex_unlock( &count_mutex );

       sleepMillis(10);
       if(count >= MSG_COUNT) return(NULL);
    }
}

