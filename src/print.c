#include "print.h"
#include <json-c/json_object.h>
#include <stdio.h>

#define RESULT_NAME_PROP "Name"
#define RESULT_DESCRIPTION_PROP "Description"
#define RESULT_VERSION_PROP "Version"
#define RESULT_CLONE_PROP "Clone"
#define RESULT_FORMAT_STR_SIZE 30

typedef struct json_object *search_result;

char result_format_str[RESULT_FORMAT_STR_SIZE];

static const char *get_string_prop(search_result result, const char *prop_name);

static int get_json_str_len(search_result result, const char *prop_name);

static const char *get_string_prop(search_result result, const char *prop_name) {
  return json_object_get_string(json_object_object_get(result, prop_name));
}

static int get_json_str_len(search_result result, const char *prop_name) {
  return json_object_get_string_len(json_object_object_get(result, prop_name));
}

void set_column_sizes(struct array_list *search_results, struct column_sizes *cols) {
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
  snprintf(result_format_str, RESULT_FORMAT_STR_SIZE, "%%-%ds\t%%-%ds\t%%-%ds\t%%-s\n", cols->name,
           cols->description, cols->version);
}

void print_header() {
  printf(result_format_str, RESULT_NAME_PROP, RESULT_DESCRIPTION_PROP, RESULT_VERSION_PROP,
         RESULT_CLONE_PROP);
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
    snprintf(&clone[26], 73, "%s", name);
    printf(result_format_str, name, desc, version, clone);
  }
}
