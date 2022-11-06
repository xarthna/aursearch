#include "util.h"
#include <curl/curl.h>
#include <json-c/arraylist.h>
#include <json-c/json_object.h>
#include <json-c/json_tokener.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: Wrap long output and fit to terminal size
// TODO: Sort param, Raw Line Based Output, Raw Json output params

const char *USER_AGENT = "libcurl-agent/1.0";
const char *URL_FMT = "https://aur.archlinux.org/rpc/?v=5&type=search&arg=%s";
const size_t URL_FMT_SIZE = 80;
const size_t SEARCH_PARAM_SIZE = 50;
const size_t URL_SIZE = URL_FMT_SIZE + SEARCH_PARAM_SIZE;

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s <keyword-in-package-name-or-desc>\n", argv[0]);
    exit(1);
  }

  CURLcode res;

  char *search_param = argv[1];
  char url[URL_SIZE];

  CURL *curl = curl_easy_init();

  struct MemoryStruct chunk = {.memory = malloc(1), .size = 0};

  snprintf(url, URL_SIZE, URL_FMT, search_param);

  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
      fprintf(stderr, "error: %s\n", curl_easy_strerror(res));
      exit(1);
    }

    struct json_object *obj = json_tokener_parse(chunk.memory);
    struct json_object *result_count =
        json_object_object_get(obj, "resultcount");
    struct json_object *result_ptr = json_object_object_get(obj, "results");
    int32_t count = json_object_get_int(result_count);
    struct array_list *results = json_object_get_array(result_ptr);

    printf("Total Results: %d\n", count);

    struct json_object *o;
    const char *name, *desc, *version;

    int max_name = 0;
    int max_desc = 0;
    int max_version = 0;

    for (size_t i = 0; i < results->size; ++i) {
      o = json_object_get(results->array[i]);
      if (json_object_get_string_len(json_object_object_get(o, "Name")) >
          max_name)
        max_name =
            json_object_get_string_len(json_object_object_get(o, "Name"));

      if (json_object_get_string_len(json_object_object_get(o, "Description")) >
          max_desc)
        max_desc = json_object_get_string_len(
            json_object_object_get(o, "Description"));

      if (json_object_get_string_len(json_object_object_get(o, "Version")) >
          max_version)
        max_version =
            json_object_get_string_len(json_object_object_get(o, "Version"));
    }

    char fmt[27];
    snprintf(fmt, 27, "%%-%ds\t%%-%ds\t%%-%ds\t%%-s\n", max_name, max_desc,
             max_version);

    printf(fmt, "Name", "Description", "Version", "Clone");
    for (size_t i = 0; i < results->size; ++i) {
      char str[100];
      strcpy(str, "https://aur.archlinux.org/");
      o = json_object_get(results->array[i]);
      name = json_object_get_string(json_object_object_get(o, "Name"));
      desc = json_object_get_string(json_object_object_get(o, "Description"));
      version = json_object_get_string(json_object_object_get(o, "Version"));

      // TODO: snprintf
      printf(fmt, name, desc, version, strcat(str, name));
    }

    free(chunk.memory);
    curl_easy_cleanup(curl);
    return 0;
  }
}
