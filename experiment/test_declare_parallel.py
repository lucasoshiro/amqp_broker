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

def declare_many_thread(s):
    while True:
        try:
            conn = amqp.Connection(HOST, ssl=False)
            conn.connect()
            ch = conn.channel()
            break
        except:
            pass
    ch.queue_declare(''.join(s))
    conn.close()

a = time()
threads = [
    threading.Thread(target=declare_many_thread, args=(s,))
    for s in permutations('ABCDEFGH')
]
    
for thread in threads: thread.start()
for thread in threads: thread.join()

b = time()

print(b - a)
