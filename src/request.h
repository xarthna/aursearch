#ifndef REQUEST_H
#define REQUEST_H

#include "types.h"
#include <curl/curl.h>
#include <json-c/arraylist.h>

CURL *prepare_request(char *search_param, struct JsonString *chunk);

void make_request(CURL *curl);

struct array_list *parse_results(struct JsonString *chunk);

#endif
