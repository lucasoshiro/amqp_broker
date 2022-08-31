#pragma once

#include "amqp_message.h"

void log_state(char *state_name);
void log_message_header(char sender, amqp_message_header header);
void log_method_header(char sender, amqp_method_header header);
