# SYNOPSIS
Test Anything Protocol for `C++`.


# BUILD STATUS
[![build-status](https://travis-ci.org/heapwolf/cxx-tap.svg)](https://travis-ci.org/heapwolf/cxx-tap)


# USAGE
This module is designed to work with the [`datcxx`][0] build tool. To add this
module to your project us the following command...

```bash
build add heapwolf/cxx-tap
```


# TEST

```bash
build run test
```


# EXAMPLE

### CODE

```c++
#include "deps/heapwolf/cxx-tap/index.hxx"

using namespace TAP;

int main() {

  Test t;

  t.test("test1", [&](auto t) {

    t->ok(true, "true is true");
    t->end(); // t is not automatically called for children.
  });

  t.test("test2", [&](auto t) {

    t->test("test2a", [&](auto t) {
      t->ok();
    });

    float a = 2.23;
    int b = 2;

    t->equal(a, b, "a float is not an int");
  });

  // t.end(); // is automatically called by t's destructor :)
}
```

### OUTPUT

```
TAP version 13
# test1
ok 2 true is true
# test2
# test2a
ok 5 (unnamed assert)
not ok 6 a float is not an int
  ---
    operator: equal
    expected: 2
    actual:   2.23
  ...

1..6
# tests 6
# pass  2
# fail  1
```
