#include "errReport.h"
#include <stdio.h>
#include <stdarg.h>

int errorCount = 0;
extern int linenum;             /* declared in tokens.l */

void semanticError(const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  printf(RED_TEXT "<Error>" NORMAL_TEXT " found in Line " BOLD_TEXT "%d: " NORMAL_TEXT, linenum);
  vprintf(fmt, ap);
  va_end(ap);
  errorCount++;
}

void syntaxError(const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  printf(RED_TEXT "<Error>" NORMAL_TEXT " found in Line " BOLD_TEXT "%d: " NORMAL_TEXT, linenum);
  vprintf(fmt, ap);
  va_end(ap);
  errorCount++;
}

