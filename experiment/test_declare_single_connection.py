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
with amqp.Connection(HOST) as c:
    ch = c.channel()
    for s in permutations('ABCDEFGH'):
        ch.queue_declare(''.join(s))
b = time()

print(b - a)
