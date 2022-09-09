#!/usr/bin/env python3
import amqp
from time import time
from itertools import permutations
import threading

HOST = 'localhost:5673'

def declare_many_thread(s):
    with amqp.Connection(HOST) as c:
        ch = c.channel()
        # ch.queue_declare(''.join(s))

a = time()
# with amqp.Connection(HOST) as c:
threads = [
    threading.Thread(target=lambda: declare_many_thread(s))
    for s in permutations('AB')
]
    
for thread in threads: thread.start()
for thread in threads: thread.join()

b = time()

print(b - a)
