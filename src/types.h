#ifndef TYPES_H
#define TYPES_H

#include <stdlib.h>

#define RESULT_NAME_PROP "Name"
#define RESULT_DESCRIPTION_PROP "Description"
#define RESULT_VERSION_PROP "Version"
#define RESULT_CLONE_PROP "Clone"
#define USER_AGENT "libcurl-agent/1.0"
#define ENDPOINT_FORMAT_STR                                                    \
  "https://aur.archlinux.org/rpc/?v=5&type=search&arg=%s"
#define RESULT_FORMAT_STR_SIZE 30
#define ENDPOINT_FORMAT_STR_SIZE 80
#define SEARCH_PARAM_SIZE 50
#define TOTAL_ENDPOINT_FORMAT_STR_SIZE                                         \
  ENDPOINT_FORMAT_STR_SIZE + SEARCH_PARAM_SIZE

typedef struct json_object *search_result;

struct MemoryStruct {
  char *memory;
  size_t size;
};

struct column_sizes {
  int name;
  int description;
  int version;
};

#endif
