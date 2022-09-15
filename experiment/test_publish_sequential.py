#!/usr/bin/env python3
import amqp
from time import time
from itertools import permutations
from sys import argv

argv = dict(enumerate(argv))

IP = argv.get(1) or 'localhost'
PORT = argv.get(2) or '5672'
HOST = ':'.join([IP, PORT])

with amqp.Connection(HOST) as c:
    ch = c.channel()
    ch.queue_declare('queue')
    ch.close()

a = time()
for s in permutations('ABCDEFGHI'):
    while True:
        try:
            c = amqp.Connection(HOST)
            c.connect()
            break
        except:
            pass
    ch = c.channel()
    ch.basic_publish(msg=amqp.Message(''.join(s)), routing_key='queue')
    ch.close()
    c.close()
b = time()

print(b - a)
