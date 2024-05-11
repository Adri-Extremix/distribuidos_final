#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>


int main() {
    int pid = fork(); 

    if (pid == 0) {
        fprintf(stdin, "%s", "REGISTER A\nCONNECT A\nQUIT\n");
        close(STDOUT_FILENO);
        dup2(open("0000f.log", O_CREAT | O_RDWR), STDOUT_FILENO);
        execl("python", "", "src/cliente/client.py -s localhost -p 4500");
    } else {
        execl("python", "", "src/cliente/client.py");
    }

}