
OBJ=servidor.o common.o server_storage.o
FLAGS=-g -Wall -lrt
SRC=src/servidor/
RPCFlAGS = -g -I/usr/include/tirpc -fPIC
LDLIBS += -lnsl -lpthread -ldl -ltirpc

.PHONY: all clean DonLimpio testing

all: $(OBJ)
	@printf "\n\033[;33m\033[1mCOMPILING: GENERATING 2 FILES...\033[0m\n"
	gcc $(FLAGS)  -o servidor servidor.o server_storage.o common.o print_clnt.o $(LDLIBS)
	gcc $(FLAGS) -o servidor_rpc print_server.o print_svc.o $(LDLIBS)
	@printf "\n\033[;32m\033[1mSUCCESS\033[0m\n"
	@printf "USAGE\n-----\n\t1. Run both servers using ./servidor -p <port_number> and ./servidor_rpc\n\t2. Run python web service using python src/servicio_web/timestamp.py\n\t3. Run a client using python src/cliente/client.py -s <ip_server> -p <port_number>\n\n"

servidor.o: $(SRC)servidor.c common.o rpc
	@echo "compiling servers..."
	gcc $(RPCFlAGS) -Wall -c $(SRC)servidor.c
	gcc $(RPCFlAGS) -c src/rpc/print_server.c

rpc: 
	@printf "compiling rpc_files"
	gcc $(RPCFlAGS) -D_REENTRANT -o print_clnt.o -c src/rpc/print_clnt.c
	gcc $(RPCFlAGS) -D_REENTRANT -o print_svc.o -c src/rpc/print_svc.c
	
server_storage.o: $(SRC)server_storage.c 
	@echo "compiling server_storage..."
	gcc -c $(SRC)server_storage.c

common.o: $(SRC)common.c
	@echo "compiling common..."
	gcc -c -fPIC $<

testing: tests/tests_userList.c src/servidor/server_storage.c
	gcc -g -Wall -c src/servidor/server_storage.c
	gcc -g -Wall -c tests/tests_userList.c
	gcc -o test tests_userList.o server_storage.o
	gcc -o test_concurrency tests/tests_concurrency.c
	@./test
	@bash tests/test_imp.sh

clean:
	rm *.o

DonLimpio:
	rm *.o servidor servidor_rpc test

