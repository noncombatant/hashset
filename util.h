// Copyright 2023 Chris Palmer, https://noncombatant.org/
// SPDX-License-Identifier: Apache-2.0

#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

#define COUNT(array) (sizeof((array)) / sizeof((array)[0]))

// Allocates `count` bytes on the heap, copies `count` bytes from `source` into
// that allocation, and returns a pointer to the allocation.
void* CopyNew(const void* source, size_t count);

// Returns true if `a` equals `b`.
bool StringEquals(const char* a, const char* b);

// Treats `key` as a `NUL`-terminated C string and returns a decently-uniform
// hash of it.
size_t StringHash(const void* key);

#endif
