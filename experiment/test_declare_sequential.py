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

a = time()
for s in permutations('ABCDEFG'):
    while True:
        try:
            c = amqp.Connection(HOST)
            c.connect()
            break
        except:
            pass
    
    ch = c.channel()
    ch.queue_declare(''.join(s))
    c.close()
b = time()

print(b - a)
