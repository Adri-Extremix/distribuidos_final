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
 * @return 0 in case of success, -1 in any other case
*/
int createUser(user *usr, char *name, char *ip, int port);


typedef struct {
    int size;
    int max_size;
    user* users;
} __user_list;

typedef __user_list* user_list;

/**
 * Creates new list of users 
 * 
 * * 
 * @return created user
*/
user_list createUserList(); 

/**
 * searchs username in list 
 * 
 * @param user_name user_name
 * @param users list of users
 * 
 * @return index of user or -1 if doesn't exists
*/
int searchUser(user_list users, char *user_name);


/**
 * insert new user in list
 * 
 * @param user_name user_name
 * @param users list of users
 * @param ip ip of user
 * @param port port of user 
 * 
 * @return 0 in success, -1 in any other case 
*/
int addUser(user_list users, char *user_name, char *ip, int port);

/**
 * deletes user from a given list
 * 
 * @param user_name user_name
 * @param users list of users
 * 
 * @return 0 in success, -1 in any other case 
*/
int removeUser(user_list users, char* user_name);

/**
 * add a file to an user 
 * 
 * @param user_name user_name
 * @param users list of users
 * @param file_name file name
 * @param description description of file
 * 
 * @return 0 in success, -1 in any other case 
*/
int addContent(user_list users, char *user_name, char* file_name, char *description);

/**
 * removes a file from an user 
 * 
 * @param user_name user_name
 * @param users list of users
 * @param file_name file name
 * 
 * @return 0 in success, -1 in any other case 
*/
int removeContent(user_list users, char *user_name, char* file_name);


/**
 * destroys an existing list and its contents. Freeing the memory.
 * 
 * @param users list of users
 * 
 * @return 0 in success, -1 in any other case 
*/
int destroyList(user_list users);


#endif