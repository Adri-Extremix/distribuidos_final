#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "server_storage.h"

#define false 0
#define true 1

#define DEFAULT_SIZE 256

int createUser(user *usr, char *name, char *ip, int port) {
    /*validacion*/
    if (strlen(name) < 1 || strlen(name) > CHARSIZE) {
        fprintf(stderr, "createUser: user_name not valid\n");
        return -1;
    }

    if (strlen(ip) < 1 || strlen(ip) > CHARSIZE) {
        fprintf(stderr, "createUser: ip not valid\n");
    }

    strcpy(usr->name, name);
    usr->conected = false;
    usr->contentsMaxLen = DEFAULT_SIZE;
    usr->contents = (file*)malloc(usr->contentsMaxLen*sizeof(file));
    usr->contentsLen = 0; 
    usr->port = port;
    strcpy(usr->ip, ip);
    return 0;

}

int searchContent(user *generoso, char *file_name) {
    for (int i = 0; i < generoso->contentsLen; ++i) {
        if (strcmp(file_name, generoso->contents[i].name) == 0) {
            return i;

        }
    }
    perror("searchContent: file not found");
    return -1;
}

int expandContents(user *usr) {
    usr->contentsMaxLen *= 2;
    usr->contents = (file*)realloc(usr->contents, usr->contentsMaxLen*sizeof(file));
    return 0;
}

user_list createUserList() {
    __user_list list;
    list.max_size = 256;
    list.size = 0; 
    list.users = (user*)malloc(list.max_size*sizeof(user));
    user_list plist= (user_list)malloc(sizeof(__user_list));
    *plist = list;
    return plist;

}

int expandSpace(user_list users) {
    users->max_size *= 2;
    users->users = realloc(users->users, users->max_size*sizeof(user));
    return 0;
}

int destroyList(user_list users) {
    for (int i = 0; i < users->size; ++i) {
        free(users->users[i].contents);
    }
    free(users);
    return 0;
}

int searchUser(user_list users, char *user_name) {
    for (int i = 0; i < users->size; ++i) {
        if (strcmp(users->users[i].name, user_name) == 0) {
            return i;
        }
    }
    perror("searchUser: user not found");
    return -1;
}

int addUser(user_list users, char *user_name, char *ip, int port) {
    if (users->size == users->max_size) {
        expandSpace(users);
    }
    if (searchUser(users, user_name) != -1) {
        perror("addUser: usuario ya existente");
        return 1;
    }
    user newusr;
    if (createUser(&newusr, user_name, ip, port) == 0) {
        users->users[users->size++] = newusr;
        return 0;
    }
    perror("addUser: error in createUser");
    return 2;
}

int removeUser(user_list users, char *user_name) {
    int index = searchUser(users, user_name); // si la lista está vacía se detectará aquí
    if (index == -1) {
        perror("removeUser: error in searchUser");
        return 1;
    }
    user penitente = users->users[index];
    free(penitente.contents);
    users->users[index] = users->users[--(users->size)];
    return 0;
}

int addContent(user_list users, char *user_name, char* file_name, char* description) {
    /*validacion*/
    if (strlen(file_name) < 1 || strlen(file_name) > CHARSIZE) {
        fprintf(stderr, "addContent: file_name not valid\n");
        return 4;
    }

    if (strlen(description) < 1 || strlen(description) > CHARSIZE) {
        fprintf(stderr, "addContent: description not valid\n");
        return 4;
    }
    int index = searchUser(users, user_name);
    if (index == -1) {
        perror("addContent: error in searchUser");
        return 1;
    }
    if (!users->users[index].conected) {
        perror("addContent: user not connected");
        return 2;
    }
    user *generoso = &(users->users[index]);
    if (generoso->contentsLen == generoso->contentsMaxLen) {
        expandContents(generoso);
    }
    int i = searchContent(generoso, file_name);
    if (i != -1) {
        perror("addContent: file already exists");
        return 3;
    }
    strcpy(generoso->contents[generoso->contentsLen].name, file_name);
    strcpy(generoso->contents[generoso->contentsLen].description, description);
    ++(generoso->contentsLen);
    return 0;
}

int removeContent(user_list users, char *user_name, char* file_name) {
    /*validacion*/
    if (strlen(file_name) < 1 || strlen(file_name) > CHARSIZE) {
        fprintf(stderr, "removeContent: filename not valid\n");
        return 4;
    }

    int index = searchUser(users, user_name);
    if (index == -1) {
        perror("removeUser: error in searchUser");
        return 1;
    }
    user *generoso = &(users->users[index]);
    int i = searchContent(generoso, file_name);
    if (i == -1) {
        perror("removeContent: file not found");
        return 3;
    }

    generoso->contents[i] = generoso->contents[generoso->contentsLen - 1];
    --(generoso->contentsLen);
    return 0;
}


