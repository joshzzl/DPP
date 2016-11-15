#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>


typedef int bool;
#define true 1;
#define false 0;

#define EXPECTED_ARGS 4
#define BASE 10
#define STR_NOTINT "Argument is not an integer: %s\n"
#define STR_CONVERTING "Error in converting string: %s\n"
#define STR_ARGNUM "Wrong number of input arguments: %d\n"

typedef struct {
  int position;
  int count;
  sem_t *forks;
  sem_t *lock;
} params_t;

void initialize_semaphores(sem_t *lock, sem_t *forks, int num_forks);
void run_all_threads(pthread_t *threads, sem_t *forks, sem_t *lock, int num_philosophers);

void *philosopher(void *params);
void think(int position);
void eat(int position);

int main(int argc, char *argv[])
{
  if(argc != EXPECTED_ARGS){
    (void)fprintf(stderr, STR_ARGNUM, argc);
    return EXIT_FAILURE;
  }
  int arguments[3];
  int numArg=1;
  
  bool error=false;
  while(numArg<argc){
    errno=0;
    char *endptr;
    arguments[numArg-1]=strtol(argv[numArg], &endptr, BASE);
    if(errno!=0){
      error=true;
      (void)fprintf(stderr, STR_CONVERTING, argv[numArg]);
      ++numArg;
      continue;
    }
    if((*endptr)!='\0'){
      error=true;
      (void)fprintf(stderr, STR_NOTINT, argv[numArg]);
      ++numArg;
      continue;
    }
    
    ++numArg;
  }
  
  if(!error){
    (void)fprintf(stdout, "No error in input\nArguments are %d, %d, %d\n", 
      arguments[0], arguments[1], arguments[2]);
  }
  /*
  int num_philosophers = 5;

  sem_t lock;
  sem_t forks[num_philosophers];
  pthread_t philosophers[num_philosophers];

  initialize_semaphores(&lock, forks, num_philosophers);
  run_all_threads(philosophers, forks, &lock, num_philosophers);
  pthread_exit(NULL);
  */
}

void initialize_semaphores(sem_t *lock, sem_t *forks, int num_forks)
{
  int i;
  for(i = 0; i < num_forks; i++) {
    sem_init(&forks[i], 0, 1);
  }

  sem_init(lock, 0, num_forks - 1);
}

void run_all_threads(pthread_t *threads, sem_t *forks, sem_t *lock, int num_philosophers)
{
  int i;
  for(i = 0; i < num_philosophers; i++) {
    params_t *arg = malloc(sizeof(params_t));
    arg->position = i;
    arg->count = num_philosophers;
    arg->lock = lock;
    arg->forks = forks;

    pthread_create(&threads[i], NULL, philosopher, (void *)arg);
  }
}

void *philosopher(void *params)
{
  int i;
  params_t self = *(params_t *)params;

  for(i = 0; i < 3; i++) {
    think(self.position);

    sem_wait(self.lock);
    sem_wait(&self.forks[self.position]);
    sem_wait(&self.forks[(self.position + 1) % self.count]);
    eat(self.position);
    sem_post(&self.forks[self.position]);
    sem_post(&self.forks[(self.position + 1) % self.count]);
    sem_post(self.lock);
  }

  think(self.position);
  pthread_exit(NULL);
}

void think(int position)
{
  printf("Philosopher %d thinking...\n", position);
}

void eat(int position)
{
  printf("Philosopher %d eating...\n", position);
}
