#pragma once
#ifndef MyHash_INCLUDED
#define MyHash_INCLUDED
struct HashBucket {
  struct HashBucket *next;
  void *key;
  void *value;
};

struct MyHash {
  struct HashBucket **_buckets;
  size_t _nb; // number of buckets
  size_t _size; // number of keys

  // compare function
  // if a < b return < 0
  // if a = b return 0
  // if a > b return > 0
  int (*_cmp)(const void *a, const void *b);
  size_t (*_hf)(const void *key); // hash function
};

struct MyHashIterator {
  struct MyHash *table;
  size_t bucket;
  struct HashBucket *it; // points to current bucket or NULL if at end
};

void MyHash_Init(struct MyHash *table, int (*cmpFn)(const void *, const void *), size_t (*hashFn)(const void *));

// returns HashBucket if key exists, NULL if not
struct HashBucket *MyHash_Get(struct MyHash *table, const void *key);

// return old value if key exists, NULL if not
// old value is replaced
void *MyHash_Set(struct MyHash *table, void *key, void *value);

// returns HashBucket if key exists, NULL if not
struct HashBucket *MyHash_Delete(struct MyHash *table, const void *key);

void MyHash_Resize(struct MyHash *table, size_t newSize);

// stores iterator into iter
// Don't modify the hash table when iterating
void MyHash_Iterate(struct MyHash *table, struct MyHashIterator *iter);

void MyHash_IterateNext(struct MyHashIterator *iter);

#endif
