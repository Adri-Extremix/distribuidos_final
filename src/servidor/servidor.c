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
 *  "LIST_USERS" -> int (result), int (num_users), (username, ip, port)*num_users
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
        readLine(local_sc, temp, ARR_SIZE);
        printf("register %s %s %i\n", temp, ip, port); 
        int result = addUser(usuarios, temp, ip, port);
        sprintf(temp, "%i", result);
        writeLine(local_sc, temp);

    } else if (strcmp(temp, "UNREGISTER") == 0) {
         // leer nombre de usuario
        readLine(local_sc, temp, ARR_SIZE);
        printf("unregister %s\n", temp);
        int result = removeUser(usuarios, temp);
        sprintf(temp, "%i", result);
        writeLine(local_sc, temp);

    } else if (strcmp(temp, "CONNECT") == 0) {
        readLine(local_sc, temp, ARR_SIZE); // username
        printf("connect %s\n", temp); 
        int result = 0;
        int index = searchUser(usuarios, temp);
        if (index != -1) {
            if (! usuarios->users[index].conected) {
                usuarios->users[index].conected = 1;
                result = 0;
                printf("connect complete\n");
            } else {
                result = 2;
            }
        } else {
            result = 1;
        }
        sprintf(temp, "%i", result);
        writeLine(local_sc, temp);


    } else if (strcmp(temp, "DISCONNECT") == 0) {
        readLine(local_sc, temp, ARR_SIZE); // username
        printf("disconnect %s\n", temp); 
        int result = 0;
        int index = searchUser(usuarios, temp);
        if (index != -1) {
            if ( usuarios->users[index].conected) {
                usuarios->users[index].conected = 0;
                result = 0;
            } else {
                result = 2;
            }
        } else {
            result = 1;
        }
        sprintf(temp, "%i", result);
        writeLine(local_sc, temp);

    } else if (strcmp(temp, "PUBLISH") == 0) {
        printf("publish\n"); 
        char fileName[ARR_SIZE];
        char description[ARR_SIZE];
        char username[ARR_SIZE];
        readLine(local_sc, fileName, ARR_SIZE); // read fileName
        readLine(local_sc, description, ARR_SIZE); // read description
        readLine(local_sc, username, ARR_SIZE); // read username
        int result = addContent(usuarios, username, fileName, description);
        sprintf(temp, "%i", result);
        writeLine(local_sc, temp);

    } else if (strcmp(temp, "DELETE") == 0) {
        char fileName[ARR_SIZE];
        char username[ARR_SIZE];
        readLine(local_sc, username, ARR_SIZE); // read username
        readLine(local_sc, fileName, ARR_SIZE); // read fileName
        printf("delete ( %s, %s )\n", username, fileName);
        int index = searchUser(usuarios, username);
        int result = 0;
        if (index != -1) {
            if ( usuarios->users[index].conected) {
                result = removeContent(usuarios, username, fileName);
                
            } else {
                result = 2;
            }
        } else {
            result = 1;
        }
        sprintf(temp, "%i", result);
        writeLine(local_sc, temp);

        
    } else if (strcmp(temp, "LIST_USERS") == 0) {
        printf("list_users\n"); 

        sprintf(temp, "%i", 0); // se procesa la operacion correctamente
        writeLine(local_sc, temp);

        int num_users = usuarios->size;
        for (int i = 0; i < num_users; ++i) {
            user curr = usuarios->users[i];
            sprintf(temp, "%s", curr.name);
            writeLine(local_sc, temp);
            sprintf(temp, "%s", curr.ip);
            writeLine(local_sc, temp);
            sprintf(temp, "%i", curr.port);
            writeLine(local_sc, temp);
        }


    } else if (strcmp(temp, "LIST_CONTENT") == 0) {
        readLine(local_sc, temp, ARR_SIZE); // username
        printf("list_content %s\n", temp);
        int index = searchUser(usuarios, temp);
        if (-1 == index) {
            // user does not exists 
            sprintf(temp, "%i", 1);
            printf("User does not exists\n");

        } else {
            printf("listando contenidos:\n");

            sprintf(temp, "%i", 0);
            writeLine(local_sc, temp);
            
            sprintf(temp, "%i", usuarios->users[index].contentsLen); // tamaño de la lista
            if(writeLine(local_sc, temp) < 0) {
                printf("error shoket\n");
            }
            printf("%s\n", temp);
            for (int i = 0; i < usuarios->users[index].contentsLen; ++i) {
                printf("--%s %s\n", usuarios->users[index].contents[i].name, usuarios->users[index].contents[i].description);

                writeLine(local_sc, usuarios->users[index].contents[i].name);
                writeLine(local_sc, usuarios->users[index].contents[i].description);
            }
        }

        
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

