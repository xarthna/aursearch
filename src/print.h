#ifndef PRINT_H
#define PRINT_H

#include "types.h"
#include <json-c/arraylist.h>

void set_column_sizes(struct array_list *results, struct column_sizes *cols);

void print_results(struct array_list *search_results);

void set_result_print_format(struct column_sizes *cols);

void print_header(void);

#endif
