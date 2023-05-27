// Copyright 2023 Chris Palmer, https://noncombatant.org/
// SPDX-License-Identifier: Apache-2.0

#include <sys/stat.h>

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashset.h"
#include "util.h"

// Example: A dictionary of words and their definitions. The `word` is the key.

typedef struct Word {
  char* word;
  char* definition;
} Word;

static size_t WordHash(const void* word) {
  const Word* w = word;
  return StringHash(w->word);
}

static int WordCompare(const void* a, const void* b) {
  const Word* w1 = a;
  const Word* w2 = b;
  return strcmp(w1->word, w2->word);
}

static void TestDictionary() {
  HashSet set = HashSetNew(10, WordHash, WordCompare);

  Word cat = {.word = "cat", .definition = "A fine animal indeed"};
  Word dog = {.word = "dog",
              .definition = "A friend who likes to play frisbee"};

  HashSetAdd(&set, &cat);
  HashSetAdd(&set, &dog);
  assert(HashSetContains(&set, &(Word){.word = "cat"}));
  assert(HashSetContains(&set, &(Word){.word = "dog"}));

  Word* result = HashSetGet(&set, &(Word){.word = "cat"});
  assert(StringEquals(result->definition, cat.definition));
  result = HashSetGet(&set, &(Word){.word = "dog"});
  assert(StringEquals(result->definition, dog.definition));

  char new_cat_definition[] = "A nice friend who loves food";
  Word* c = HashSetGet(&set, &(Word){.word = "cat"});
  c->definition = new_cat_definition;
  Word* c2 = HashSetGet(&set, &(Word){.word = "cat"});
  assert(StringEquals(c2->definition, new_cat_definition));

  HashSetDelete(&set);
}

// Example: A sparse array of words. The `index` is the key.

typedef struct Item {
  size_t index;
  char* word;
} Item;

static size_t ItemHash(const void* item) {
  const Item* i = item;
  return i->index;
}

static int ItemCompare(const void* a, const void* b) {
  const Item* i1 = a;
  const Item* i2 = b;
  if (i1->index < i2->index) {
    return -1;
  } else if (i1->index > i2->index) {
    return 1;
  }
  return 0;
}

static void TestSparseArray() {
  HashSet set = HashSetNew(10, ItemHash, ItemCompare);

  Item i1 = {.index = 1, .word = "item 1"};
  Item i273 = {.index = 273, .word = "item 273"};
  Item i6000 = {.index = 6000, .word = "item 6000"};

  HashSetAdd(&set, &i1);
  HashSetAdd(&set, &i273);
  HashSetAdd(&set, &i6000);

  assert(HashSetContains(&set, &i1));
  assert(HashSetContains(&set, &i273));
  assert(HashSetContains(&set, &i6000));
  assert(!HashSetContains(&set, &(Item){.index = 2}));

  HashSetDelete(&set);
}

static void TestAddUpdates() {
  HashSet set = HashSetNew(10, ItemHash, ItemCompare);

  Item i1 = {.index = 1, .word = "hello"};
  Item i273 = {.index = 273, .word = "world"};
  Item i6000 = {.index = 6000, .word = "wow"};

  HashSetAdd(&set, &i1);
  HashSetAdd(&set, &i273);
  HashSetAdd(&set, &i6000);

  Item* item = HashSetGet(&set, &(Item){.index = 1});
  assert(StringEquals(item->word, i1.word));
  item = HashSetGet(&set, &(Item){.index = 273});
  assert(StringEquals(item->word, i273.word));
  item = HashSetGet(&set, &(Item){.index = 6000});
  assert(StringEquals(item->word, i6000.word));

  Item* i1_new = CopyNew(&(Item){.index = 1, .word = "HELLO"}, sizeof(Item));
  HashSetAdd(&set, i1_new);
  item = HashSetGet(&set, &(Item){.index = 1});
  assert(StringEquals(item->word, i1_new->word));

  size_t count = 0;
  HashSetIterator it = HashSetIteratorNew(&set);
  while ((item = HashSetIteratorNext(&it))) {
    count++;
  }
  assert(count == 3);

  free(i1_new);
  HashSetDelete(&set);
}

// Example: A set of unique files on a POSIX filesystem.

// A file on a POSIX system is uniquely identified by the combination of its
// `dev_t` and its `ino_t`. In this example, we also store an associated
// `value`.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
typedef struct FileID {
  dev_t device;
  ino_t inode;
  char* value;
} FileID;
#pragma clang diagnostic pop

static size_t FileIDHasher(const void* file_id) {
  const FileID* id = file_id;
  return 31U * (size_t)id->device * (size_t)id->inode;
}

static int FileIDComparator(const void* a, const void* b) {
  const FileID* id1 = a;
  const FileID* id2 = b;
  if (id1->device < id2->device) {
    return -1;
  } else if (id1->device > id2->device) {
    return 1;
  }
  if (id1->inode < id2->inode) {
    return -1;
  } else if (id1->inode > id2->inode) {
    return 1;
  }
  return 0;
}

// static void HashSetPrint(HashSet* set) {
//   HashSetIterator it = HashSetIteratorNew(set);
//   FileID* id;
//   while ((id = HashSetIteratorNext(&it))) {
//     printf("device %d inode %llu\n", id->device, id->inode);
//   }
// }

static void TestAddContains() {
  HashSet set = HashSetNew(500, FileIDHasher, FileIDComparator);
  assert(set.count >= 500);
  for (size_t i = 0; i < set.count; i++) {
    assert(NULL == set.elements[i]);
  }

  // Try a few manually first:
  HashSetAdd(&set, CopyNew(&(FileID){.device = 1, .inode = 1}, sizeof(FileID)));
  HashSetAdd(&set, CopyNew(&(FileID){.device = 2, .inode = 1}, sizeof(FileID)));
  HashSetAdd(&set, CopyNew(&(FileID){.device = 1, .inode = 2}, sizeof(FileID)));
  assert(HashSetContains(&set, &(FileID){.device = 1, .inode = 1}));
  assert(HashSetContains(&set, &(FileID){.device = 2, .inode = 1}));
  assert(HashSetContains(&set, &(FileID){.device = 1, .inode = 2}));
  assert(!HashSetContains(&set, &(FileID){.device = 1, .inode = 3}));

  // Now try many:
  for (ino_t i = 0; i < 1000; i++) {
    FileID id = {.device = 1, .inode = i};
    HashSetAdd(&set, CopyNew(&id, sizeof(FileID)));
  }
  for (ino_t i = 0; i < 1000; i++) {
    const FileID good = {.device = 1, .inode = i};
    assert(HashSetContains(&set, &good));
    const FileID bad = {.device = 5000, .inode = i};
    assert(!HashSetContains(&set, &bad));
  }

  HashSetIterator it = HashSetIteratorNew(&set);
  FileID* id;
  while ((id = HashSetIteratorNext(&it))) {
    free(id);
  }
  HashSetDelete(&set);
}

static void TestAddContainsGetUpdate() {
  HashSet set = HashSetNew(100, FileIDHasher, FileIDComparator);
  for (ino_t i = 0; i < 1000; i++) {
    FileID* id = CopyNew(&(FileID){.device = 1, .inode = i}, sizeof(FileID));
    HashSetAdd(&set, id);
  }
  for (ino_t i = 0; i < 1000; i++) {
    const FileID good = {.device = 1, .inode = i};
    assert(HashSetContains(&set, &good));
    const FileID bad = {.device = 5000, .inode = i};
    assert(!HashSetContains(&set, &bad));
  }
  for (ino_t i = 0; i < 1000; i++) {
    const FileID id = {.device = 1, .inode = i};
    FileID* got = HashSetGet(&set, &id);
    assert(got);
    got->value = "goat";
  }
  for (ino_t i = 0; i < 1000; i++) {
    const FileID id = {.device = 1, .inode = i};
    const FileID* got = HashSetGet(&set, &id);
    assert(StringEquals(got->value, "goat"));
  }
  for (ino_t i = 0; i < 1000; i++) {
    const FileID id = {.device = 1, .inode = i};
    FileID* got = HashSetGet(&set, &id);
    assert(got);
    HashSetRemove(&set, got);
    free(got);
    assert(!HashSetContains(&set, &id));
  }
  HashSetDelete(&set);
}

static void TestIterator() {
  HashSet set = HashSetNew(10, FileIDHasher, FileIDComparator);
  for (ino_t i = 0; i < 10; i++) {
    FileID* id = CopyNew(&(FileID){.device = 1, .inode = i}, sizeof(FileID));
    HashSetAdd(&set, id);
  }

  HashSetIterator it = HashSetIteratorNew(&set);
  ino_t seen = 0;
  FileID* id;
  while ((id = HashSetIteratorNext(&it))) {
    assert(1 == id->device);
    assert(id->inode >= 0 && id->inode < 10);
    seen++;
  }
  assert(10 == seen);

  for (ino_t i = 0; i < 10; i++) {
    id = HashSetGet(&set, &(FileID){.device = 1, .inode = i});
    assert(id);
    HashSetRemove(&set, id);
    free(id);
  }
  HashSetDelete(&set);
}

// Example: Using a `HashSet` to test the time- and space-efficiency of
// `HashSet` itself.

// These are used in `TestStringHashUniformity`: For each bucket size, count how
// many times it occurs. We want to see mostly short buckets, and few empty
// buckets.
typedef struct SizeCount {
  size_t size;
  size_t count;
} SizeCount;

static size_t SizeCountHash(const void* sc) {
  const SizeCount* c = sc;
  return c->size;
}

static int SizeCountCompare(const void* a, const void* b) {
  const SizeCount* c1 = a;
  const SizeCount* c2 = b;
  if (c1->size < c2->size) {
    return -1;
  } else if (c1->size > c2->size) {
    return 1;
  }
  return 0;
}

static void TestStringHashUniformity() {
  // Populate a set with all the words.
  FILE* words = fopen("/usr/share/dict/words", "r");
  assert(words);
  HashSet word_set = HashSetNew(80000, StringHash, (Comparator*)strcmp);
  char* word = NULL;
  size_t capacity = 0;
  while (getline(&word, &capacity, words) > 0) {
    HashSetAdd(&word_set, strdup(word));
  }
  (void)fclose(words);

  // Iterate over `word_set`’s buckets, and store their lengths. This requires
  // knowing the internals of `HashSet`, which normal calling code would not
  // (need to) do.
  HashSet size_counts = HashSetNew(50, SizeCountHash, SizeCountCompare);
  for (size_t i = 0; i < word_set.count; i++) {
    HashSetElements* e = word_set.elements[i];
    size_t size = 0;
    while (e) {
      size++;
      e = e->next;
    }

    SizeCount* sc = HashSetGet(&size_counts, &(SizeCount){.size = size});
    if (sc) {
      sc->count += 1;
    } else {
      HashSetAdd(&size_counts, CopyNew(&(SizeCount){.size = size, .count = 1},
                                       sizeof(SizeCount)));
    }
  }

  HashSetIterator it = HashSetIteratorNew(&word_set);
  char* w;
  while ((w = HashSetIteratorNext(&it))) {
    free(w);
  }

  // This is safe because we aren’t mutating the structure of the `HashSet`,
  // just freeing the regions that it contains pointers to.
  it = HashSetIteratorNew(&size_counts);
  SizeCount* sc;
  while ((sc = HashSetIteratorNext(&it))) {
    printf("%zu %zu\n", sc->size, sc->count);
    free(sc);
  }
}

int main(int count, char* arguments[]) {
  TestDictionary();
  TestSparseArray();
  TestAddUpdates();
  TestAddContains();
  TestAddContainsGetUpdate();
  TestIterator();
  if (count > 1 && StringEquals(arguments[1], "uniformity")) {
    TestStringHashUniformity();
  }
}
