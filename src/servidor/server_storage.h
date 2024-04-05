#ifndef STORAGE_H
#define STORAGE_H

#define CHARSIZE 256

typedef struct {
    char name[CHARSIZE];
    char description[CHARSIZE];

} file;

typedef struct {
    char name[CHARSIZE];
    file* contents;
    int contentsLen;
    int contentsMaxLen;
    int conected;
    int port;
    char ip[32];

} user;

/**
 * Creates new user with memory region for contents. 
 * 
 * @param name user_name
 * @param ip ip of user
 * @param port port number
 * 
 * @return created user
*/
int createUser(user *usr, char *name, char *ip, int port);


typedef struct {
    int size;
    int max_size;
    user* users;
} __user_list;

typedef __user_list* user_list;

user_list createUserList(); 

int searchUser(user_list users, char *user_name);

int addUser(user_list users, char *user_name, char *ip, int port);

int removeUser(user_list users, char* user_name);

int addContent(user_list users, char *user_name, char* file_name, char *description);

int removeContent(user_list users, char *user_name, char* file_name);

int destroyList(user_list users);


#endif