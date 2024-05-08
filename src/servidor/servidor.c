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
 *  
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

int send_rpc(char *username, char *operacion, char* timestamp) {
    char tmp[ARR_SIZE*2]; 
    /*char timestamp[] = "17/12/2014 13:20:35"; // sustituir por conseguir timestamp*/
    sprintf(tmp, "%s\t%s\t%s", username, operacion, timestamp);

    char *rpc_ip = get_ip();
    if (NULL == rpc_ip) {
        perror("not setted ip, unable to connect to servidor_rpc (please, make sure the enviroment variable, RPC_IP, is setted up)");
        return -1;
    }

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
    char username[ARR_SIZE];
    char timestamp[ARR_SIZE];

    pthread_mutex_lock(&mutex);
    // copia de la petición 
    local_sc = ((peticion*)pet)->sc;
    strcpy(ip, inet_ntoa(((peticion*)pet)->client.sin_addr));
    port = ntohs(((peticion*)pet)->client.sin_port);
    copiado = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    // FOR DEBBUGING
    //printf("tratando peticion de %i...\n", local_sc);
    //readLine(local_sc, temp, ARR_SIZE); // read username 

    // leer operacion 
    if (readLine(local_sc, temp, ARR_SIZE) < 0) {
        perror("error: readline (coop)");
        close(local_sc);
        pthread_exit(NULL);
        return -1;
    }
    // timestamp 
    if (readLine(local_sc, timestamp, ARR_SIZE) < 0) {
        perror("error: readline register (timestamp)");
        close(local_sc);
        pthread_exit(NULL);
        return -1;
    }
    
    // leer nombre de usuario
    if (readLine(local_sc, username, ARR_SIZE) < 0) {
        perror("error: readline register (username)");
        close(local_sc);
        pthread_exit(NULL);
        return -1;
    }
    
    

    if (strcmp(temp, "REGISTER") == 0) {
        
        //printf("register %s %s %i\n", temp, ip, port); 

        pthread_mutex_lock(&mutex_hilos); // acceso a la estructura
        int result = addUser(usuarios, username, ip, port);
        pthread_mutex_unlock(&mutex_hilos);
        
        int8_t to_send_result = (int8_t)result;
        // envio de resultado => 1 byte
        if (sendMessage(local_sc, &to_send_result, 1) < 0) {
            perror("error sending ret_val");
            close(local_sc);
            pthread_exit(NULL);
            return -1;    
        }


        send_rpc(username, "REGISTER", timestamp);

    }
    else if (strcmp(temp, "UNREGISTER") == 0) {
      
        //printf("register %s %s %i\n", temp, ip, port); 

        pthread_mutex_lock(&mutex_hilos); // acceso a la estructura
        int result = removeUser(usuarios, username);
        pthread_mutex_unlock(&mutex_hilos);
        
        int8_t to_send_result = (int8_t)result;
        // envio de resultado => 1 byte
        if (sendMessage(local_sc, &to_send_result, 1) < 0) {
            perror("error sending ret_val");
            close(local_sc);
            pthread_exit(NULL);
            return -1;    
        }


        send_rpc(username, "UNREGISTER", timestamp);

    } else if (strcmp(temp, "CONNECT") == 0) {
        
        char port[ARR_SIZE];

        // lectura número de puerto
        if (readLine(local_sc, port, ARR_SIZE) < 0) {
            perror("error: readline connect (port_number)");
            close(local_sc);
            pthread_exit(NULL);
            return -1;
        } 
        //printf("connect %s %s %s\n", username, sc_ip, port); 
        int result = 0;

        // acceso a la estructura 
        pthread_mutex_lock(&mutex_hilos);
        int index = searchUser(usuarios, username);

        if (index != -1) {
            if (!usuarios->users[index].conected) {
                usuarios->users[index].conected = 1;
                usuarios->users[index].port = atoi(port);
                result = 0;
                // printf("--> %i\n", usuarios->users[index].port);
                // printf("--> %s\n", usuarios->users[index].ip);
                // printf("connect complete\n");
            }
            else {
                result = 2; // user already conected
            }
        }
        else {
            result = 1; // user does not exists
        }
        pthread_mutex_unlock(&mutex_hilos);

        int8_t to_send_result = (int8_t)result;
        // envio de resultado => 1 byte
        if (sendMessage(local_sc, &to_send_result, 1) < 0) {
            perror("error sending ret_val");
            close(local_sc);
            pthread_exit(NULL);
        }
        send_rpc(username, "CONNECT", timestamp);


    }
    else if (strcmp(temp, "DISCONNECT") == 0) {
        
        //printf("disconnect %s\n", temp); 
        int result = 0;

        // acceso a la estructura
        pthread_mutex_lock(&mutex_hilos);
        int index = searchUser(usuarios, temp);
        if (index != -1) {
            if (usuarios->users[index].conected) {
                usuarios->users[index].conected = 0;
                result = 0;
            }
            else {
                result = 2; // already disconnected
            }
        }
        else {
            result = 1; // user does not exists 
        }
        pthread_mutex_unlock(&mutex_hilos);

        int8_t to_send_result = (int8_t)result;
        // envio de resultado => 1 byte
        if (sendMessage(local_sc, &to_send_result, 1) < 0) {
            perror("error sending ret_val");
            close(local_sc);
            pthread_exit(NULL);
        }
        send_rpc(temp, "DISCONNECT", timestamp);

    }
    else if (strcmp(temp, "PUBLISH") == 0) {
        //printf("publish\n"); 
        char fileName[ARR_SIZE];
        char description[ARR_SIZE];

        // leer nombre fichero
        if (readLine(local_sc, fileName, ARR_SIZE) < 0) {
            perror("error: readline connect (fileName)");
            close(local_sc);
            pthread_exit(NULL);
            return -1;
        }

        // leer descripción fichero
        if (readLine(local_sc, description, ARR_SIZE) < 0) {
            perror("error: readline connect (description)");
            close(local_sc);
            pthread_exit(NULL);
            return -1;
        }

        pthread_mutex_lock(&mutex_hilos);
        int result = addContent(usuarios, username, fileName, description);
        pthread_mutex_unlock(&mutex_hilos);

        int8_t to_send_result = (int8_t)result;
        // envio de resultado => 1 byte
        if (sendMessage(local_sc, &to_send_result, 1) < 0) {
            perror("error sending ret_val");
            close(local_sc);
            pthread_exit(NULL);
        }

        char cadena[ARR_SIZE];
        sprintf(cadena, "%s %s", "PUBLISH", fileName);
        send_rpc(username, cadena, timestamp);

    }
    else if (strcmp(temp, "DELETE") == 0) {
        char fileName[ARR_SIZE];
        
        // leer nombre fichero
        if (readLine(local_sc, fileName, ARR_SIZE) < 0) {
            perror("error: readline connect (fileName)");
            close(local_sc);
            pthread_exit(NULL);
            return -1;
        }
        //printf("delete ( %s, %s )\n", username, fileName);

        // acceso a la estructura
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
        if (sendMessage(local_sc, &to_send_result, 1) < 0) {
            perror("error sending ret_val");
            close(local_sc);
            pthread_exit(NULL);
        }

        char cadena[ARR_SIZE];
        sprintf(cadena, "%s %a", "PUBLISH", fileName);
        send_rpc(username, cadena, timestamp);

    }
    else if (strcmp(temp, "LIST_USERS") == 0) {
        printf("list_users\n"); 
        
        int8_t to_send_result = (int8_t)0;
        // envio de resultado => 1 byte
        if (sendMessage(local_sc, &to_send_result, 1) < 0) {
            perror("error sending ret_val");
            close(local_sc);
            pthread_exit(NULL);
        }

        pthread_mutex_lock(&mutex_hilos);
        
        int num_users = 0;
        for (int i = 0; i < usuarios->size; ++i) {
            if (usuarios->users[i].conected)  num_users++;
        }
        sprintf(temp, "%i", num_users); 
        
        if (writeLine(local_sc, temp) < 0) {
                perror("error: writeline list_users (num_users)");
                close(local_sc);
                pthread_exit(NULL);
                return -1;
            }
        for (int i = 0; i < usuarios->size; ++i) {
            
            user curr = usuarios->users[i];
            if (curr.conected) {
                sprintf(temp, "%s", curr.name);
                if (writeLine(local_sc, temp) < 0) {
                    perror("error: writeline list_users (name)");
                    close(local_sc);
                    pthread_exit(NULL);
                    return -1;
                }
                sprintf(temp, "%s", curr.ip);
                if (writeLine(local_sc, temp) < 0) {
                    perror("error: writeline list_users (ip)");
                    close(local_sc);
                    pthread_exit(NULL);
                    return -1;
                }
                sprintf(temp, "%i", curr.port);
                if (writeLine(local_sc, temp) < 0) {
                    perror("error: writeline list_users (port)");
                    close(local_sc);
                    pthread_exit(NULL);
                    return -1;
                }
            }
            
        }
        pthread_mutex_unlock(&mutex_hilos);
        printf("complete list_users\n"); 
        
        send_rpc(username, "LIST_USERS", timestamp); 


    }
    else if (strcmp(temp, "LIST_CONTENT") == 0) {
        
        // leer nombre de usuario (argumento)
        if (readLine(local_sc, temp, ARR_SIZE) < 0) {
            perror("error: readline connect (username)");
            close(local_sc);
            pthread_exit(NULL);
            return -1;
        }
        //printf("list_content %s\n", username);

        // acceso a la estructura
        pthread_mutex_lock(&mutex_hilos);
        int index = searchUser(usuarios, temp);
        if (-1 == index) {
            // user does not exists 
            int8_t to_send_result = (int8_t)1;
            // envio de resultado => 1 byte
            if (sendMessage(local_sc, &to_send_result, 1) < 0) {
                perror("error sending ret_val");
                close(local_sc);
                pthread_exit(NULL);
                return -1;
            }

        }
        else {
            //printf("listando contenidos:\n");
            if (!usuarios->users[index].conected) {
                int8_t to_send_result = (int8_t)2;
                // envio de resultado => 1 byte
                if (sendMessage(local_sc, &to_send_result, 1) < 0) {
                    perror("error sending ret_val");
                    close(local_sc);
                    pthread_exit(NULL);
                    return -1;
                }
            } else {
                int8_t to_send_result = (int8_t)0;
                // envio de resultado => 1 byte
                if (sendMessage(local_sc, &to_send_result, 1) < 0) {
                    perror("error sending ret_val");
                    close(local_sc);
                    pthread_exit(NULL);
                    return -1;
                }

                sprintf(temp, "%i", usuarios->users[index].contentsLen); // tamaño de la lista
                if (writeLine(local_sc, temp) < 0) {
                    perror("error: writeLine list_content (Contentslen)");
                    close(local_sc);
                    pthread_exit(NULL);
                    return -1;
                }
                //printf("%s\n", temp);
                for (int i = 0; i < usuarios->users[index].contentsLen; ++i) {
                    //printf("--%s %s\n", usuarios->users[index].contents[i].name, usuarios->users[index].contents[i].description);

                    if (writeLine(local_sc, usuarios->users[index].contents[i].name) < 0) {
                        perror("error: writeLine list_content (FileName)");
                        close(local_sc);
                        pthread_exit(NULL);
                        return -1;
                    }

                    if (writeLine(local_sc, usuarios->users[index].contents[i].description) < 0) {
                        perror("error: writeLine list_content (description)");
                        close(local_sc);
                        pthread_exit(NULL);
                        return -1;
                    }
                }
            }
        }
        pthread_mutex_unlock(&mutex_hilos);

        send_rpc(username, "LIST_CONTENT", timestamp); 
        

    }
    else {
        fprintf(stderr, "server: not recognised operation (%s)\n", temp);
        close(local_sc);
        pthread_exit(NULL);
        return -1;
    }

    close(local_sc);
    //printf("finish: %i\n", local_sc);
    printf("Operacion aceptada de %s\n", username);
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

