#include "request.h"

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

struct array_list *parse_results(struct MemoryStruct *chunk) {
  struct json_object *json = json_tokener_parse(chunk->memory);
  struct json_object *results_ptr = json_object_object_get(json, "results");
  return json_object_get_array(results_ptr);
}

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb,
                           void *userp) {
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

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

const char *get_string_prop(search_result result, const char *prop_name) {
  return json_object_get_string(json_object_object_get(result, prop_name));
}

int get_json_str_len(search_result result, const char *prop_name) {
  return json_object_get_string_len(json_object_object_get(result, prop_name));
}
