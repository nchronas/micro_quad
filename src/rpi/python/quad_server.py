#!/usr/bin/env python
import threading
import os
import socket
from flask import Flask, render_template
from flask_socketio import SocketIO, emit
from time import sleep

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP

app = Flask(__name__)
app.config['SECRET_KEY'] = 'secret!'
socketio = SocketIO(app)

def tst_udp_server():
    while True:
        sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))
        sleep(1)

def udp_server():
    while True:
        data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
        print "received message:", data
        dict_out = {'roll': data}
        emit('my response', dict_out)


@app.route('/')
def index():
    '''View test index html.'''
    return render_template('../html/index.html')

@socketio.on('my event')
def test_message(message):
    emit('my response', {'data': 'got it!'})

if __name__ == '__main__':

    UDP_IP = "10.0.0.6"
    UDP_PORT = 4210
    MESSAGE = "Hello, World!"

    print "UDP target IP:", UDP_IP
    print "UDP target port:", UDP_PORT
    print "message:", MESSAGE


    sock.bind(("0.0.0.0", UDP_PORT))
    udp_ser = threading.Thread(target=udp_server, args=())
    udp_ser.daemon = True
    udp_ser.start()

    udp_tst_ser = threading.Thread(target=tst_udp_server, args=())
    udp_tst_ser.daemon = True
    udp_tst_ser.start()

    try:
        socketio.run(app, host='0.0.0.0', port=8080)
    except (KeyboardInterrupt, SystemExit):
        socketio.stop()