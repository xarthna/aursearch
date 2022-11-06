#ifndef REQUEST_H
#define REQUEST_H

#include "types.h"
#include <curl/curl.h>
#include <json-c/arraylist.h>
#include <json-c/json_tokener.h>
#include <string.h>

CURL *prepare_request(char *search_param, struct MemoryStruct *chunk);
void make_request(CURL *curl);
struct array_list *parse_results(struct MemoryStruct *chunk);
size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb,
                           void *userp);

const char *get_string_prop(search_result result, const char *prop_name);

int get_json_str_len(search_result result, const char *prop_name);

#endif

