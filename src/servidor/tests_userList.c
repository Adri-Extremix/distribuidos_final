#include <stdio.h>
#include "server_storage.h"

int main() {
    printf("1\n"); 
    user_list lista; 
    printf("1\n"); 
    lista = createUserList();
    printf("lista: %d %d\n", lista->size, lista->max_size);
    searchUser(lista, "Adrian");// error por vacía
    printf("1\n");
    addUser(lista, "Cesar", "localhost", 4000);
    addUser(lista, "Cesar", "localhost", 4000 ); // error por duplicado
    addUser(lista, "Adrian", "localhost", 4000);
    // error por longitud
    addUser(lista, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", "localhost", 4000);
    
    addUser(lista, "Otro", "localhost", 4000);

    printf("\ndespués de add\n[");
    for (int i = 0; i < lista->size; ++i) {
        printf("%s, ", lista->users[i].name);
    }
    printf("]\n\n");


    printf("seach adrian: %d", searchUser(lista, "Adrian"));
    searchUser(lista, "Pepe"); // error por no existencia

    removeUser(lista, "Otro");

    printf("\ndespués de remove\n[");
    for (int i = 0; i < lista->size; ++i) {
        printf("%s, ", lista->users[i].name);
    }
    printf("]\n\n");

    removeUser(lista, "Pepe"); // error por no existencia

    addContent(lista, "Otro", "pito", "pitopito"); // error no existe usuario
    addContent(lista, "Cesar", "pito", "pitopito"); 
    addContent(lista, "Cesar", "holabuenas", "adios"); 
    addContent(lista, "Cesar", "pito", "pitopito"); // error fichero que ya existe
    printf("\ndespués de addContent\n[");
    for (int i = 0; i < lista->size; ++i) {
        printf("%s: ", lista->users[i].name);
        for (int j = 0; j < lista->users[i].contentsLen; ++j) {
            printf("(%s, %s), ", lista->users[i].contents[j].name, lista->users[i].contents[j].description);
        }
        printf("; ");

    }
    printf("]\n\n");

    removeContent(lista, "Otro", "pito"); // fallo no existencia user
    removeContent(lista, "Adrian", "pito"); // fallo no existencia archivo
    removeContent(lista, "Cesar", "pito"); 

    printf("\ndespués de removeContent\n[");
    for (int i = 0; i < lista->size; ++i) {
        printf("%s: ", lista->users[i].name);
        for (int j = 0; j < lista->users[i].contentsLen; ++j) {
            printf("(%s, %s), ", lista->users[i].contents[j].name, lista->users[i].contents[j].description);
        }
        printf("; ");

    }
    printf("]\n\n");
    printf("lista: %d %d\n", lista->size, lista->max_size);
    destroyList(lista);


}