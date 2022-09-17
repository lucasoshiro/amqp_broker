#include "hardcoded_values.h"

const char CONNECTION_START_ARGS[] =
    "\x00\x09\x00\x00\x01\xd2\x0c\x63\x61\x70\x61\x62\x69\x6c\x69\x74"
    "\x69\x65\x73\x46\x00\x00\x00\xc7\x12\x70\x75\x62\x6c\x69\x73\x68"
    "\x65\x72\x5f\x63\x6f\x6e\x66\x69\x72\x6d\x73\x74\x01\x1a\x65\x78"
    "\x63\x68\x61\x6e\x67\x65\x5f\x65\x78\x63\x68\x61\x6e\x67\x65\x5f"
    "\x62\x69\x6e\x64\x69\x6e\x67\x73\x74\x01\x0a\x62\x61\x73\x69\x63"
    "\x2e\x6e\x61\x63\x6b\x74\x01\x16\x63\x6f\x6e\x73\x75\x6d\x65\x72"
    "\x5f\x63\x61\x6e\x63\x65\x6c\x5f\x6e\x6f\x74\x69\x66\x79\x74\x01"
    "\x12\x63\x6f\x6e\x6e\x65\x63\x74\x69\x6f\x6e\x2e\x62\x6c\x6f\x63"
    "\x6b\x65\x64\x74\x01\x13\x63\x6f\x6e\x73\x75\x6d\x65\x72\x5f\x70"
    "\x72\x69\x6f\x72\x69\x74\x69\x65\x73\x74\x01\x1c\x61\x75\x74\x68"
    "\x65\x6e\x74\x69\x63\x61\x74\x69\x6f\x6e\x5f\x66\x61\x69\x6c\x75"
    "\x72\x65\x5f\x63\x6c\x6f\x73\x65\x74\x01\x10\x70\x65\x72\x5f\x63"
    "\x6f\x6e\x73\x75\x6d\x65\x72\x5f\x71\x6f\x73\x74\x01\x0f\x64\x69"
    "\x72\x65\x63\x74\x5f\x72\x65\x70\x6c\x79\x5f\x74\x6f\x74\x01\x0c"
    "\x63\x6c\x75\x73\x74\x65\x72\x5f\x6e\x61\x6d\x65\x53\x00\x00\x00"
    "\x10\x72\x61\x62\x62\x69\x74\x40\x6d\x79\x2d\x72\x61\x62\x62\x69"
    "\x74\x09\x63\x6f\x70\x79\x72\x69\x67\x68\x74\x53\x00\x00\x00\x37"
    "\x43\x6f\x70\x79\x72\x69\x67\x68\x74\x20\x28\x63\x29\x20\x32\x30"
    "\x30\x37\x2d\x32\x30\x32\x32\x20\x56\x4d\x77\x61\x72\x65\x2c\x20"
    "\x49\x6e\x63\x2e\x20\x6f\x72\x20\x69\x74\x73\x20\x61\x66\x66\x69"
    "\x6c\x69\x61\x74\x65\x73\x2e\x0b\x69\x6e\x66\x6f\x72\x6d\x61\x74"
    "\x69\x6f\x6e\x53\x00\x00\x00\x39\x4c\x69\x63\x65\x6e\x73\x65\x64"
    "\x20\x75\x6e\x64\x65\x72\x20\x74\x68\x65\x20\x4d\x50\x4c\x20\x32"
    "\x2e\x30\x2e\x20\x57\x65\x62\x73\x69\x74\x65\x3a\x20\x68\x74\x74"
    "\x70\x73\x3a\x2f\x2f\x72\x61\x62\x62\x69\x74\x6d\x71\x2e\x63\x6f"
    "\x6d\x08\x70\x6c\x61\x74\x66\x6f\x72\x6d\x53\x00\x00\x00\x11\x45"
    "\x72\x6c\x61\x6e\x67\x2f\x4f\x54\x50\x20\x32\x35\x2e\x30\x2e\x34"
    "\x07\x70\x72\x6f\x64\x75\x63\x74\x53\x00\x00\x00\x08\x52\x61\x62"
    "\x62\x69\x74\x4d\x51\x07\x76\x65\x72\x73\x69\x6f\x6e\x53\x00\x00"
    "\x00\x06\x33\x2e\x31\x30\x2e\x37\x00\x00\x00\x0e\x41\x4d\x51\x50"
    "\x4c\x41\x49\x4e\x20\x50\x4c\x41\x49\x4e\x00\x00\x00\x05\x65\x6e"
    "\x5f\x55\x53";

const size_t CONNECTION_START_ARGS_SIZE = 499;

const char CONNECTION_TUNE_ARGS[] = "\x07\xff\x00\x02\x00\x00\x00\x3c";

const size_t CONNECTION_TUNE_ARGS_SIZE = 8;

const char CONNECTION_OPEN_OK_ARGS[] = "\x00";

const size_t CONNECTION_OPEN_OK_ARGS_SIZE = 1;

const char ARGS_EMPTY[] = "";

const size_t ARGS_EMPTY_SIZE = 0;

const char CHANNEL_OPEN_OK_ARGS[] = "\x00\x00\x00\x00";

const size_t CHANNEL_OPEN_OK_ARGS_SIZE = 4;

const char BASIC_CONSUME_OK_ARGS[] = "\x00\x00\x00\x00\x00\x00\x00\x01\x00";

const size_t BASIC_CONSUME_OK_ARGS_SIZE = 13;

const char BASIC_DELIVER_ARGS[] =
    "\x1f\x61\x6d\x71\x2e\x63\x74\x61\x67\x2d\x56\x64\x34\x59\x53\x35"
    "\x52\x49\x32\x34\x5f\x2d\x71\x48\x68\x61\x6e\x51\x4e\x51\x4a\x67"
    "\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x07\x63\x68\x65\x65\x74"
    "\x6f\x73";

const size_t BASIC_DELIVER_ARGS_SIZE = 50;

const char CONTENT_HEADER_PROPERTIES[] =  "\x01";
