# amqp broker (unnamed so far)

This was part of the subject [MAC5910](https://uspdigital.usp.br/janus/Disciplina?sgldis=MAC5910) (Network Programming) at IME-USP.

## Requirements, building and execution

### Requirements

- Unix-like operating system (tested: Manjaro Linux, Debian and Mac OS) with ``pthread`` support
- C compiler (tested: ``gcc`` and ``clang``)
- GNU Make
- Little-endian processor (tested: Intel x86 64bit, ARM 32bit and ARM 64bit)

### Client requirements:
- AMQP 0.9.1
- No authentication
- Do not send exchange messages
- Send ACK after consume

Tested: amqp-tools and python-aqmp.

### Configuration file

The file ``config.h`` contains some configuration constants, which set up the
code in compilation time. In that file, you can set, for example, the maximum
number of connections and the maximum length of the queue names.

### Building

Just run `make` on the project root.

### Execution

Once built, just run `./amqp_broker <port>` on the project root.

## Detailed design

### State machine

AMQP is a stateful protocol, so it's natural that we keep the connection control
using a state machine.

Each state machine is created after the beginning of the connection with the
client, starting in the state WAIT. In that state, it waits for the protocol
header.

The last state of the connection is FINISHED, when the connection finishes
gracefully. There is also a FAIL state, if something goes wrong.

The state machine is described in ``state_machine.c`` e ``state_machine.h``.

#### State machine diagram

The FAIL stated is hidden here in order to simplify the diagram, as any state
may have a event that leads to it.

The connection establishment states are the following:

~~~
        *--------*                      *------------*           *------------*
        |  WAIT  | -------------------> |   HEADER   | --------> |    WAIT    |
        |        | C: Protocol Header   |  RECEIVED  | S: Start  |  START OK  |
        *--------*                      *------------*           *------------*
                                                                          |
                                                              C: Start OK |
                                                                          |
*-----------------*            *-----------*            *------------*    |
|       WAIT      |<---------- |   WAIT    | <--------  |  START OK  | <--
| OPEN CONNECTION | C: Tune Ok |  TUNE OK  |  S: Tune   |  RECEIVED  |
*-----------------*            *-----------*            *------------*
  |
  | C: Open Connection
  |
  |    *-----------------*                         *-----------------*
   --> | OPEN CONNECTION | ----------------------> | OPEN CONNECTION |
       |     RECEIVED    | S: Open Connection OK   |     RECEIVED    |
       *-----------------*                         *-----------------*
                                                                   |
                                                  C: Open Channel  |
                                                                   |
       *-------------*                        *--------------*     |
       |     WAIT    | <--------------------- | OPEN CHANNEL | <---
       |  FUNCTIONAL |  S: Open Channel OK    |   RECEIVED   |
       *-------------*                        *--------------*
         |
         | C: Método
         |
         |-> Queue declare states
         |
         |-> Publish states
         |
          -> Consume states
~~~

After the state WAIT FUNCTIONAL, it can close the connection, or take one of
these different paths:
1. queue declare
2. publish
3. consume

Each one of them can begin the connection closing in its states. This will also
be hidden here in order to simplify the diagram.

The queue declaration states are the following:

~~~
*------------*                      *---------------*
|    WAIT    | -------------------> | QUEUE DECLARE |
| FUNCTIONAL | C: Queue Declare     |    RECEIVED   |
|            |                      |               |
|            | <------------------  |               |
|            | S: Queue Declare OK  |               |
*------------*                      *---------------*

~~~

The publish states are the following:

~~~
*------------*                      *---------------*    *----------------*
|    WAIT    | -------------------> | BASIC PUBLISH | -> |  WAIT PUBLISH  |
| FUNCTIONAL | C: Basic Publish     |    RECEIVED   |    | CONTENT HEADER |
*------------*                      *---------------*    *----------------*
                                          ^                              |
                         C: Basic Publish |            C: Content Header |
                                          |                              |
                                          |         *----------------*   |
                                          |-------- |  WAIT PUBLISH  | <-
                                          |         |     CONTENT    |
                                           -------> |                |
                                           C: Body  |                |
                                                    *----------------*
~~~

The consume states are the following:

~~~
*------------*                   *---------------*
|    WAIT    | ----------------> | BASIC CONSUME |
| FUNCTIONAL | C: Basic Consume  |    RECEIVED   |
*------------*                   *---------------*
                                         |
                                         | S: Basic Consume OK   *------------*
                                          ---------------------> | WAIT VALUE |
                      -----------------------------------------> |  DEQUEUE   |
                     |   C: Consume Ack                          *------------*
                     |                                                  |
                     |                                                  |
                     |                                       Q: Dequeue |
                     |                                                  |
             *--------------*                      *---------------*    |
             | WAIT CONSUME | <------------------- | VALUE DEQUEUE | <--
             |     ACK      |   S: Basic Deliver   |   RECEIVED    |
             |              |   S: Content Header  |               |
             |              |   S: Body            |               |
             *--------------*                      *---------------*
~~~

The connection close can be started on several states when they receive close
channel or close connection messages:

~~~
                  *---------------*                      *-----------*
----------------> | CLOSE CHANNEL | -------------------> | WAIT OPEN | 
 C: Close Channel |   RECEIVED    | S: Close Channel OK  |  CHANNEL  | 
                  *---------------*                      *-----------*

                     *------------------*                         *----------*
-------------------> | CLOSE CONNECTION | ----------------------> | FINISHED | 
 C: Close Connection |     RECEIVED     | S: Close Connection OK  |          | 
                     *------------------*                         *--------- *
~~~
