#!/bin/bash

function set_server () {
    printf "\033[0;33mSETUP SERVERS\033[0m\n"
    env RPC_IP=localhost ./servidor -p 4500 > svc_$1.log &
    ./servidor_rpc > rpc_$1.log &
    python3 src/servicio_web/timestamp.py > tmstmp_$1.log &
}

function kill_server () {
    killall -s INT servidor
    echo "kill server"
    killall -s KILL servidor_rpc
    echo "kill python3"
    killall -s KILL python3
} 
