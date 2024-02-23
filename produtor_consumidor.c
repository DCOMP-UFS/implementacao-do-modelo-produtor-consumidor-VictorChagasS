#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> 
#include <unistd.h>
#include <semaphore.h>
#include <time.h>

#define THREAD_NUM 3    // Tamanho do pool de threads
#define BUFFER_SIZE 90 // Número máximo de tarefas enfileiradas



typedef struct Clock { 
    int p[3];
    int producer_id;
} Clock;

void randomClock(Clock *clock) {
    for (int i = 0; i < 3; i++) {
        clock->p[i] = rand() % 10; // Gera valores aleatórios entre 0 e 9
    }
}

Clock taskQueue[BUFFER_SIZE];
int taskCount = 0;

pthread_mutex_t mutex;
pthread_cond_t condFull;
pthread_cond_t condEmpty;

//PARA MUDANÇA DE CENARIOS ENTRE PRODUTOR E CONSUMIDOR, MODIFIQUE ESTA VARIAVEL
//
//
int sleepInProducer = -1; // Variável global para indicar se deve haver sleep no produtor (1) ou no consumidor (0) e -1 para nenhum deles
//
//
typedef struct {
    int id;
} ThreadArgs;

void executeTask(Clock *clock, int id) {
    printf("ID_produtor: %d, Clock: (%d, %d, %d)\n",clock->producer_id, clock->p[0], clock->p[1], clock->p[2]);
}

Clock getTask() {
    pthread_mutex_lock(&mutex);

    while (taskCount == 0) {
        pthread_cond_wait(&condEmpty, &mutex);
        printf("Fila esta vazia\n");
    }

    Clock task = taskQueue[0];
   
    int i;
    for (i = 0; i < taskCount - 1; i++) {
        taskQueue[i] = taskQueue[i + 1];
    }
    taskCount--;

    pthread_mutex_unlock(&mutex);
    pthread_cond_signal(&condFull);
   
    return task;
}

void submitTask(Clock task) {
    pthread_mutex_lock(&mutex);

    while (taskCount == BUFFER_SIZE) {
        pthread_cond_wait(&condFull, &mutex);
          printf("Fila esta cheia\n");
    }

    taskQueue[taskCount] = task;
    taskCount++;

    pthread_mutex_unlock(&mutex);
    pthread_cond_signal(&condEmpty);
}

void *producer(void *arg) {
    ThreadArgs *thread_args = (ThreadArgs *)arg;
    int id = thread_args->id;
  
    while (1) {
        Clock clock;
        randomClock(&clock);
        clock.producer_id = id;

        printf("(Producer %d) SUBMETEU  Clock: (%d, %d, %d)\n", id, clock.p[0], clock.p[1], clock.p[2]);
        submitTask(clock);
        
        if (sleepInProducer == 1)
            sleep(2); // Se sleepInProducer for 1, então dorme por 2 segundos

        
    }
    pthread_exit(NULL);
}

void *consumer(void *arg) {
    ThreadArgs *thread_args = (ThreadArgs *)arg;
    int id = thread_args->id;
   
    while (1) {
        Clock task = getTask();
       
        printf("(Consumer %d) CONSUMIU ", id);
        executeTask(&task, id);

        if (sleepInProducer == 0)
            sleep(2); // Se sleepInProducer for 0, então dorme por 2 segundos
       
        
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    srand(time(NULL)); // Inicializa a semente do gerador de números aleatórios
  
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&condEmpty, NULL);
    pthread_cond_init(&condFull, NULL);

    pthread_t producer_threads[THREAD_NUM];
    pthread_t consumer_threads[THREAD_NUM];
    ThreadArgs producer_args[THREAD_NUM];
    ThreadArgs consumer_args[THREAD_NUM];
    
    for (int i = 0; i < THREAD_NUM; i++) {
        producer_args[i].id = i + 1;
        consumer_args[i].id = i + 1;

        pthread_create(&consumer_threads[i], NULL, consumer, (void *)&consumer_args[i]);
        pthread_create(&producer_threads[i], NULL, producer, (void *)&producer_args[i]);
    }

    for (int i = 0; i < THREAD_NUM; i++) {
        pthread_join(consumer_threads[i], NULL);
        pthread_join(producer_threads[i], NULL);
    }

    return 0;
}
