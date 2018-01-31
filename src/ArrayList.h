#pragma once
#ifndef ARRAYLIST_INCLUDED
#define ARRAYLIST_INCLUDED
#include <stddef.h>

// ArrayList can only store pointers
typedef void* RefType;

struct ArrayList {
  size_t size;
  size_t capacity;
  RefType *items;
};

int ArrayList_init(struct ArrayList *arr);
void ArrayList_destroy(struct ArrayList *arr);

void ArrayList_add(struct ArrayList *arr, RefType obj);
void ArrayList_clear(struct ArrayList *arr);
RefType ArrayList_get(struct ArrayList *arr, size_t index);
void ArrayList_set(struct ArrayList *arr, size_t index, RefType obj);
RefType ArrayList_pop(struct ArrayList *arr);

#endif
