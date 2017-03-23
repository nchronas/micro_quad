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
tx_rate = 0.1

tx_page = ''

def joy_event_server():
    global tx_page

    # Left stick
    AXIS_A = 1

    #right stick
    AXIS_X = 2
    AXIS_Y = 3

    BUT_ST = 4

    while True:
        pygame.event.get()
        mq_a = controller.get_axis( AXIS_A )
        mq_x = controller.get_axis( AXIS_X )
        mq_y = controller.get_axis( AXIS_Y )

        mq_s = controller.get_button( BUT_ST ) # X

        dict_out = { 'joy_a' : mq_a,
                     'joy_x' : mq_x, 
                     'joy_y' : mq_y, 
                     'joy_s' : mq_s }

        #normalize()

        print "Joystick: ", dict_out
        sock.sendto("$MQJOY," + str(mq_a) + ","
                              + str(mq_x) + ","
                              + str(mq_y) + ","
                              + str(mq_s) + ",*\n", (UDP_IP, UDP_PORT))
        if tx_page == 'joy_page':
            sio.emit('my_response', {'data': dict_out}, namespace='/test')
        eventlet.sleep(0.1)

def quad_response_parser(data):
    temp = data.split(',')
    if len(temp) < 2:
        dict_out = { }
    elif temp[0] == '$MQRPY':
        dict_out = { 'r' : temp[2], 'p' : temp[3],'y' : temp[4], }
    else:
        dict_out = { }
    return dict_out


def tx_udp_server():
    global tx_flag, tx_rate
    while True:
        if tx_flag:
            sock.sendto("$MQRPY*\n", (UDP_IP, UDP_PORT))
        eventlet.sleep(tx_rate)

def rec_udp_server():
    """Example of how to send server generated events to clients."""
    global tx_page
    count = 0
    while True:
        data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
        dict_out = quad_response_parser(data)
        count += 1
        print "received message:", data, dict_out
        if tx_page == 'plot_page':
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
def joy():
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
    sio.emit('my_response', {'data': 'Connected', 'count': 0}, room=sid, namespace='/test')

@sio.on('disconnect', namespace='/test')
def test_disconnect(sid):
    print('Client disconnected')

@sio.on('my_event', namespace='/test')
def test_message(sid, message):
    global tx_flag, tx_rate, tx_page
    print "Received socket msg", message
    if message['data'] == 'toggle_tx':
        if tx_flag:
            tx_flag = False
        else:
            tx_flag = True
        print "Toggling  UDP Tx to: ", tx_flag
    elif message['data'] == 'rate_tx':
         tx_rate = message['rate']
    elif message['data'] == 'plot_page':
        tx_page = 'plot_page'
    elif message['data'] == 'joy_page':
        tx_page = 'joy_page'
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

    udp_inc_ser = threading.Thread(target=rec_udp_server, args=())
    udp_inc_ser.daemon = True
    udp_inc_ser.start()

    eventlet.spawn(tx_udp_server)
    eventlet.spawn(joy_event_server)

    #try:
    eventlet.wsgi.server(eventlet.listen(('', 8080)), app)
    #except (KeyboardInterrupt, SystemExit):
        #sio.stop()
