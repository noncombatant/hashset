// Copyright 2023 Chris Palmer, https://noncombatant.org/
// SPDX-License-Identifier: Apache-2.0

#include <stdlib.h>
#include <string.h>

#include "hashset.h"
#include "util.h"

// `TestStringHashUniformity` shows that rounding up bucket counts results in
// overall much worse space inefficiency, with little time efficiency benefit.
// It might help to have finer-grained `PrimeSizes`, though.
//
// static size_t RoundUpPrime(size_t size) {
//   // https://planetmath.org/goodhashtableprimes
//   const static size_t PrimeSizes[] = {
//       3,         7,         13,        23,       31,       53,        97,
//       193,       389,       769,       1543,     3079,     6151,      12289,
//       24593,     49157,     98317,     196613,   393241,   786433, 1572869,
//       3145739,   6291469,   12582917,  25165843, 50331653, 100663319,
//       201326611,
//       402653189, 805306457, 1610612741};
//   for (size_t i = 0; i < COUNT(PrimeSizes); i++) {
//     if (PrimeSizes[i] >= size) {
//       return PrimeSizes[i];
//     }
//   }
//   return size;
// }

void HashSetAdd(HashSet* set, void* element) {
  const size_t hash = set->hasher(element) % set->count;
  HashSetElements* es = set->elements[hash];
  if (es == NULL) {
    set->elements[hash] = malloc(sizeof(HashSetElements));
    set->elements[hash]->element = element;
    set->elements[hash]->next = NULL;
    return;
  }
  while (es) {
    if (set->comparator(es->element, element) == 0) {
      es->element = element;
      return;
    }
    if (es->next == NULL) {
      es->next = malloc(sizeof(HashSetElements));
      es->next->element = element;
      es->next->next = NULL;
      return;
    }
    es = es->next;
  }
}

bool HashSetContains(const HashSet* set, const void* element) {
  return HashSetGet(set, element) != NULL;
}

void HashSetDelete(HashSet* set) {
  for (size_t i = 0; i < set->count; i++) {
    for (HashSetElements* es = set->elements[i]; es != NULL;) {
      HashSetElements* next = es->next;
      free(es);
      es = next;
    }
  }
  free(set->elements);
}

void* HashSetGet(const HashSet* set, const void* element) {
  const size_t hash = set->hasher(element) % set->count;
  HashSetElements* es = set->elements[hash];
  while (es) {
    if (set->comparator(es->element, element) == 0) {
      return es->element;
    }
    es = es->next;
  }
  return NULL;
}

HashSet HashSetNew(size_t count, Hasher* hasher, Comparator* comparator) {
  return (HashSet){.count = count,
                   .elements = calloc(count, sizeof(HashSetElements*)),
                   .hasher = hasher,
                   .comparator = comparator};
}

void HashSetRemove(HashSet* set, const void* element) {
  const size_t hash = set->hasher(element) % set->count;
  HashSetElements* es = set->elements[hash];
  HashSetElements* previous = NULL;
  while (es) {
    if (set->comparator(es->element, element) == 0) {
      if (previous) {
        previous->next = es->next;
      } else {
        set->elements[hash] = es->next;
      }
      free(es);
      return;
    }
    previous = es;
    es = es->next;
  }
}

HashSetIterator HashSetIteratorNew(HashSet* set) {
  return (HashSetIterator){
      .bucket = 0, .element = set->elements[0], .set = set};
}

void* HashSetIteratorNext(HashSetIterator* i) {
  while (i->bucket < i->set->count) {
    if (i->element) {
      HashSetElements* e = i->element;
      i->element = e->next;
      return e->element;
    }
    if (++(i->bucket) == i->set->count) {
      break;
    }
    i->element = i->set->elements[i->bucket];
  }
  return NULL;
}
