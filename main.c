#include "util.h"
#include <curl/curl.h>
#include <json-c/arraylist.h>
#include <json-c/json_object.h>
#include <json-c/json_tokener.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESULT_NAME_PROP "Name"
#define RESULT_DESCRIPTION_PROP "Description"
#define RESULT_VERSION_PROP "Version"
#define RESULT_CLONE_PROP "Clone"
#define USER_AGENT "libcurl-agent/1.0"

// TODO: Wrap long output and fit to terminal size
// TODO: Sort param, Raw Line Based Output, Raw Json output params

typedef struct json_object *search_result;
const char *URL_FMT = "https://aur.archlinux.org/rpc/?v=5&type=search&arg=%s";
const size_t RESULT_FORMAT_SIZE = 30;
const size_t URL_FMT_SIZE = 80;
const size_t SEARCH_PARAM_SIZE = 50;
const size_t URL_SIZE = URL_FMT_SIZE + SEARCH_PARAM_SIZE;

CURL *prepare_request(char *url, struct MemoryStruct *chunk);
void make_request(CURL *curl);
void set_column_sizes(struct array_list *results, int *name_col, int *desc_col,
                      int *ver_col);
int get_json_str_len(search_result result, const char *prop_name);
void print_results(char *format, struct array_list *search_results);
void set_result_print_format(char *format, int name_column_size,
                             int description_column_size,
                             int version_column_size);

struct array_list *parse_results(struct MemoryStruct *chunk);
const char *get_string_prop(search_result result, const char *prop_name);
void print_header(char *format);

/* IMPL */

CURL *prepare_request(char *url, struct MemoryStruct *chunk) {
  CURL *curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)chunk);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
  return curl;
}

void make_request(CURL *curl) {
  CURLcode response = curl_easy_perform(curl);

  if (response != CURLE_OK) {
    fprintf(stderr, "error: %s\n", curl_easy_strerror(response));
    exit(1);
  }
}

int get_json_str_len(search_result result, const char *prop_name) {
  return json_object_get_string_len(json_object_object_get(result, prop_name));
}

void set_column_sizes(struct array_list *search_results, int *name_column_size,
                      int *description_column_size, int *version_column_size) {
  int name_length;
  int description_length;
  int version_length;
  search_result result;

  for (size_t i = 0; i < search_results->size; ++i) {
    result = json_object_get(search_results->array[i]);
    name_length = get_json_str_len(result, RESULT_NAME_PROP);
    description_length = get_json_str_len(result, RESULT_DESCRIPTION_PROP);
    version_length = get_json_str_len(result, RESULT_VERSION_PROP);

    if (name_length > *name_column_size)
      *name_column_size = name_length;

    if (description_length > *description_column_size)
      *description_column_size = description_length;

    if (version_length > *version_column_size)
      *version_column_size = version_length;
  }
}

struct array_list *parse_results(struct MemoryStruct *chunk) {
  struct json_object *json = json_tokener_parse(chunk->memory);
  struct json_object *results_ptr = json_object_object_get(json, "results");
  return json_object_get_array(results_ptr);
}

void set_result_print_format(char *format, int name_column_size,
                             int description_column_size,
                             int version_column_size) {
  snprintf(format, RESULT_FORMAT_SIZE, "%%-%ds\t%%-%ds\t%%-%ds\t%%-s\n",
           name_column_size, description_column_size, version_column_size);
}

const char *get_string_prop(search_result result, const char *prop_name) {
  return json_object_get_string(json_object_object_get(result, prop_name));
}

void print_header(char *format) {
  printf(format, RESULT_NAME_PROP, RESULT_DESCRIPTION_PROP, RESULT_VERSION_PROP,
         RESULT_CLONE_PROP);
}

void print_results(char *format, struct array_list *search_results) {
  const char *name;
  const char *desc;
  const char *version;
  search_result result;

  for (size_t i = 0; i < search_results->size; ++i) {
    char clone[100] = "https://aur.archlinux.org/";
    result = json_object_get(search_results->array[i]);
    name = get_string_prop(result, RESULT_NAME_PROP);
    desc = get_string_prop(result, RESULT_DESCRIPTION_PROP);
    version = get_string_prop(result, RESULT_VERSION_PROP);
    snprintf(&clone[26], strlen(name) + 1, "%s", name);
    printf(format, name, desc, version, clone);
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s <keyword-in-package-name-or-desc>\n", argv[0]);
    exit(1);
  }

  struct array_list *results;
  struct MemoryStruct chunk = {.memory = malloc(1), .size = 0};
  char result_print_format[RESULT_FORMAT_SIZE];
  char *search_param = argv[1];
  char url[URL_SIZE];
  int name_col_size = 0;
  int desc_col_size = 0;
  int ver_col_size = 0;
  snprintf(url, URL_SIZE, URL_FMT, search_param);
  CURL *curl = prepare_request(url, &chunk);

  if (curl) {
    make_request(curl);
    results = parse_results(&chunk);
    set_column_sizes(results, &name_col_size, &desc_col_size, &ver_col_size);
    set_result_print_format(result_print_format, name_col_size, desc_col_size,
                            ver_col_size);
    print_header(result_print_format);
    print_results(result_print_format, results);

    free(chunk.memory);
    curl_easy_cleanup(curl);
    return 0;
  }
}
