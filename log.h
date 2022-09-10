#pragma once

#include "amqp_message.h"

void log_state(char *state_name, connection_state *cs);

void log_message_header(char sender, amqp_message_header header, connection_state *cs);

void log_method_header(char sender, amqp_method_header header, connection_state *cs);

void log_queue_creation(char *queue_name, connection_state *cs);
