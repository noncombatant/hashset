// Copyright 2023 Chris Palmer, https://noncombatant.org/
// SPDX-License-Identifier: Apache-2.0

#ifndef HASHSET_H
#define HASHSET_H

#include <stdbool.h>
#include <stddef.h>

// A hash map/set of opaque, dynamically typed elements.
//
// The set is structured as an array of pointers to list heads (often called
// “buckets”). If this array is large enough relative to the number of elements
// inserted, and if the `Hasher` produces a uniform distribution, the lists will
// tend to be short enough for lookup to be fast.
//
// Elements are typed and inspected only by their `Hasher` and `Comparator`
// functions; the `HashSet` itself sees only opaque pointers. Callers retain
// ownership of the elements.
//
// Elements have a “key part”, and an optional “value part”. `Hasher` and
// `Comparator` operate only on the key part, and only the caller knows about
// the value part (if any).
//
// This enables elements to be anything: plain keys (a set), or (key, value)
// pairs (a map). All that matters is that their hasher and comparator operate
// correctly for the key part of the element.

// Returns a suitable hash value for the key part of `element`. For time
// efficiency in lookup operations, this function should return a uniform
// distribution.
typedef size_t Hasher(const void* element);

// Returns negative if the key part of `a` sorts before that of `b`, positive if
// it sorts after, and 0 if they compare equal.
typedef int Comparator(const void* a, const void* b);

typedef struct HashSetElements {
  void* element;
  struct HashSetElements* next;
} HashSetElements;

typedef struct HashSet {
  size_t count;
  HashSetElements** elements;
  Hasher* hasher;
  Comparator* comparator;
} HashSet;

void HashSetAdd(HashSet* set, void* element);

bool HashSetContains(const HashSet* set, const void* element);

// `free`s the `HashSet`’s internal storage, but not the elements. The caller
// owns the elements.
void HashSetDelete(HashSet* set);

// Returns the element in `set` matching the key part of `element`, or `NULL` if
// no matching element is present.
void* HashSetGet(const HashSet* set, const void* element);

HashSet HashSetNew(size_t count, Hasher* hasher, Comparator* comparator);

// Removes from `set` the element matching the key part of `element`, if one is
// present.
void HashSetRemove(HashSet* set, const void* element);

typedef struct HashSetIterator {
  size_t bucket;
  HashSetElements* element;
  HashSet* set;
} HashSetIterator;

// Returns a `HashSetIterator` that starts at the beginning of `set`.
HashSetIterator HashSetIteratorNew(HashSet* set);

// Returns the next element, or `NULL` if iteration has ended.
void* HashSetIteratorNext(HashSetIterator* i);

#endif
