#!/bin/bash

function set_server () {
    env RPC_IP=localhost ./servidor -p 4500 > svc_$1.log &
    ./servidor_rpc > rpc_$1.log &
    python3 src/servicio_web/timestamp.py > tmstmp_$1.log &
}

function kill_server () {
    killall -s INT  servidor
    killall -s INT servidor_rpc
    killall -s INT python3
} 

printf "\033[0;33mSETUP\033[0m\n"
python3 tests/setup_clean.py "0"

# implementacion 
printf "\033[0;33mRUNNING: test implementacion\033[0m\n";
./test > implementacion.log

# test 1

printf "\033[0;33mRUNNING: 1 cliente\033[0m\n";

set_server "1client"

python3 src/cliente/client.py -s localhost -p 4500 < prueba1clnt.in > 1client.log

kill_server

# test 2





python3 tests/setup_clean.py "1"


    