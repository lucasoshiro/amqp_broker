# EP1

Lucas Seiki Oshiro - 9298228

## Introdução

Este EP visa a implementação de um servidor de mensageria compatível com o
protocolo AMQP.

Foram implementados os funcionamentos básicos desse tipo de servidor:

- Declaração de fila
- Publicação em uma fila
- Consumo de uma fila

Vários recursos do AMQP não são suportados, como por exemplo:

- Multiplexação de canais (Channels): apenas 1 canal é suportado por conexão
- Exchanges
- Autenticação

## Requisitos, compilação e execução

### Requisitos:

- Sistema operacional Unix-like (testados: Manjaro Linux, Debian e Mac OS) com
  suporte a pthreads
- Compilador C (testados: GCC e CLANG)
- GNU Make
- Processador little-endian (testados: Intel x86 64bit, ARM 32bit e ARM 64bit)

### Requisitos do cliente:
- AMQP 0.9.1
- Sem autenticação
- Não enviar mensagens de Exchange
- Enviar ACK após consumo

Testados: amqp-tools e python-aqmp.

### Configuração

O arquivo config.h contém algumas constantes que configuram o código em tempo
de compilação. Por lá, é possível definir, por exemplo, o número máximo de
conexões e o tamanho máximo do nome das filas.

### Compilação

Para compilar, basta executar `make` na raiz do projeto.

### Execução

Uma vez compilado, basta executar `./ep1 <porta>` na raiz do projeto, passando
como argumento o número da porta onde serão escutadas as conexões.

### Conexão de cliente

O cliente pode se conectar das seguintes formas:
- `amqp-declare-queue -q <fila> --server <host> --port <porta>`: declaração de
uma fila
- `amqp-publish --server <host> --port <porta> -r <fila> -b <mensagem>`:
publicação de uma mensagem
- `amqp-consume --server <host> --port <porta> -q <fila> cat`: consumo de uma
fila

## Funcionamento

### Conexões

Cada conexão é executada em uma thread. Optei por usar threads em vez de
processos por ser mais simples o gerenciamento da memória compartilhada entre
elas, uma vez que há um estado compartilhado entre as conexões em memória (as
filas).

### Máquina de estados

Como o AMQP é um protocolo com estados, uma forma de manter o controle da
comunicação com o cliente é usando uma máquina de estados.

Cada máquina de estados é criada após o inicío da conexão com o cliente, e
inicia no estado WAIT, onde espera o cabeçalho do protocolo (Protocol Header).

O último estado da conexão é o FINISHED, em que a conexão é encerrada sem
problemas. Há ainda um estado FAIL, para qual qualquer outro estado pode ir caso
ocorra algum erro.

A maquina de estados é descrita em state_machine.c e state_machine.h.

#### Diagrama da máquina de estados

O estado FAIL é omitido aqui para simplificar o diagrama, já que qualquer
estado pode ir para ele caso encontre algum problema.

Os estados durante o estabelecimento da conexão são os seguintes:

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
         |-> Fluxo de declação de filas
         |
         |-> Fluxo de publicação
         |
          -> Fluxo de consumo
~~~

A partir do estado WAIT FUNCTIONAL, podemos ter 3 fluxos diferentes: o da
declaração de filas, o da publicação e o do consumo, ou ainda, iniciar o
fechamento de uma conexão.

Esses fluxos podem iniciar o encerramento da conexão dentro de seus estados,
o que também será omitido aqui para simplificar o diagrama.

O fluxo da declaração de filas é o seguinte:

~~~
*------------*                      *---------------*
|    WAIT    | -------------------> | QUEUE DECLARE |
| FUNCTIONAL | C: Queue Declare     |    RECEIVED   |
|            |                      |               |
|            | <------------------  |               |
|            | S: Queue Declare OK  |               |
*------------*                      *---------------*

~~~

O fluxo da publicação é o seguinte:

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

O fluxo do consumo é o seguinte:

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

Quanto ao encerramento, ele pode ser iniciado em alguns estados quando eles
recebem mensagens de encerramento de canal ou de conexão:

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

### Leitura e envio de mensagens

Como o AMQP não é um protocolo ASCII, e como o tamanho de cada campo na maior
parte das vezes bem definido na especificação, parsear as mensagens é
relativamente simples: normalmente, basta copiar todos os bytes (usando memcpy)
da mensagem direto para uma struct que contem os campos nos tamanhos certos, e
depois fazer as conversões do endianess. Essa struct passará a representar os
dados da mensagem para a tomada de decisões na máquina de estados.

Toda mensagem (exceto o Protocol Header, que inicia a conexão) tem um cabeçalho
e o corpo. O cabeçalho define o tipo de mensagem (ex: Method ou Body), o canal e
o tamanho do corpo. A partir dele, podemos ler a quantidade correta de bytes
restantes da mensagem, e da mesma forma, parsear os dados copiando os valores
para a struct usando memcpy.

Alguns campos têm tamanho variável (as short strings e long strings). Como uma
parte delas representa o tamanho e o resto, o conteúdo da string, elas podem
ser interpretadas como sendo arrays flexíveis.

Para o envio das mensagens, fazemos o caminho inverso: a partir das structs
contendo o header e o corpo da mensagem, converte-se o endianess e os bytes das
structs são enviados para os clientes

A implementação está em amqp_message.c e amqp_message.h

### Filas

As filas são implementadas usando listas encadeadas, sendo que cada nó também
funciona como array flexível, sendo que a parte fixa armazena o tamanho e o
próximo nó da lista e a parte flexível armazena o próximo nó da 

Cada fila também armazena seu nome, a quantidade de elementos, um scheduler para
o round robin, e uma variável condicional para sinalizar a chegada de novas
mensagens.

A implementação está em queue.c e queue.h.

### Pool de filas

O pool de filas mapeia cada nome de fila para uma fila. Como C nativamente não
têm tabelas de símbolos, a implementação do pool foi feita usando uma trie.
Tries são estruturas de dados em árvore que permitem a construção de tabelas de
símbolos cujas chaves são strings. Além disso, tries conseguem indexar
rapidamente uma string em O(n).

A implementação está em queue_pool.c e queue_pool.h.

### Round robin

Os valores de cada fila são entregues para cada consumidor inscrito usando um
escalonamento em round robin. Cada vez que uma thread espera um valor para ser
consumido, ele espera até ser a sua vez de consumir o valor, e espera até a fila
ter algum valor.

Toda vez que a fila recebe um valor, ela envia um sinal para a cada thread
consumidora, já que é possível que a thread esteja esperando que fila tenha
algum valor. Toda vez que uma thread consome um valor, ela envia um sinal
para a proxima thread, avisando que chegou a vez dela.

A implementação está em round_robin.c e round_robin.h.
