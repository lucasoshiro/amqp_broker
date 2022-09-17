/* Hardcoded values

   These values are used in responses. They can always be sent without being
   changed, so there's no need to generate them everytime.
 */

#pragma once

#include <stdlib.h>

extern const char CONNECTION_START_ARGS[];
extern const size_t CONNECTION_START_ARGS_SIZE;

extern const char CONNECTION_TUNE_ARGS[];
extern const size_t CONNECTION_TUNE_ARGS_SIZE;

extern const char CONNECTION_OPEN_OK_ARGS[];
extern const size_t CONNECTION_OPEN_OK_ARGS_SIZE;

extern const char ARGS_EMPTY[];
extern const size_t ARGS_EMPTY_SIZE;

extern const char CHANNEL_OPEN_OK_ARGS[];
extern const size_t CHANNEL_OPEN_OK_ARGS_SIZE;

extern const char BASIC_CONSUME_OK_ARGS[];
extern const size_t BASIC_CONSUME_OK_ARGS_SIZE;

extern const char BASIC_DELIVER_ARGS[];
extern const size_t BASIC_DELIVER_ARGS_SIZE;

extern const char CONTENT_HEADER_PROPERTIES[];
