#include "print.h"
#include "request.h"
#include "types.h"
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * TODO: Wrap long output and fit to terminal size
 *       Sort param, Raw Line Based Output, Raw Json output params
 */

void cleanup(struct MemoryStruct *chunk, CURL *curl);

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
