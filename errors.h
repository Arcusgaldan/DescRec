#ifndef Oberon_errors_h
#define Oberon_errors_h

#include "scanner.h"

typedef enum _error {
  error_log,
  error_info,
  error_tip,
  error_warning,
  error_scanner,
  error_parser,
  error_fatal,
  error_unknown
} error_t;

void mark_at(const error_t error, const position_t position, const char *message, ...);
void mark(const error_t error, const char *message, ...);
void mark_missing(symbol_t symbol);

#endif
