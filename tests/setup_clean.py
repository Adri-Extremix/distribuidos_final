import os 
from time import sleep
import sys
names = "ABCDE"
subprocessPID = None

prueba1A = """REGISTER A
REGISTER B

CONNECT C
CONNECT A
CONNECT B
CONNECT A
PUBLISH HOLA HOLA 
LIST_USERS
LIST_CONTENT B
LIST_CONTENT A
QUIT
"""

prueba2A = """REGISTER A

CONNECT A

PUBLISH /tmp/ficheroA.txt HOLA MUNDO

"""

prueba2B = """REGISTER B

CONNECT B

GET_FILE A /tmp/ficheroA.txt /tmp/ficheroA.txt
"""

curr_dir = os.getcwd()

def set_up():
    # genera los archivos para test 
    # test 1 cliente 
    os.system(f"echo \"{prueba1A}\" >  {curr_dir}/prueba1clnt.in ")

    # test 2 clientes
    os.system("echo \"HOLA MUNDO DESDE CLIENTE A\" > /tmp/ficheroA.in")
    os.system(f"echo \"{prueba2A}\" >  {curr_dir}/prueba2clntA.in ")
    os.system(f"echo \"{prueba2B}\" >  {curr_dir}/prueba1clntB.in ")
    # test 4 clientes
    for x, i in enumerate(names):
        # entrada de terminal
        os.system(f"echo \"register {i}\nconnect {i}\npublish /tmp/ficheroPrueba{i}.in fichero de {i}\nlist_content {names[(x + 1) % len(names)]}\nget_file {names[(x + 1) % len(names)]} /tmp/ficheroPrueba{names[(x+1)%len(names)]}.in {curr_dir}/obtenido{i}.txt\n\" > {curr_dir}/test_stress{i}.in")
        # ficheros a compartir 
        os.system(f"echo \"{i} te saluda agradablemente en una mañana de agosto, mirando las horas pasar como olas rompiendo en el acantilado.\" > /tmp/ficheroPrueba{i}.in")
"""
def exec_serv(pid_list, test):
    for i in range(3):
        pid_list[i] = os.fork()
        if pid_list[i] == 0:
            match i:
                case 0:
                    # sys.stdout = open(f"rpc_{test}.log", "w")
                    os.execv(f"{curr_dir}/servidor_rpc", (" ",))
                case 1:
                    os.system("export RPC_IP=localhost")
                    # sys.stdout = open(f"svc_{test}.log", "w")
                    os.execl(f"{curr_dir}/servidor", "servidor", "-p", "4500")
                case 2:
                    print("esto es facil de encontrar")
                    # sys.stdout = open(f"timestamp_{test}.log", "w")
                    os.system([f"python3 {curr_dir}/src/servicio_web/timestamp.py"])
                    exit()

                case _:
                    continue
    return 0

def killer(pid_list):
    for pid in pid_list:
        os.system(f"kill -9 {pid}")
    for pid in pid_list:
        os.wait()
    
    


def prueba_1clnt():
    pid = os.fork()
    if pid == 0:
        os.system(f"python3 {curr_dir}/src/cliente/client.py -s localhost -p 4500 < prueba1clnt.in")
    else:
        os.wait()"""


def clean():
    os.system("rm /tmp/*.in")
    os.system(f" rm {curr_dir}/*.in ")


if __name__ == "__main__":
    if sys.argv[1] == "0":
        set_up()
    else:
        clean()
