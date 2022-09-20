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

def publish_many_thread():
    while True:
        try:
            conn = amqp.Connection(HOST, ssl=False)
            conn.connect()
            ch = conn.channel()
            break
        except:
            pass
    for s in permutations('ABCDEFG'):
        ch.basic_publish(msg=amqp.Message(''.join(s)), routing_key='queue')
    ch.close()
    conn.close()

a = time()
threads = [
    threading.Thread(target=publish_many_thread)
    for _ in range(25)
]
    
for thread in threads: thread.start()
for thread in threads: thread.join()
b = time()

print(b - a)
