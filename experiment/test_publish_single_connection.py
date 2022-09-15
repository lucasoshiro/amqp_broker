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
with amqp.Connection(HOST) as c:
    ch = c.channel()
    for s in permutations('ABCDEFGHI'):
        ch.basic_publish(msg=amqp.Message(''.join(s)), routing_key='queue')
    ch.close()
b = time()

print(b - a)
