
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>

#define MSG_COUNT  100
#define FILEPATH "mmapped.bin"


pthread_mutex_t count_mutex     = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condition_var   = PTHREAD_COND_INITIALIZER;

typedef struct Message
{
	long msgid;
	char data[4000];
	long timestamp;
} Message;

#define FILESIZE (MSG_COUNT * sizeof(Message))

Message *messageMap = NULL;

void *produceMessages();
void *consumeMessages();
int  count = 0;
int start = 0; 



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
   int fd;
   int result;

    fd = open(FILEPATH, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
    if (fd == -1) {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }

    result = lseek(fd, FILESIZE-1, SEEK_SET);
    if (result == -1) {
        close(fd);
        perror("Error calling lseek() to 'stretch' the file");
        exit(EXIT_FAILURE);
    }

    result = write(fd, "", 1);
    if (result != 1) {
        close(fd);
        perror("Error writing last byte of the file");
        exit(EXIT_FAILURE);
    }

    /* Now the file is ready to be mmapped.
     */
    messageMap = mmap(0, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (messageMap == MAP_FAILED) {
        close(fd);
        perror("Error mmapping the file");
        exit(EXIT_FAILURE);
    }

   //messages = (Message *)malloc(sizeof(Message)*MSG_COUNT);

   pthread_create( &producer, NULL, &produceMessages, NULL);
   pthread_create( &consumer, NULL, &consumeMessages, NULL);

   pthread_join( producer, NULL);
   pthread_join( consumer, NULL);

    if (munmap(messageMap, FILESIZE) == -1) {
        perror("Error un-mmapping the file");
        /* Decide here whether to close(fd) and exit() or not. Depends... */
    }

    /* Un-mmaping doesn't close the file, so we still need to do that.
     */
    close(fd);
   return(EXIT_SUCCESS);
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
      Message msg = messageMap[count];
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
      messageMap[count] = *msg;
      pthread_cond_signal( &condition_var );
      printf("Sent message: %ld\n", msg->msgid);

       pthread_mutex_unlock( &count_mutex );

       sleepMillis(10);
       if(count >= MSG_COUNT) return(NULL);
    }
}

