#include <stdio.h> 
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "common.h"
#include "server_storage.h"

const int CHAR_SIZE = 256;
const int ARR_SIZE = 1024;

user_list usuarios; 

/**
 *  
 *  "REGISTER" username -> int
 *  "UNREGISTER" username -> int
 *  "CONNECT" username -> int
 *  "DISCONNECT" username -> int
 *  "PUBLISH" fileName, description, username -> int
 *  "DELETE" username, fileName -> int
 *  "LIST_USERS" -> int, username, ip, port
 *  "LIST_CONTENT", username -> int, int (num_elem), (filename, file_description) * num_elem
*/

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

typedef struct p {
    int sc;
    struct sockaddr_in client; 
} peticion; 

int tratar_peticion(void* pet) {
    int local_sc;
    char ip[32];
    int port = 0;  
    char temp[ARR_SIZE];

    pthread_mutex_lock(&mutex);
    // copia de la petición 
    local_sc = ((peticion*)pet)->sc;
    strcpy(ip, inet_ntoa(((peticion*)pet)->client.sin_addr));
    port = ntohs(((peticion*)pet)->client.sin_port);
    copiado = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    printf("tratando peticion de %i...\n", local_sc);
    // leer operacion 
    readLine(local_sc, temp, ARR_SIZE);

    if (strcmp(temp, "REGISTER") == 0) {
        // leer nombre de usuario
        printf("register\n"); 
        readLine(local_sc, temp, ARR_SIZE);
        
        //addUser(usuarios, temp, )


    } else if (strcmp(temp, "UNREGISTER") == 0) {
        printf("unregister\n"); 

    } else if (strcmp(temp, "CONNECT") == 0) {
        printf("connect\n"); 

    } else if (strcmp(temp, "DISCONNECT") == 0) {
        printf("disconnect\n"); 

    } else if (strcmp(temp, "PUBLISH") == 0) {
        printf("publish\n"); 

    } else if (strcmp(temp, "DELETE") == 0) {
        printf("delete\n"); 
        
    } else if (strcmp(temp, "LIST_USERS") == 0) {
        printf("list_users\n"); 

    } else if (strcmp(temp, "LIST_CONTENT") == 0) {
        printf("list_content\n"); 
        
    } else {
        fprintf(stderr, "server: not recognised operation (%s)\n", temp);
    }
    
    close(local_sc);
    printf("finish: %i\n", local_sc);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    pthread_mutex_init(&mutex, NULL);
    usuarios = createUserList();

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
        peticion p;
        struct sockaddr_in client;

        sc = serverAccept(sd, &client);
        p.client = client;
        p.sc = sc;

        if (sc < 0) {
            printf("Error en serverAccept\n");
            continue;
        }
        pthread_t hilo;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_create(&hilo, &attr, (void*)tratar_peticion, (void*)&p);
        pthread_mutex_lock(&mutex);
        while (!copiado) {
            pthread_cond_wait(&cond, &mutex);
        }
        copiado = 0;
        pthread_mutex_unlock(&mutex);

    }

    return 0;

}

