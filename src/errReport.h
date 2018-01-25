#pragma once
#ifndef ERROR_REPORT_INCLUDED
#define ERROR_REPORT_INCLUDED

#ifdef COLORTEXT
  #define RED_TEXT "\x1B[91;1m"
  #define NORMAL_TEXT "\x1B[0m"
  #define BOLD_TEXT "\x1B[1m"
#else
  #define RED_TEXT
  #define NORMAL_TEXT
  #define BOLD_TEXT
#endif

extern int errorCount;
void semanticError(const char *fmt, ...);
void syntaxError(const char *fmt, ...);

#endif
