#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define N 5               // Número de filósofos / tenedores
#define THINKING 0        // Estado: Pensando[cite: 22]
#define HUNGRY 1          // Estado: Hambriento (quiere comer)[cite: 22, 23]
#define EATING 2          // Estado: Comiendo[cite: 22, 23]

#define ITERACIONES 3     // Cantidad de ciclos de comer/pensar para la simulación

// Variables globales de sincronización y estado (según la diapositiva)
int state[N];             
sem_t mutex;              // Exclusión mutua para modificar estados[cite: 22, 23]
sem_t s[N];               // Semáforos individuales para cada filósofo[cite: 22, 23]

// Prototipos de funciones clave
void test(int i);
void take_forks(int i);
void put_forks(int i);
void* philosopher(void* num);

// Función test: Verifica si el filósofo i puede comer[cite: 23]
void test(int i) {
    if (state[i] == HUNGRY && 
        state[(i + N - 1) % N] != EATING && 
        state[(i + 1) % N] != EATING) {
        
        state[i] = EATING;
        sem_post(&s[i]); // V(s[i]): Despierta al filósofo si estaba esperando
    }
}

// Procedimiento take_forks: Tomar tenedores[cite: 23]
void take_forks(int i) {
    sem_wait(&mutex); // P(mutex): Sección crítica protegida[cite: 23]
    state[i] = HUNGRY;
    printf("Filósofo %d tiene HAMBRE y solicita tenedores.\n", i);
    fflush(stdout);
    
    test(i);          // Intenta adquirir los tenedores[cite: 23]
    sem_post(&mutex); // V(mutex)[cite: 23]
    
    sem_wait(&s[i]);  // P(s[i]): Si no pudo comer, se bloquea aquí[cite: 23]
}

// Procedimiento put_forks: Devolver tenedores[cite: 23]
void put_forks(int i) {
    sem_wait(&mutex); // P(mutex): Sección crítica protegida[cite: 23]
    state[i] = THINKING;
    printf("Filósofo %d terminó de comer y vuelve a PENSAR.\n", i);
    fflush(stdout);
    
    test((i + N - 1) % N); // Comprueba si el vecino izquierdo puede comer ahora[cite: 23]
    test((i + 1) % N);     // Comprueba si el vecino derecho puede comer ahora[cite: 23]
    
    sem_post(&mutex); // V(mutex)[cite: 23]
}

// Rutina que ejecuta cada hilo (filósofo)[cite: 22]
void* philosopher(void* num) {
    int i = *(int*)num;
    
    for (int c = 0; c < ITERACIONES; c++) {
        // Estado: Pensando
        printf("Filósofo %d está PENSANDO...\n", i);
        fflush(stdout);
        sleep(1); // Simula el tiempo de pensamiento

        take_forks(i); // Solicitar tenedores

        // Estado: Comiendo
        printf(">>> Filósofo %d está COMIENDO [Ciclo %d]...\n", i, c + 1);
        fflush(stdout);
        sleep(1); // Simula el tiempo de comida

        put_forks(i);  // Liberar tenedores
    }
    
    return NULL;
}

int main() {
    pthread_t hilos[N];
    int ids[N];

    // Inicialización del semáforo mutex en 1
    sem_init(&mutex, 0, 1);

    // Inicialización de los semáforos individuales en 0 y estados en THINKING
    for (int i = 0; i < N; i++) {
        sem_init(&s[i], 0, 0);
        state[i] = THINKING;
        ids[i] = i;
    }

    printf("=== INICIO DE LA CENA DE LOS FILÓSOFOS ===\n\n");

    // Crear los 5 hilos concurrentes
    for (int i = 0; i < N; i++) {
        pthread_create(&hilos[i], NULL, philosopher, &ids[i]);
    }

    // Esperar a que terminen todas las ejecuciones
    for (int i = 0; i < N; i++) {
        pthread_join(hilos[i], NULL);
    }

    // Destruir recursos de semáforos
    sem_destroy(&mutex);
    for (int i = 0; i < N; i++) {
        sem_destroy(&s[i]);
    }

    printf("\n=== TODOS LOS FILÓSOFOS HAN CENADO EXITOSAMENTE ===\n");

    return 0;
}
