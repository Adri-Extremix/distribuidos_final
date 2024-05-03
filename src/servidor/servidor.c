#include <stdio.h> 
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "common.h"
#include "server_storage.h"
#include "../rpc/print.h"

#define RPC_IP "RPC_IP"

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
pthread_mutex_t mutex_hilos;
pthread_cond_t cond_hilos;
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

char* get_ip() {
    char* ip = getenv(RPC_IP);
    if (ip == NULL) {
        perror("NOT setted ip");
        return NULL;
    }
    return ip;
}

int send_rpc(char *username, char *operacion) {
    char tmp[ARR_SIZE*2]; 
    char timestamp[] = "17/12/2014 13:20:35"; // sustituir por conseguir timestamp
    sprintf(tmp, "%s\t%s\t%s", username, operacion, timestamp);

    char *rpc_ip = get_ip();

    CLIENT *clnt = clnt_create (rpc_ip, PRINT, PRINTVER, "udp");
	if (clnt == NULL) {
		clnt_pcreateerror (rpc_ip);
		perror("unable to connect to rpc_server");
	}
    int return_value = 234;
    enum clnt_stat retval_1 = rpc_print_1(tmp, &return_value, clnt);
    if (retval_1 != RPC_SUCCESS) {
		clnt_perror (clnt, "call failed");
	}
    return return_value;
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

    //printf("tratando peticion de %i...\n", local_sc);
    //readLine(local_sc, temp, ARR_SIZE); // read username 
    printf("s> OPERATION FROM %s...\n", ip);
    // leer operacion 
    readLine(local_sc, temp, ARR_SIZE);

    if (strcmp(temp, "REGISTER") == 0) {
        // leer nombre de usuario
        readLine(local_sc, temp, ARR_SIZE);
        //printf("register %s %s %i\n", temp, ip, port); 

        pthread_mutex_lock(&mutex_hilos); // acceso a la estructura
        int result = addUser(usuarios, temp, ip, port);
        pthread_mutex_unlock(&mutex_hilos);
        int8_t to_send_result = (int8_t)result;
        // envio de resultado => 1 byte
        sendMessage(local_sc, &to_send_result, 1);

        send_rpc(temp, "REGISTER");

    }
    else if (strcmp(temp, "UNREGISTER") == 0) {
        // leer nombre de usuario
        readLine(local_sc, temp, ARR_SIZE);
        //printf("unregister %s\n", temp);

        pthread_mutex_lock(&mutex_hilos);
        int result = removeUser(usuarios, temp);
        pthread_mutex_unlock(&mutex_hilos);


        int8_t to_send_result = (int8_t)result;
        // envio de resultado => 1 byte
        sendMessage(local_sc, &to_send_result, 1);
        send_rpc(temp, "UNREGISTER");

    } else if (strcmp(temp, "CONNECT") == 0) {
        char username[ARR_SIZE];
        char port[ARR_SIZE];
        char sc_ip[ARR_SIZE];
        readLine(local_sc, username, ARR_SIZE); // username
        readLine(local_sc, sc_ip, ARR_SIZE); // ip
        readLine(local_sc, port, ARR_SIZE); // port
        printf("connect %s %s %s\n", username, sc_ip, port); 
        int result = 0;

        pthread_mutex_lock(&mutex_hilos);
        int index = searchUser(usuarios, username);

        if (index != -1) {
            if (!usuarios->users[index].conected) {
                usuarios->users[index].conected = 1;
                usuarios->users[index].port = atoi(port);
                strcpy(usuarios->users[index].ip, sc_ip);
                result = 0;
                printf("--> %i\n", usuarios->users[index].port);
                printf("--> %s\n", usuarios->users[index].ip);
                //printf("connect complete\n");
            }
            else {
                result = 2;
            }
        }
        else {
            result = 1;
        }
        pthread_mutex_unlock(&mutex_hilos);

        int8_t to_send_result = (int8_t)result;
        // envio de resultado => 1 byte
        sendMessage(local_sc, &to_send_result, 1);
        send_rpc(username, "CONNECT");


    }
    else if (strcmp(temp, "DISCONNECT") == 0) {
        readLine(local_sc, temp, ARR_SIZE); // username
        //printf("disconnect %s\n", temp); 
        int result = 0;

        pthread_mutex_lock(&mutex_hilos);
        int index = searchUser(usuarios, temp);
        if (index != -1) {
            if (usuarios->users[index].conected) {
                usuarios->users[index].conected = 0;
                result = 0;
            }
            else {
                result = 2;
            }
        }
        else {
            result = 1;
        }
        pthread_mutex_unlock(&mutex_hilos);

        int8_t to_send_result = (int8_t)result;
        // envio de resultado => 1 byte
        sendMessage(local_sc, &to_send_result, 1);
        send_rpc(temp, "DISCONNECT");

    }
    else if (strcmp(temp, "PUBLISH") == 0) {
        //printf("publish\n"); 
        char fileName[ARR_SIZE];
        char description[ARR_SIZE];
        char username[ARR_SIZE];
        readLine(local_sc, username, ARR_SIZE); // read username
        printf("Este es el nombre del usuario %s\n", username);
        readLine(local_sc, fileName, ARR_SIZE); // read fileName
        readLine(local_sc, description, ARR_SIZE); // read description

        pthread_mutex_lock(&mutex_hilos);
        int result = addContent(usuarios, username, fileName, description);
        pthread_mutex_unlock(&mutex_hilos);

        int8_t to_send_result = (int8_t)result;
        // envio de resultado => 1 byte
        sendMessage(local_sc, &to_send_result, 1);
        
        char cadena[ARR_SIZE];
        sprintf(cadena, "%s %s", "PUBLISH", fileName);
        send_rpc(username, cadena);

    }
    else if (strcmp(temp, "DELETE") == 0) {
        char fileName[ARR_SIZE];
        char username[ARR_SIZE];
        readLine(local_sc, username, ARR_SIZE); // read username
        readLine(local_sc, fileName, ARR_SIZE); // read fileName
        //printf("delete ( %s, %s )\n", username, fileName);

        pthread_mutex_lock(&mutex_hilos);
        int index = searchUser(usuarios, username);
        int result = 0;
        if (index != -1) {
            if (usuarios->users[index].conected) {
                result = removeContent(usuarios, username, fileName);

            }
            else {
                result = 2;
            }
        }
        else {
            result = 1;
        }
        pthread_mutex_unlock(&mutex_hilos);

        int8_t to_send_result = (int8_t)result;
        // envio de resultado => 1 byte
        sendMessage(local_sc, &to_send_result, 1);
        char cadena[ARR_SIZE];
        sprintf(cadena, "%s %a", "PUBLISH", fileName);
        send_rpc(username, cadena);

    }
    else if (strcmp(temp, "LIST_USERS") == 0) {
        printf("list_users\n"); 

        int8_t to_send_result = (int8_t)0;
        // envio de resultado => 1 byte
        sendMessage(local_sc, &to_send_result, 1);

        pthread_mutex_lock(&mutex_hilos);
        
        int num_users = 0;
        for (int i = 0; i < usuarios->size; ++i) {
            if (usuarios->users[i].conected)  num_users++;
        }
        sprintf(temp, "%i", num_users); // se procesa la operacion correctamente
        
        if (writeLine(local_sc, temp) < 0) {
                perror("error");
            }
        for (int i = 0; i < usuarios->size; ++i) {
            
            user curr = usuarios->users[i];
            if (curr.conected) {
                sprintf(temp, "%s", curr.name);
                if (writeLine(local_sc, temp) < 0) {
                    perror("error");
                }
                sprintf(temp, "%s", curr.ip);
                if (writeLine(local_sc, temp) < 0) {
                    perror("error");
                }
                sprintf(temp, "%i", curr.port);
                if (writeLine(local_sc, temp) < 0) {
                    perror("error");
                }
            }
            
        }
        pthread_mutex_unlock(&mutex_hilos);
        printf("complete list_users\n"); 
        
        send_rpc("PACO", "LIST_USERS"); // TODO: NOMBRE USUARIO


    }
    else if (strcmp(temp, "LIST_CONTENT") == 0) {
        readLine(local_sc, temp, ARR_SIZE); // username
        //printf("list_content %s\n", temp);
        pthread_mutex_lock(&mutex_hilos);
        int index = searchUser(usuarios, temp);
        if (-1 == index) {
            // user does not exists 
            int8_t to_send_result = (int8_t)1;
            // envio de resultado => 1 byte
            sendMessage(local_sc, &to_send_result, 1);
            //printf("User does not exists\n");

        }
        else {
            //printf("listando contenidos:\n");

            int8_t to_send_result = (int8_t)0;
            // envio de resultado => 1 byte
            sendMessage(local_sc, &to_send_result, 1);

            sprintf(temp, "%i", usuarios->users[index].contentsLen); // tamaño de la lista
            if (writeLine(local_sc, temp) < 0) {
                //printf("error shoket\n");
            }
            //printf("%s\n", temp);
            for (int i = 0; i < usuarios->users[index].contentsLen; ++i) {
                //printf("--%s %s\n", usuarios->users[index].contents[i].name, usuarios->users[index].contents[i].description);

                writeLine(local_sc, usuarios->users[index].contents[i].name);
                writeLine(local_sc, usuarios->users[index].contents[i].description);
            }
        }
        pthread_mutex_unlock(&mutex_hilos);

        send_rpc("PACO", "LIST_CONTENT"); // TODO: NOMBRE USUARIO
        

    }
    else {
        fprintf(stderr, "server: not recognised operation (%s)\n", temp);
    }

    close(local_sc);
    //printf("finish: %i\n", local_sc);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    //printf(">>%i\n", send_rpc("paco", "deposit 45"));
    // validate program input 
    if (argc != 3) {
        printf("USAGE: %s %s %s\n", argv[0], "-p", "<port_number>");
        return -1;
    }
    else if (strcmp(argv[1], "-p") != 0) {
        printf("USAGE: %s %s %s\n", argv[0], "-p", "<port_number>");
        return -1;
    }
    else if (atoi(argv[2]) == 0) {
        printf("Port number must be numeric and greater than 0\n");
        return -1;
    }
    else {
        //everything alright 
    }

    // print ip
    char hostname[256];

    gethostname(hostname, sizeof(hostname));
    struct hostent* hp = gethostbyname(hostname);
    struct in_addr ip;
    bcopy(hp->h_addr, (char*)&ip, sizeof(ip));
    char* ip_str = inet_ntoa(ip);

    printf("s> init server %s:%s\n", ip_str, argv[2]);

    // initializations
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutex_hilos, NULL);
    usuarios = createUserList();

    signal(SIGINT, stop_server);
    int sc;

    sd = serverSocket(INADDR_ANY, atoi(argv[2]), SOCK_STREAM);

    if (sd < 0) {
        perror("SERVER: Error en serverSocket\n");
        return 0;
    }
    while (!!1) {
        peticion p;
        struct sockaddr_in client;
        printf("s>\n");

        sc = serverAccept(sd, &client);
        p.client = client;
        p.sc = sc;

        if (sc < 0) {
            perror("Error en serverAccept\n");
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

