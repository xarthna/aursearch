#include "util.h"
#include <curl/curl.h>
#include <json-c/arraylist.h>
#include <json-c/json_object.h>
#include <json-c/json_tokener.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct json_object *search_result;

// TODO: Wrap long output and fit to terminal size
// TODO: Sort param, Raw Line Based Output, Raw Json output params

const char *USER_AGENT = "libcurl-agent/1.0";
const char *URL_FMT = "https://aur.archlinux.org/rpc/?v=5&type=search&arg=%s";
const size_t URL_FMT_SIZE = 80;
const size_t SEARCH_PARAM_SIZE = 50;
const size_t URL_SIZE = URL_FMT_SIZE + SEARCH_PARAM_SIZE;

CURL *prepare_request(char *url, struct MemoryStruct *chunk);
void make_request(CURL *curl);
void set_column_sizes(struct array_list *results, int *name_col, int *desc_col,
                      int *ver_col);
int get_json_str_len(search_result result, const char *prop_name);
void print_results(struct array_list *search_results, int name_column_size,
                   int description_column_size, int version_column_size);

struct array_list *parse_results(struct MemoryStruct *chunk);

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
    name_length = get_json_str_len(result, "Name");
    description_length = get_json_str_len(result, "Description");
    version_length = get_json_str_len(result, "Version");

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

void print_results(struct array_list *search_results, int name_column_size,
                   int description_column_size, int version_column_size) {
  const char *name;
  const char *desc;
  const char *version;
  char fmt[27];
  snprintf(fmt, 27, "%%-%ds\t%%-%ds\t%%-%ds\t%%-s\n", name_column_size,
           description_column_size, version_column_size);

  printf(fmt, "Name", "Description", "Version", "Clone");
  struct json_object *o;
  for (size_t i = 0; i < search_results->size; ++i) {
    char str[100];
    strcpy(str, "https://aur.archlinux.org/");
    o = json_object_get(search_results->array[i]);
    name = json_object_get_string(json_object_object_get(o, "Name"));
    desc = json_object_get_string(json_object_object_get(o, "Description"));
    version = json_object_get_string(json_object_object_get(o, "Version"));

    // TODO: snprintf
    printf(fmt, name, desc, version, strcat(str, name));
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s <keyword-in-package-name-or-desc>\n", argv[0]);
    exit(1);
  }

  struct MemoryStruct chunk = {.memory = malloc(1), .size = 0};
  char *search_param = argv[1];
  char url[URL_SIZE];
  snprintf(url, URL_SIZE, URL_FMT, search_param);

  CURL *curl = prepare_request(url, &chunk);
  if (curl) {
    int name_col_size = 0;
    int desc_col_size = 0;
    int ver_col_size = 0;

    make_request(curl);
    struct array_list *results = parse_results(&chunk);
    set_column_sizes(results, &name_col_size, &desc_col_size, &ver_col_size);
    print_results(results, name_col_size, desc_col_size, ver_col_size);
    free(chunk.memory);
    curl_easy_cleanup(curl);
    return 0;
  }
}
