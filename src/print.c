#include "print.h"
#include "request.h"

char result_format_str[RESULT_FORMAT_STR_SIZE];

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
