#!/usr/bin/env python
import eventlet
eventlet.monkey_patch()

import threading
import os
import socket
from flask import Flask, render_template
import socketio
from time import sleep
import pygame
import eventlet.wsgi

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP

async_mode = 'eventlet'

sio = socketio.Server(logger=True, async_mode=async_mode)
app = Flask(__name__)
app.debug = True
app.wsgi_app = socketio.Middleware(sio, app.wsgi_app)
app.config['SECRET_KEY'] = 'secret!'

pygame.init()
size = [1, 1]
screen = pygame.display.set_mode(size)

pygame.joystick.init()
controller = pygame.joystick.Joystick(0)
controller.init()

tx_flag = False

def joy_event_server():

    # Left hat
    AXIS_X = 0
    AXIS_Y = 1

    while True:
        pygame.event.get()
        rot_x = controller.get_axis( AXIS_X )
        rot_y = controller.get_axis( AXIS_Y )

        #controller.get_button( 1 ) # X
        dict_out = { 'joy_x' : rot_x, 'joy_y' : rot_y}

        print "Joystick: ", dict_out
        sio.emit('my response', {'data': dict_out}, room=sid, namespace='/test')
        eventlet.sleep(0.5)

def quad_response_parser(data):
    temp = data.split(',')
    if len(temp) < 2:
        dict_out = { }
    elif temp[0] == '$MQRPY':
        dict_out = { 'r' : temp[2], 'p' : temp[3],'y' : temp[4], }
    else:
        dict_out = { }
    return dict_out


def tst_udp_server():
    global tx_flag
    while True:
        if tx_flag:
            sock.sendto("$MQRPY*\n", (UDP_IP, UDP_PORT))
        eventlet.sleep(0.5)

def background_thread():
    """Example of how to send server generated events to clients."""
    count = 0
    while True:
        data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
        dict_out = quad_response_parser(data)
        count += 1
        print "received message:", data, dict_out
        sio.emit('my_response', {'data': dict_out}, namespace='/test')

@app.route('/')
def index():
    '''View test index html.'''
    return render_template('index.html')

@app.route('/cube')
def cube():
    '''View test index html.'''
    return render_template('cube.html')

@app.route('/joy')
def cube():
    '''View test index html.'''
    return render_template('joy.html')

@app.route('/canvasjs.min.js')
def plot():
    '''View test index html.'''
    return app.send_static_file('canvasjs.min.js')

@app.route('/socket.io.min.js')
def sockets():
    '''View test index html.'''
    return app.send_static_file('socket.io.min.js')

@app.route('/Three.js')
def three():
    '''View test index html.'''
    return app.send_static_file('Three.js')

@sio.on('connect', namespace='/test')
def test_connect(sid, environ):
    print "Socket Connected"
    sio.emit('my response', {'data': 'Connected', 'count': 0}, room=sid, namespace='/test')

@sio.on('disconnect', namespace='/test')
def test_disconnect(sid):
    print('Client disconnected')

@sio.on('my_event', namespace='/test')
def test_message(sid, message):
    global tx_flag
    print "Received socket msg", message
    if message['data'] == 'toggle_tx':
        if tx_flag:
            tx_flag = False
        else:
            tx_flag = True
        print "Toggling  UDP Tx to: ", tx_flag
    sio.emit('my_response', {'data': message['data']}, room=sid, namespace='/test')

if __name__ == '__main__':

    UDP_IP = "10.0.0.6"
    UDP_PORT = 4210
    MESSAGE = "Hello, World!"

    print "UDP target IP:", UDP_IP
    print "UDP target port:", UDP_PORT
    print "message:", MESSAGE

    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind(("0.0.0.0", UDP_PORT))

    udp_inc_ser = threading.Thread(target=background_thread, args=())
    udp_inc_ser.daemon = True
    udp_inc_ser.start()

    eventlet.spawn(tst_udp_server)

    #try:
    eventlet.wsgi.server(eventlet.listen(('', 8080)), app)
    #except (KeyboardInterrupt, SystemExit):
        #sio.stop()
