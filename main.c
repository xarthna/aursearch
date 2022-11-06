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
#define ENDPOINT_FORMAT_STR                                                    \
  "https://aur.archlinux.org/rpc/?v=5&type=search&arg=%s"
#define RESULT_FORMAT_STR_SIZE 30
#define ENDPOINT_FORMAT_STR_SIZE 80
#define SEARCH_PARAM_SIZE 50
#define TOTAL_ENDPOINT_FORMAT_STR_SIZE                                         \
  ENDPOINT_FORMAT_STR_SIZE + SEARCH_PARAM_SIZE

/*
 * TODO: Wrap long output and fit to terminal size
 *       Sort param, Raw Line Based Output, Raw Json output params
 */

typedef struct json_object *search_result;
struct column_sizes {
  int name;
  int description;
  int version;
};

char result_format_str[RESULT_FORMAT_STR_SIZE];

/* Proto */
CURL *prepare_request(char *search_param, struct MemoryStruct *chunk);
void make_request(CURL *curl);
void set_column_sizes(struct array_list *results, struct column_sizes *cols);
int get_json_str_len(search_result result, const char *prop_name);
void print_results(struct array_list *search_results);
void set_result_print_format(struct column_sizes *cols);
struct array_list *parse_results(struct MemoryStruct *chunk);
const char *get_string_prop(search_result result, const char *prop_name);
void cleanup(struct MemoryStruct *chunk, CURL *curl);
void print_header(void);

/* IMPL */

CURL *prepare_request(char *search_param, struct MemoryStruct *chunk) {
  char aur_endpoint[TOTAL_ENDPOINT_FORMAT_STR_SIZE];
  snprintf(aur_endpoint, TOTAL_ENDPOINT_FORMAT_STR_SIZE, ENDPOINT_FORMAT_STR,
           search_param);

  CURL *curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_URL, aur_endpoint);
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

struct array_list *parse_results(struct MemoryStruct *chunk) {
  struct json_object *json = json_tokener_parse(chunk->memory);
  struct json_object *results_ptr = json_object_object_get(json, "results");
  return json_object_get_array(results_ptr);
}

const char *get_string_prop(search_result result, const char *prop_name) {
  return json_object_get_string(json_object_object_get(result, prop_name));
}

void set_column_sizes(struct array_list *search_results,
                      struct column_sizes *cols) {
  int name_length;
  int description_length;
  int version_length;
  search_result result;

  for (size_t i = 0; i < search_results->size; ++i) {
    result = json_object_get(search_results->array[i]);
    name_length = get_json_str_len(result, RESULT_NAME_PROP);
    description_length = get_json_str_len(result, RESULT_DESCRIPTION_PROP);
    version_length = get_json_str_len(result, RESULT_VERSION_PROP);

    if (name_length > cols->name)
      cols->name = name_length;

    if (description_length > cols->description)
      cols->description = description_length;

    if (version_length > cols->version)
      cols->version = version_length;
  }
}


void set_result_print_format(struct column_sizes *cols) {
  snprintf(result_format_str, RESULT_FORMAT_STR_SIZE,
           "%%-%ds\t%%-%ds\t%%-%ds\t%%-s\n", cols->name, cols->description,
           cols->version);
}

void print_header() {
  printf(result_format_str, RESULT_NAME_PROP, RESULT_DESCRIPTION_PROP,
         RESULT_VERSION_PROP, RESULT_CLONE_PROP);
}

void print_results(struct array_list *search_results) {
  char clone[100] = "https://aur.archlinux.org/";
  const char *name;
  const char *desc;
  const char *version;
  search_result result;

  for (size_t i = 0; i < search_results->size; ++i) {
    result = json_object_get(search_results->array[i]);
    name = get_string_prop(result, RESULT_NAME_PROP);
    desc = get_string_prop(result, RESULT_DESCRIPTION_PROP);
    version = get_string_prop(result, RESULT_VERSION_PROP);
    snprintf(&clone[26], strlen(name) + 1, "%s", name);
    printf(result_format_str, name, desc, version, clone);
  }
}

void cleanup(struct MemoryStruct *chunk, CURL *curl) {
  free(chunk->memory);
  curl_easy_cleanup(curl);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s <keyword-in-package-name-or-desc>\n", argv[0]);
    exit(1);
  }

  struct array_list *results;
  struct column_sizes cols = {.name = 0, .description = 0, .version = 0};
  struct MemoryStruct chunk = {.memory = malloc(1), .size = 0};
  CURL *curl = prepare_request(argv[1], &chunk);

  if (curl) {
    make_request(curl);
    results = parse_results(&chunk);
    set_column_sizes(results, &cols);
    set_result_print_format(&cols);
    print_header();
    print_results(results);
    cleanup(&chunk, curl);
    return 0;
  }
}
