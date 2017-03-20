#!/usr/bin/env python
import threading
import os
import socket
from time import sleep

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP

def udp_server():

    while True:

        data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
        print "received message:", data
        sleep(1)

if __name__ == '__main__':

    UDP_IP = "10.0.0.6"
    UDP_PORT = 4210
    MESSAGE = "Hello, World!"

    print "UDP target IP:", UDP_IP
    print "UDP target port:", UDP_PORT
    print "message:", MESSAGE


    sock.bind(("0.0.0.0", UDP_PORT))
    udp_ser = threading.Thread(target=udp_server, args=())
    udp_ser.start()

    while True:
        sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))
        sleep(1)
