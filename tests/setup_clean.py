import os 
from time import sleep
import sys
names = "ABCDE"
subprocessPID = None

prueba1A = "REGISTER A\nREGISTER B\nCONNECT C\nCONNECT A\nCONNECT B\nCONNECT A\nPUBLISH HOLA HOLA\nLIST_USERS\nLIST_CONTENT B\nLIST_CONTENT A\nQUIT\n"

prueba2A = "REGISTER A\nCONNECT A\nPUBLISH /tmp/ficheroA.txt HOLA MUNDO\nQUIT\n"

prueba2B = "REGISTER B\nCONNECT B\nGET_FILE A /tmp/ficheroA.txt /tmp/fileA.txt\nQUIT\n"

curr_dir = os.getcwd()

def set_up():
    # genera los archivos para test 
    # test 1 cliente 
    os.system(f"echo \"{prueba1A}\" >  {curr_dir}/prueba1clnt.in ")

    # test 2 clientes
    os.system("echo \"HOLA MUNDO DESDE CLIENTE A\" > /tmp/ficheroA.txt")
    os.system(f"echo \"{prueba2A}\" >  {curr_dir}/prueba2clntA.in ")
    os.system(f"echo \"{prueba2B}\" >  {curr_dir}/prueba2clntB.in ")
    # test 4 clientes
    for x, i in enumerate(names):
        # entrada de terminal
        os.system(f"echo \"register {i}\nconnect {i}\npublish /tmp/ficheroPrueba{i}.in fichero de {i}\nlist_content {names[(x + 1) % len(names)]}\nget_file {names[(x + 1) % len(names)]} /tmp/ficheroPrueba{names[(x+1)%len(names)]}.in {curr_dir}/obtenido{i}.txt\nQUIT\n\" > {curr_dir}/test_stress{i}.in\n")
        # ficheros a compartir 
        os.system(f"echo \"{i} te saluda agradablemente en una mañana de agosto, mirando las horas pasar como olas rompiendo en el acantilado.\" > /tmp/ficheroPrueba{i}.in")

def clean():
    os.system("rm /tmp/*.in")
    os.system(f" rm {curr_dir}/*.in ")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("invalid number of arguments")
        exit()

    if sys.argv[1] == "0":
        set_up()
    elif sys.argv[1] == "1":
        clean()
    else:
        print("invalid argument. Must be 0 or 1")
