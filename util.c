// Copyright 2023 Chris Palmer, https://noncombatant.org/
// SPDX-License-Identifier: Apache-2.0

#include <stdlib.h>
#include <string.h>

#include "util.h"

void* CopyNew(const void* source, size_t count) {
  return memcpy(malloc(count), source, count);
}

bool StringEquals(const char* a, const char* b) {
  return strcmp(a, b) == 0;
}

size_t StringHash(const void* string) {
  const size_t prime = 31;
  size_t h = 0;
  for (const unsigned char* p = string; *p != '\0'; p++) {
    h = prime * h + *p;
  }
  return h;
}
