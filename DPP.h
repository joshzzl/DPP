/**************************************************
 * Name: ZHANG Zhili
 * UID: 3035141243
 * Platform: Ubuntu 16.04
 * Last Modified: Nov 16
 * Compilation: gcc DPP.c -o DPP -g -Wall -pthread
 * Library for DPP 
 **************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>


typedef int bool;
#define true 1
#define false 0

#define EXPECTED_ARGS 4
#define BASE 10
#define STR_NOTINT "Argument is not an integer: %s\n"
#define STR_CONVERTING "Error in converting string: %s\n"
#define STR_ARGNUM "Wrong number of input arguments: %d\n"
#define TITLE "\nPhilo    State           Fork     Held by      \n"
#define FORMAT_STR "[%-3d]:   %-16s[%-3d]:   %-8s    \n"
#define EAT "Eating"
#define THINK "Thinking"
#define FREE "Free"
#define SUMMARY "Th=%-3d Wa=%-3d Ea=%-3d              Use=%-3d Avail=%-3d\n"

typedef struct {
  int position;
  int count;
  sem_t *forks;
  sem_t *lock;
  bool* state;
} philo;

typedef struct{
  int num_philo;
  pthread_t *threads;
  philo** philosophers;
  bool* state;
} w_arg;

void initialize_semaphores(sem_t *lock, sem_t *forks, int num_forks);
void run_all_threads(pthread_t *threads, philo*** philosophers, 
	w_arg** wArg, sem_t *forks, sem_t *lock, int num_agent, bool* states);

void *philosopher(void *philosopher);
void *watcher(void *args);
void think(long tTime);
void eat(long eTime);
void freePhilo(philo** philos, int num);