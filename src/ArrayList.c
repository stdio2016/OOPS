#include <stdlib.h>
#include "ArrayList.h"
#define BASE_ARRAY_SIZE 5

int ArrayList_init(struct ArrayList *arr) {
  arr->capacity = BASE_ARRAY_SIZE;
  arr->items = malloc(sizeof(RefType) * arr->capacity);
  arr->size = 0;
  if (arr->items == NULL) {
    return 0;
  }
  return 1;
}

void ArrayList_destroy(struct ArrayList *arr) {
  free(arr->items);
  arr->items = NULL;
}

void ArrayList_add(struct ArrayList *arr, RefType obj) {
  if (arr->size >= arr->capacity) {
    arr->capacity *= 2;
    arr->items = realloc(arr->items, sizeof(RefType) * arr->capacity);
    if (arr->items == NULL) {
      exit(-1);
    }
  }
  arr->items[arr->size++] = obj;
}

void ArrayList_clear(struct ArrayList *arr) {
  arr->capacity = BASE_ARRAY_SIZE;
  arr->items = realloc(arr->items, sizeof(RefType) * arr->capacity);
  arr->size = 0;
}

RefType ArrayList_get(struct ArrayList *arr, size_t index) {
  return arr->items[index];
}

void ArrayList_set(struct ArrayList *arr, size_t index, RefType obj) {
  arr->items[index] = obj;
}

RefType ArrayList_pop(struct ArrayList *arr) {
  arr->size--;
  return arr->items[arr->size];
}
