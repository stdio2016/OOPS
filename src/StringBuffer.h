#pragma once
#ifndef STRINGBUFFER_INCLUDED
#define STRINGBUFFER_INCLUDED
#include <stddef.h>

// StringBuffer allows you to create a long string
struct StringBuffer {
  char *buf; // result string is stored here
  size_t capacity;
  size_t size; // length of string
};

// initialize StringBuffer
void StrBuf_init(struct StringBuffer *sb);

// add string at end
void StrBuf_append(struct StringBuffer *sb, const char *str);

void StrBuf_appendN(struct StringBuffer *sb, const char *str, size_t length);

// add char at end
void StrBuf_appendChar(struct StringBuffer *sb, const char ch);

// recycle StringBuffer (StringBuffer is no longer usable)
void StrBuf_destroy(struct StringBuffer *sb);

// clear StringBuffer's result string (StringBuffer can be reused)
void StrBuf_clear(struct StringBuffer *sb);

#endif
