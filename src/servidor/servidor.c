#include <stdio.h> 
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "common.h"

const int CHAR_SIZE = 256;
const int ARR_SIZE = 1024;

/* int i = 0; */
pthread_mutex_t mutex;
pthread_cond_t cond;
int copiado = 0;
int sd = 70000;
void stop_server() {
    pthread_mutex_destroy(&mutex);
    printf("Cerrando servidor...\n");
    if (sd != 70000)
        close(sd);
    exit(0);
}

int tratar_peticion(void* sc) {
    int local_sc;

    int success;
    int op;
    int N_i;
    int key;
    char value1[CHAR_SIZE];
    int N_value2;
    double value2[ARR_SIZE];

    char buffer_local[CHAR_SIZE];

    pthread_mutex_lock(&mutex);
    // copia de la petición 
    local_sc = *((int*)sc);
    copiado = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    printf("tratando peticion de %i...\n", local_sc);
    readLine(local_sc, buffer_local, sizeof(char) + 1);
    op = buffer_local[0] - '0';
    printf(">>> op: %i\n", op);

    switch (op) {
    case 0: // init
        printf("init:\n");
        success = init();
        sprintf(buffer_local, "%i", success);
        writeLine(local_sc, buffer_local);
        break;

    case 1: // set value
        // Leemos key
        readLine(local_sc, buffer_local, CHAR_SIZE);
        key = atoi(buffer_local);

        // Leemos value1
        readLine(local_sc, buffer_local, CHAR_SIZE);
        strcpy(value1, buffer_local);

        // Leemos N_i
        readLine(local_sc, buffer_local, CHAR_SIZE);
        N_i = atoi(buffer_local);

        // Leemos value2
        for (int i = 0; i < N_i; i++) {
            readLine(local_sc, buffer_local, CHAR_SIZE);
            value2[i] = atof(buffer_local);
        }

        // Obtenemos el resultado de la operación y escribimos al cliente el resultado
        success = set_value(key, value1, N_i, value2);
        sprintf(buffer_local, "%i", success);
        writeLine(local_sc, buffer_local);
        break;

    case 2: // get value

        readLine(local_sc, buffer_local, CHAR_SIZE);
        key = atoi(buffer_local);

        success = get_value(key, value1, &N_i, value2);

        // Escribimos value 1
        strcpy(buffer_local, value1);
        writeLine(local_sc, buffer_local);
        // Escribimos N_value 2
        sprintf(buffer_local, "%i", N_i);
        writeLine(local_sc, buffer_local);

        // Escribimos value2
        for (int i = 0; i < N_i; i++) {
            sprintf(buffer_local, "%lf", value2[i]);
            writeLine(local_sc, buffer_local);
        }

        // Escribimos el resultado de la operación
        sprintf(buffer_local, "%i", success);
        writeLine(local_sc, buffer_local);

        break;

    case 3: // modify value
        // Leemos key
        readLine(local_sc, buffer_local, CHAR_SIZE);
        key = atoi(buffer_local);

        // Leemos value1
        readLine(local_sc, buffer_local, CHAR_SIZE);
        strcpy(value1, buffer_local);

        // Leemos N_i
        readLine(local_sc, buffer_local, CHAR_SIZE);
        N_i = atoi(buffer_local);

        // Leemos value2
        for (int i = 0; i < N_i; i++) {
            readLine(local_sc, buffer_local, CHAR_SIZE);
            value2[i] = atof(buffer_local);
        }

        // Obtenemos el resultado de la operación y escribimos al cliente el resultado
        success = modify_value(key, value1, N_i, value2);
        sprintf(buffer_local, "%i", success);
        writeLine(local_sc, buffer_local);
        break;

    case 4: // delete key

        // Leemos la key
        readLine(local_sc, buffer_local, CHAR_SIZE);
        key = atoi(buffer_local);

        // Escribimos el resultado de la operación
        success = delete_key(key);
        sprintf(buffer_local, "%i", success);
        writeLine(local_sc, buffer_local);
        break;

    case 5: // exists

        // Leemos la key
        readLine(local_sc, buffer_local, CHAR_SIZE);
        key = atoi(buffer_local);

        // Escribimos el resultado de la operación
        success = exist(key);
        sprintf(buffer_local, "%i", success);
        writeLine(local_sc, buffer_local);
        break;

    default:
        fprintf(stderr, "Not recognised operation: expected [0, 5] but %d was received\n", op);
        break;
    }
    close(local_sc);
    printf("finish: %i\n", local_sc);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {

    signal(SIGINT, stop_server);
    int sc;
    if (argc != 2) {
        perror("Uso: ./servidor <puerto>");
        return -1;
    }

    sd = serverSocket(INADDR_ANY, atoi(argv[1]), SOCK_STREAM);

    if (sd < 0) {
        printf("SERVER: Error en serverSocket\n");
        return 0;
    }
    while (!!1) {
        sc = serverAccept(sd);
        if (sc < 0) {
            printf("Error en serverAccept\n");
            continue;
        }
        pthread_t hilo;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_create(&hilo, &attr, (void*)tratar_peticion, (void*)&sc);
        pthread_mutex_lock(&mutex);
        while (!copiado) {
            pthread_cond_wait(&cond, &mutex);
        }
        copiado = 0;
        pthread_mutex_unlock(&mutex);

    }

    return 0;

}

