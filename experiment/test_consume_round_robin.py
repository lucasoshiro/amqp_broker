#!/usr/bin/env python3
import amqp
from time import time
from itertools import permutations
import threading
from sys import argv

argv = dict(enumerate(argv))

IP = argv.get(1) or 'localhost'
PORT = argv.get(2) or '5672'
HOST = ':'.join([IP, PORT])

with amqp.Connection(HOST) as c:
    ch = c.channel()
    ch.queue_declare('queue')
    ch.close()
    c.close()

print('queue ok')

with amqp.Connection(HOST) as c:
    ch = c.channel()
    ch.queue_declare('queue')
    for _ in range(1000000):
        for letter in 'ABC':
            ch.basic_publish(msg=amqp.Message(letter), routing_key='queue');
    ch.close()
    c.close()

print('publish ok')

