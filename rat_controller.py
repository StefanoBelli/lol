#!/usr/bin/python3

import select
import socket
import sys

local_addr = ('', 12345)

def stdout_controller_print(text):
    print("> {}".format(text))

def obtain_socket(address):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind(address)
    sock.listen(1)
    
    return sock

def controller_kind_of_shell(sock):
    while True:
        readsock, _, _ = select.select([sys.stdin, sock],[],[])
        for sck in readsock:
            if sck == sock:
                try:
                    text = sock.recv(4096).decode('ascii')

                    if len(text) is 0:
                        raise ConnectionResetError
                except ConnectionResetError:
                    return

                phrases = text.split('\n')
                for phrase in phrases:
                    if phrase != "":
                        sys.stdout.write("remote: {}\n".format(phrase))
            else:
                command = sys.stdin.readline().rstrip()

                if len(command) > 255:
                    stdout_controller_print("seems too long to me...")
                    break
                elif len(command) == 0:
                    stdout_controller_print("seems too short... :/")
                    break

                if command == "close":
                    return
                
                stdout_controller_print("got it: sent {} bytes...".format(sock.send(command.encode('ascii'))))

if __name__ == '__main__':
    lsock = None
    conn_sock = None

    stdout_controller_print("setting up...")

    try:
        lsock = obtain_socket(local_addr)
        stdout_controller_print("waiting for connection...")
        (conn_sock, conn_info) = lsock.accept()
        
        stdout_controller_print("connection accepted:")
        stdout_controller_print("\taddress: {}".format(conn_info[0]))
        stdout_controller_print("\tlocal port: {}".format(conn_info[1]))
        stdout_controller_print("type \"close\" to end connection correctly");
        stdout_controller_print("input max length is 256 including newline");
        
        controller_kind_of_shell(conn_sock)
    except KeyboardInterrupt:
        stdout_controller_print("KeyboardInterrupt");

    if conn_sock is not None: 
        conn_sock.close()

    if lsock is not None: 
        lsock.close()

    stdout_controller_print("bye")
    
