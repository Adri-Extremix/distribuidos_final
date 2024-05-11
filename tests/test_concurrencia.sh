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


set_server "concurrencia"
python3 src/cliente/client.py -s localhost -p 4500 < test_stressA.in &
python3 src/cliente/client.py -s localhost -p 4500 < test_stressB.in &
python3 src/cliente/client.py -s localhost -p 4500 < test_stressC.in &
python3 src/cliente/client.py -s localhost -p 4500 < test_stressD.in &
python3 src/cliente/client.py -s localhost -p 4500 < test_stressE.in &
sleep 5
kill_server
