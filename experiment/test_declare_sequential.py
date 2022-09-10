#!/usr/bin/env python3
import amqp
from time import time
from itertools import permutations
import threading

HOST = 'localhost:5672'

a = time()
with amqp.Connection(HOST) as c:
    ch = c.channel()
    for s in permutations('ABCDEFGH'):
        ch.queue_declare(''.join(s))
b = time()

print(b - a)