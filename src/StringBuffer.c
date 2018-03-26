#include "StringBuffer.h"
#include <stdlib.h>
#include <string.h>
#define BASE_CAPACITY 5

void StrBuf_init(struct StringBuffer *sb) {
  sb->size = 0;
  sb->capacity = BASE_CAPACITY;
  sb->buf = malloc(sb->capacity);
  sb->buf[0] = '\0';
}

void StrBuf_append(struct StringBuffer *sb, const char *str) {
  size_t n = strlen(str);
  if (sb->capacity - sb->size >= n+1) {
    // yay!
  }
  else {
    size_t dbl = sb->capacity * 2;
    if (sb->size + n+1 > dbl) {
      dbl = sb->size + n+1;
    }
    char *n = realloc(sb->buf, dbl);
    if (n == NULL) { exit(-1); } // failed >_<
    sb->buf = n;
    sb->capacity = dbl;
  }
  memcpy(sb->buf+sb->size, str, n+1);
  sb->size += n;
}

void StrBuf_appendN(struct StringBuffer *sb, const char *str, size_t length) {
  if (sb->capacity - sb->size >= length+1) {
    // yay!
  }
  else {
    size_t dbl = sb->capacity * 2;
    if (sb->size + length+1 > dbl) {
      dbl = sb->size + length+1;
    }
    char *n = realloc(sb->buf, dbl);
    if (n == NULL) { exit(-1); } // failed >_<
    sb->buf = n;
    sb->capacity = dbl;
  }
  memcpy(sb->buf+sb->size, str, length);
  sb->size += length;
  sb->buf[sb->size] = '\0';
}

void StrBuf_appendChar(struct StringBuffer *sb, const char ch) {
  if (sb->capacity - sb->size >= 2) {
    // yay!
  }
  else {
    size_t dbl = sb->capacity * 2;
    if (sb->size + 2 > dbl) {
      dbl = sb->size + 2;
    }
    char *n = realloc(sb->buf, dbl);
    if (n == NULL) { exit(-1); } // failed >_<
    sb->buf = n;
    sb->capacity = dbl;
  }
  sb->buf[sb->size] = ch;
  sb->buf[sb->size + 1] = '\0';
  sb->size++;
}

void StrBuf_destroy(struct StringBuffer *sb) {
  free(sb->buf);
}

void StrBuf_clear(struct StringBuffer *sb) {
  sb->size = 0;
  // not really cleared, buf good enough
  sb->buf[0] = '\0';
}
