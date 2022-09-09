/* AMQP messages

This file defines the data structures that hold incoming data from the client
and outcoming data from this server. This also defines the functions that read
and write those data from/to the client.

*/
#pragma once

#include <stdint.h>
#include <string.h>

#include "connection_state.h"

/* AMQP types */
typedef uint16_t class_id;
typedef uint16_t method_id;

/* Short string */
typedef struct {
    uint8_t size;
    char str[1];
} __attribute__((packed)) amqp_short_string;

/* Long string */
typedef struct {
    uint32_t size;
    char str[1];
} __attribute__((packed)) amqp_long_string;

/* AMQP constants */

/* Message type. */
enum msg_format {
    METHOD = 1,
    CONTENT_HEADER,
    BODY,
    HEARTBEAT
};

/* Method classes id. */
enum class {
    CONNECTION = 10,
    CHANNEL    = 20,
    EXCHANGE   = 40,
    QUEUE      = 50,
    BASIC      = 60,
    TX         = 80
};

/* Methods id. When two or more of them share the same number, their classes
   differ. */
enum method {
    /* Connection methods */
    CONNECTION_START = 10,
    CONNECTION_START_OK,

    CONNECTION_TUNE = 30,
    CONNECTION_TUNE_OK,

    CONNECTION_OPEN = 40,
    CONNECTION_OPEN_OK,

    CONNECTION_CLOSE = 50,
    CONNECTION_CLOSE_OK,

    /* Channel methods. */
    CHANNEL_OPEN = 10,
    CHANNEL_OPEN_OK,

    CHANNEL_CLOSE = 40,
    CHANNEL_CLOSE_OK,

    /* Queue methods. */
    QUEUE_DECLARE = 10,
    QUEUE_DECLARE_OK,

    /* Basic methods */
    BASIC_CONSUME = 20,
    BASIC_CONSUME_OK,
    BASIC_PUBLISH = 40,
    BASIC_DELIVER = 60,
    BASIC_ACK     = 80
};

/* AMQP messsages data structures. */

/* Protocol header. The first amqp message that the client sends to the server.
 */
typedef struct {
    char amqp[4];
    uint8_t id_major;
    uint8_t id_minor;
    uint8_t version_major;
    uint8_t version_minor;
} __attribute__((packed)) amqp_protocol_header;

/* Message header. The header of an amqp message (except protocol header). */
typedef struct {
    uint8_t msg_type;
    uint16_t channel;
    uint32_t length;
} __attribute__((packed)) amqp_message_header;

/* Method header. This holds the class id and method id.*/
typedef struct {
    class_id class;
    method_id method;
} __attribute__((packed)) amqp_method_header;

/* Method. The method header and its arguments. */
typedef struct {
    amqp_method_header header;
    char arguments[1];
} __attribute__((packed)) amqp_method;

/* The header of a content header. This may sound funny, but a "content header"
   is a type of amqp message, and it has a header, which contains its class,
   weight, size of the body that will be sent/received after it and some flags.
*/
typedef struct {
    uint16_t class;
    uint16_t weight;
    uint64_t body_size;
    uint16_t property_flags;
} __attribute__((packed)) amqp_content_header_header;

/* Content header. The header of the content header and its body. */
typedef struct {
    amqp_content_header_header header;
    char property_list[1];
} __attribute__((packed)) amqp_content_header;

/* Fill queue_name with the queue name in queue declare args. Other args
 are ignored. */
void parse_queue_declare_args(void *args, char *queue_name);

/* Fill queue_name with the queue name in queue declare args. Other args
 are ignored. */
void parse_basic_publish_args(void *args, char *queue_name);

/* Fill queue_name with the queue name in queue declare args. Other args
 are ignored. */
void parse_basic_consume_args(void *args, char *queue_name);

/* Read an amqp protocol header. This is the first thing that the client sends
   after stablishing a connection with an amqp server. Fill the
   amqp_protocol_header pointed by the argument header with is content. */
int read_protocol_header(connection_state *cs, amqp_protocol_header *header);

/* Read a header of an amqp message, then fill the amqp_message_header pointed
   by the argument header with its content. */
int read_message_header(connection_state *cs, amqp_message_header *header);

/* Read a "method" amqp message, malloc an amqp_method, copy the data from
   the method to the amqp_method, then return a pointer to it.

   The length of the message (except its header) is required.
*/
amqp_method *read_method(connection_state *cs, int length);

/* Send a "method" amqp message, given its class id, method id, channel and
   arguments.

   The arguments can be an any format, since its binary format follow the amqp
   table structure.
*/
void send_method(
    connection_state *cs,
    class_id class,
    method_id method,
    uint16_t channel,
    void *arguments,
    size_t args_size
    );

void send_queue_declare_ok(
    connection_state *cs,
    uint16_t channel,
    char *queue_name,
    uint32_t message_count,
    uint32_t consumer_count
    );

/* Read a "content header" message, malloc an amqp_content_header, copy the data
   from the content header to the amqp_content_header, then return a pointer to
   it.

   The length of the message (except its header) is required.
*/
amqp_content_header *read_content_header(connection_state *cs, int length);

/* Send a "content header" amqp message, given its class id, channel, weight,
   body size, flags and properties */
void send_content_header(
    connection_state *cs,
    uint16_t class,
    uint16_t channel,
    uint16_t weight,
    uint64_t body_size,
    uint16_t flags,
    char *properties
    );

/* Read a "body" amqp message, malloc a string, copy body payload to the string,
   then return a pointer to the string.

   The length of the body is required.
*/
char *read_body(connection_state *cs, int length);

/* Send a payload of size n as a "body" amqp message. */
void send_body(
    connection_state *cs,
    int channel,
    char *payload,
    size_t n
    );

