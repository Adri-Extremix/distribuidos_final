
OBJ=servidor.o
FLAGS=-g -Wall -lrt
SRC=src/

.PHONY: all clean DonLimpio testing

all: $(OBJ)
	@printf "\n\033[;33m\033[1mCOMPILING: GENERATING 1 FILES...\033[0m\n"
	gcc $(FLAGS) -o servidor servidor.o imp_clave.o common.o
	@printf "\n\033[;32m\033[1mSUCCESS\033[0m\n"
	@printf "USAGE\n-----\n\t1."

servidor.o: $(SRC)servidor.c common.o
	@echo "compiling server..."
	gcc -c $(SRC)servidor.c

common.o: $(SRC)common.c
	@echo "compiling common..."
	gcc -c -fPIC $<

test: src/servidor/tests_userList.c src/servidor/server_storage.c
	gcc -g -Wall -c src/servidor/server_storage.c
	gcc -g -Wall -c src/servidor/tests_userList.c
	gcc -o test tests_userList.o server_storage.o

clean:
	rm *.o

DonLimpio:
	rm *.o

