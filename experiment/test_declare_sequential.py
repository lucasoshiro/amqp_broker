#!/usr/bin/env python3
import amqp
from time import time
from itertools import permutations
import threading

HOST = 'localhost:5672'

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
