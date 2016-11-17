/**************************************************
 * Name: ZHANG Zhili
 * UID: 3035141243
 * Platform: Ubuntu 16.04
 * Last Modified: Nov 16
 * Compilation: gcc DPP.c -o DPP -g -Wall -pthread
 **************************************************/

#include "DPP.h"

bool terminate;//flag used to terminate the program
bool* p_states; //storing the status of philosophors
int* f_held; //storing the status of forks

int main(int argc, char *argv[])
{

  terminate=false;

  // dealing with input arguments
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
  
  
  if(error){
    (void)fprintf(stdout, "Error during parsing input arguments\n");
    return EXIT_FAILURE;
  }

  srandom(arguments[1]); //randomize the number generator
  
  int num_agent = arguments[0]+1;

  sem_t lock; //guarantee that at a moment not all philosophers are asking for fork
  sem_t forks[num_agent-1]; //forks
  
  pthread_t agents[num_agent];
  p_states = malloc(sizeof(bool)*(num_agent-1));
  f_held =malloc(sizeof(int)*(num_agent-1));
  
  int i;
  for(i=0; i<num_agent-1; ++i){
    p_states[i]=true;
    f_held[i]=-1;
  }

  //an array of struct philo, which is to be passed to a thread
  // as the argument
  philo **philosophers;
  // struct w_arg, to be passed to the watcher as argument
  w_arg* wArg;

  //initialize all the semaphores
  initialize_semaphores(&lock, forks, num_agent-1);
  //start all the threads
  run_all_threads(agents, &philosophers, &wArg, forks, &lock, num_agent, p_states);

  //the timer used to terminate the program
  //at the certain time.
  time_t start = time(NULL);
  time_t last = (time_t)arguments[2];
  time_t end = start+last;
  while(start<end){
    sleep(1);
    start=time(NULL);
  }
  terminate=true;

  //Join all the threads
  for(i=0; i<num_agent; ++i){
    pthread_join(agents[i], NULL);
  }

  sem_destroy(&lock);
  for(i=0; i<num_agent-1; i++)
    sem_destroy(&forks[i]);
  free(philosophers);
  free(p_states);
  free(f_held);

  return EXIT_SUCCESS;
  
}

void initialize_semaphores(sem_t *lock, sem_t *forks, int num_forks)
{
  int i;
  for(i = 0; i < num_forks; i++) {
    sem_init(&forks[i], 0, 1);
  }
  //initialize the lock to n-1, so that at a time only n-1 people could
  //be asking for fork
  sem_init(lock, 0, num_forks-1);
}

void run_all_threads(pthread_t *threads, philo*** philosophers, 
  w_arg** wArg, sem_t *forks, sem_t *lock, int num_agent, bool* states)
{
  int num_philo = num_agent-1;
  //allocate space for the philo array
  philo** temp = malloc(sizeof(philo*)*num_philo);
  if(!temp){
    (void)fprintf(stderr, "Error in malloc\n");
    return;
  }
  
  int i;
  for(i = 0; i < num_philo; i++) {
    //allocate space for each pointer inside the philo** array
    philo *arg = malloc(sizeof(philo));
    if(!arg){
      (void)fprintf(stderr, "Error in malloc\n");
      continue;
    }
    arg->position = i;
    arg->count = num_philo;
    arg->lock = lock;
    arg->forks = forks;
    arg->state=states;

    temp[i]=arg;
    //create the philosopher thread
    pthread_create(&threads[i], NULL, philosopher, (void *)arg);
  }
  *philosophers = temp;

  //allocate space for watcher's argument
  w_arg* watcherArg = malloc(sizeof(w_arg));

  if(!watcherArg){
    (void)fprintf(stderr, "Error in malloc watcherArg\n");
    return;
  }
  watcherArg->num_philo=num_philo;
  watcherArg->threads=threads;
  watcherArg->philosophers=temp;
  watcherArg->state=states;

  *wArg=watcherArg;
  //create the watcher's thread
  pthread_create(&threads[num_agent-1], NULL, watcher, (void *)watcherArg);
}

void *watcher(void *args){
  w_arg temp = *(w_arg*)args;

  int num_phi = temp.num_philo;
  
  while(!terminate){
    usleep(500000);
    (void)fprintf(stdout, TITLE);//print the title of each round

    //storing the summary for all the philosophers and forks
    int status[2]={0,0};
    int held[2]={0,0};
    int i;

    for(i=0; i<num_phi; i++){
      philo* guy = temp.philosophers[i];
      //build up the string of "held by" information
      char str[8];
      if(f_held[i]==-1){
        held[1]++;
        sprintf(str, "%s", FREE);
      }else{
        held[0]++;
        sprintf(str, "%d", f_held[i]);
      }
      //for each i, print the philosopher status and fork's held status
      if(temp.state[guy->position]){
        status[0]++;
        fprintf(stdout, FORMAT_STR, i, THINK, i, str);
      }else{
        status[1]++;
        fprintf(stdout, FORMAT_STR, i, EAT, i, str);
      }

    }
    //print the summary
    fprintf(stdout, SUMMARY, status[0], 0, status[1], held[0], held[1]);
  }
  free(args);//free teh watcher's arguments after usage
  pthread_exit(NULL);
}

void *philosopher(void *philosopher)
{
  philo self = *(philo *)philosopher;

  while(!terminate){
    long tTime = (random()*pow(10, 6))/RAND_MAX;//randomize the think time
    long eTime = (random()*pow(10, 6))/RAND_MAX;//randomize the eat time
    self.state[self.position]=true;
    think(tTime);

    //P() waiting for the locks
    sem_wait(self.lock);
    sem_wait(&self.forks[self.position]);
    sem_wait(&self.forks[(self.position + 1) % self.count]);
    //modify the fork status
    f_held[self.position]=self.position;
    f_held[(self.position+1)%self.count]=self.position;
    //modify the philosopher status
    self.state[self.position]=false;
    eat(eTime);
    
    //V()
    sem_post(&self.forks[self.position]);
    sem_post(&self.forks[(self.position + 1) % self.count]);
    sem_post(self.lock);
    f_held[self.position]=-1;
    f_held[(self.position+1)%self.count]=-1;
  }

  free(philosopher);
  pthread_exit(NULL);
}

//helper method 
void think(long tTime)
{
  usleep(tTime);
}
//helper method
void eat(long eTime)
{
  usleep(eTime);
}
