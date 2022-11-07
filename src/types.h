#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>

struct JsonString {
  char *memory;
  size_t size;
};

struct column_sizes {
  int name;
  int description;
  int version;
};

#endif
