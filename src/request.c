#include "request.h"
#include <json-c/json_tokener.h>
#include <stdlib.h>
#include <string.h>

#define USER_AGENT "libcurl-agent/1.0"
#define ENDPOINT_FORMAT_STR "https://aur.archlinux.org/rpc/?v=5&type=search&arg=%s"
#define ENDPOINT_FORMAT_STR_SIZE 130

static size_t json_data_callback(void *contents, size_t size, size_t nmemb, void *userp);

static size_t json_data_callback(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t realsize = size * nmemb;
  struct JsonString *mem = (struct JsonString *)userp;

  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if (ptr == NULL) {
    printf("error: not enough memory\n");
    return 0;
  }

  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

CURL *prepare_request(char *search_param, struct JsonString *mem) {
  char aur_endpoint[ENDPOINT_FORMAT_STR_SIZE];
  snprintf(aur_endpoint, ENDPOINT_FORMAT_STR_SIZE, ENDPOINT_FORMAT_STR, search_param);

  CURL *curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_URL, aur_endpoint);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, json_data_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)mem);
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

struct array_list *parse_results(struct JsonString *mem) {
  struct json_object *json = json_tokener_parse(mem->memory);
  struct json_object *results_ptr = json_object_object_get(json, "results");
  return json_object_get_array(results_ptr);
}
