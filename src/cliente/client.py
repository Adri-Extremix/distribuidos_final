from enum import Enum
import argparse
import socket
import threading
import sys

class client :

    # ******************** TYPES *********************
    # *
    # * @brief Return codes for the protocol methods
    class RC(Enum) :
        OK = 0
        ERROR = 1
        USER_ERROR = 2

    # ****************** ATTRIBUTES ******************
    _server = None
    _port = -1
    _sock = None
    _username = None
    # ******************** METHODS *******************

    @staticmethod
    def __read_string(serv_sock):
        message = ""
        while True:
            msg = serv_sock.recv(1)
            if(msg == b'\0'):
                break
            message += msg.decode()
        message = message + '\0'

        return message


    @staticmethod
    def __get_socket():

        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server,client._port)
        sock.connect(server_address)
        return sock

    @staticmethod
    def __reg_unreg(user:str, reg_unreg:str):

        serv_sock = client.__get_socket()
        try:
            message = f"{reg_unreg}\0".encode()
            serv_sock.sendall(message)

            message = f"{user}\0".encode()
            serv_sock.sendall(message)

            message = ""

            msg = int(serv_sock.recv(1).decode())

        except Exception as e:
            msg = 2
            
        finally:
            serv_sock.close()

        match msg:
            case 0:
                print(f"{reg_unreg} OK")
            case 1:
                if reg_unreg == "REGISTER":
                    print("USERNAME IN USE")
                else:
                    print("USER DOES NOT EXIST")
            case 2:
                print(f"{reg_unreg} FAIL")

        return msg
    
    @staticmethod
    def  register(user) :

        return client.__reg_unreg(user,"REGISTER")
   
    @staticmethod
    def  unregister(user) :
        
        return client.__reg_unreg(user,"UNREGISTER")

    @staticmethod
    def __client_listen(sock):

        sock.listen(5)

        while not not True:
            try:
                connection, client_address = sock.accept()
                try:
                    message = client.__read_string(connection)
                    print("POR TERMINAR")
                

                finally:
                    connection.close()
            except OSError:
                print("Socket cerrado mientras se esperaba una conexión")
                return
        

    @staticmethod
    def  connect(user) :
        serv_sock = client.__get_socket()

        try:
            message = b'CONNECT\0'
            serv_sock.sendall(message)

            message = f"{user}\0".encode()
            serv_sock.sendall(message)

            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            
            client._sock = sock

            ip_address = socket.gethostbyname(socket.gethostname())
            server_address = (ip_address,0)
            sock.bind(server_address)

            address, port = sock.getsockname()

            message = f"{port}\0".encode()
            serv_sock.sendall(message)

            message = ""

            msg = int(serv_sock.recv(1).decode())

            if (client._username):
                msg = 3
            match msg:
                case 0:
                    print("CONNECT OK")
                    client._username = user
                    p2p_thread = threading.Thread(target=client.__client_listen,args=(sock))
                    p2p_thread.start()
                case 1:
                    print("CONNECT FAIL, USER DOES NOT EXIST")
                    sock.close()
                case 2:
                    print("USER ALREADY CONNECTED")
                    sock.close()
                case 3:
                    print("CONNECT FAIL")
                    sock.close()

        finally:
            serv_sock.close()

        return msg


    
    @staticmethod
    def  disconnect(user) :

        serv_sock = client.__get_socket()

        try:
            message = b'DISCONNECT\0'
            serv_sock.sendall(message)

            message = f"{user}\0".encode()
            serv_sock.sendall(message)

            message = ""

            msg = int(serv_sock.recv(1).decode())

            match msg:
                case 0:
                    print("DISCONNECT OK")
                    client._sock.close()
                case 1:
                    print("DISCONNECT FAIL / USER DOES NOT EXIST")
                case 2:
                    print("DISCONNECT FAIL / USER NOT CONNECTED")
                case 3:
                    print("DISCONNECT FAIL")

        finally:
            serv_sock.close()

        return msg

    @staticmethod
    def  publish(fileName,  description) :
        
        serv_sock = client.__get_socket()

        try:
            message = b'PUBLISH\0'
            serv_sock.sendall(message)

            message = f"{fileName}\0".encode()
            serv_sock.sendall(message)

            message = f"{description}\0".encode()
            serv_sock.sendall(message)

            message = f"{client._username}\0".encode()
            serv_sock.sendall(message)

            message = ""

            msg = int(serv_sock.recv(1).decode())

            match msg:
                case 0:
                    print("PUBLISH OK")
                    client._sock.close()
                case 1:
                    print("PUBLISH FAIL, USER DOES NOT EXIST")
                case 2:
                    print("PUBLISH FAIL, USER NOT CONNECTED")
                case 3:
                    print("PUBLISH FAIL, CONTENT ALREADY PUBLISHED")
                case 4:
                    print("PUBLISH FAIL")
        finally:
            serv_sock.close()
        
        return msg

    @staticmethod
    def  delete(fileName) :

        serv_sock = client.__get_socket()

        try:
            message = b'DELETE\0'
            serv_sock.sendall(message)

            message = f"{client._username}\0".encode()
            serv_sock.sendall(message)

            message = f"{fileName}\0".encode()
            serv_sock.sendall(message)

            message = ""

            msg = int(serv_sock.recv(1).decode())

            match msg:
                case 0:
                    print("DELETE OK")
                    client._sock.close()
                case 1:
                    print("DELETE FAIL, USER DOES NOT EXIST")
                case 2:
                    print("DELETE FAIL, USER NOT CONNECTED")
                case 3:
                    print("DELETE FAIL, CONTENT NOT PUBLISHED")
                case 4:
                    print("DELETE FAIL")
        finally:
            serv_sock.close()
        
        return msg

    @staticmethod
    def  listusers() :

        serv_sock = client.__get_socket()

        try:
            message = b'LIST_USERS\0'
            serv_sock.sendall(message)

            message = f"{client._username}\0".encode()
            serv_sock.sendall(message)

            message = ""

            msg = int(serv_sock.recv(1).decode())

            match msg:
                case 0:
                    print("LIST_USERS OK")

                    num = int(message)

                    for i in range(num):
                        username = client.__read_string(serv_sock)
                        ip = client.__read_string(serv_sock)
                        port = client.__read_string(serv_sock)

                        print(f"{username} {ip} {port}")

                    client._sock.close()
                case 1:
                    print("LIST_USERS FAIL, USER DOES NOT EXIST")
                case 2:
                    print("LIST_USERS FAIL, USER NOT CONNECTED")
                case 3:
                    print("DELETE FAIL")
        
        finally:
            serv_sock.close()

        return msg

    @staticmethod
    def  listcontent(user) :
        #  Write your code here
        return client.RC.ERROR

    @staticmethod
    def  getfile(user,  remote_FileName,  local_FileName) :
        #  Write your code here
        return client.RC.ERROR

    # *
    # **
    # * @brief Command interpreter for the client. It calls the protocol functions.
    @staticmethod
    def shell():

        while (True) :
            try :
                command = input("c> ")
                line = command.split(" ")
                if (len(line) > 0):

                    line[0] = line[0].upper()

                    if (line[0]=="REGISTER") :
                        if (len(line) == 2) :
                            client.register(line[1])
                        else :
                            print("Syntax error. Usage: REGISTER <userName>")

                    elif(line[0]=="UNREGISTER") :
                        if (len(line) == 2) :
                            client.unregister(line[1])
                        else :
                            print("Syntax error. Usage: UNREGISTER <userName>")

                    elif(line[0]=="CONNECT") :
                        if (len(line) == 2) :
                            client.connect(line[1])
                        else :
                            print("Syntax error. Usage: CONNECT <userName>")
                    
                    elif(line[0]=="PUBLISH") :
                        if (len(line) >= 3) :
                            #  Remove first two words
                            description = ' '.join(line[2:])
                            client.publish(line[1], description)
                        else :
                            print("Syntax error. Usage: PUBLISH <fileName> <description>")

                    elif(line[0]=="DELETE") :
                        if (len(line) == 2) :
                            client.delete(line[1])
                        else :
                            print("Syntax error. Usage: DELETE <fileName>")

                    elif(line[0]=="LIST_USERS") :
                        if (len(line) == 1) :
                            client.listusers()
                        else :
                            print("Syntax error. Use: LIST_USERS")

                    elif(line[0]=="LIST_CONTENT") :
                        if (len(line) == 2) :
                            client.listcontent(line[1])
                        else :
                            print("Syntax error. Usage: LIST_CONTENT <userName>")

                    elif(line[0]=="DISCONNECT") :
                        if (len(line) == 2) :
                            client.disconnect(line[1])
                        else :
                            print("Syntax error. Usage: DISCONNECT <userName>")

                    elif(line[0]=="GET_FILE") :
                        if (len(line) == 4) :
                            client.getfile(line[1], line[2], line[3])
                        else :
                            print("Syntax error. Usage: GET_FILE <userName> <remote_fileName> <local_fileName>")

                    elif(line[0]=="QUIT") :
                        if (len(line) == 1) :
                            break
                        else :
                            print("Syntax error. Use: QUIT")
                    else :
                        print("Error: command " + line[0] + " not valid.")
            except Exception as e:
                print("Exception: " + str(e))

    # *
    # * @brief Prints program usage
    @staticmethod
    def usage() :
        print("Usage: python3 client.py -s <server> -p <port>")


    # *
    # * @brief Parses program execution arguments
    @staticmethod
    def  parseArguments(argv) :
        parser = argparse.ArgumentParser()
        parser.add_argument('-s', type=str, required=True, help='Server IP')
        parser.add_argument('-p', type=int, required=True, help='Server Port')
        args = parser.parse_args()

        if (args.s is None):
            parser.error("Usage: python3 client.py -s <server> -p <port>")
            return False

        if ((args.p < 1024) or (args.p > 65535)):
            parser.error("Error: Port must be in the range 1024 <= port <= 65535");
            return False;
        
        client._server = args.s
        client._port = args.p

        return True


    # ******************** MAIN *********************
    @staticmethod
    def main(argv) :
        if (not client.parseArguments(argv)) :
            client.usage()
            return

        #  Write code here
        print(client._server,client._port)
        client.shell()
        print("+++ FINISHED +++")
    

if __name__=="__main__":
    client.main([])