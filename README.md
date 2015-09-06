# SYNOPSIS
Test Anything Protocol for C++ inspired by [`tape`][0].

# BUILD STATUS
[![build-status](https://travis-ci.org/hij1nx/cpp-spinaltap.svg)](https://travis-ci.org/hij1nx/cpp-spinaltap)

# EXAMPLE
```cpp
#include "../tap.h"

using namespace TAP;

int main() {

  Tap t;

  t.test("test1", [&](auto t) {
    t.ok(true, "true is true");
    t.end();
  });

  t.test("test2", [&](auto t) {

    t.test("test2a", [&](auto t) {
      t.ok();
    });

    float a = 2.23;
    int b = 2;

    t.equal(a, b, "a float is not an int");
  });
}
```

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

[0]:https://github.com/substack/tape

