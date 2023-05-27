# hashset — A hash set/map library for C

This is a simple implementation of a hash set (or map, depending on what you put
in it) for C. I hope it’s easy to understand and easy to integrate into C
projects.

It has no dependencies other than the standard C library.

For documentation, see hashset.h.

For usage examples, see test.c.

To use it, `git clone` it into your project’s source tree.

## Notes On The Interface Design

### What About Encapsulation?

It is possible to encapsulate the implementation details by putting only
`typedef struct HashSet HashSet;` in the header file, changing `HashSetNew` to
return a heap-allocated `HashSet*`, and defining the structure fully only in the
implementation. (And doing the same for `HashSetIterator`.) I decided to expose
the details because it makes the library more useful, with no costs:

* Callers that want to ignore the internals still can; the public functions are
  sufficient.
* Callers can hold `HashSet`s on the stack or in static storage. If they want to
  store `HashSet`s on the heap, they still can.
* Callers can write code that inspects `HashSet` internals, as seen in
  `TestStringHashUniformity`. This can be crucial for writing time- or
  space-efficiency tests.

### Why Aren’t The Key And Value Types Distinct?

C does not have generics or templates, which means we have 2 basic design
choices: use C preprocessor macros to reinvent templates, or use dynamic typing
on opaque pointers. I’ve chosen the latter approach because I like the small
object code size and the clean look of calling code. But macros certainly can
work well, too.

Given that decision, and given that a set is really just a map with no value
type, overloading the element type allows us to get both a set and a map with no
additional source or object code.

So, `HashSet` is a set if you use it for objects that are keys, and it’s a map
if you use it for objects that have a key part and a value part.
