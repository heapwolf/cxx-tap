# SYNOPSIS
Test Anything Protocol for C++ inspired by [`tape`][0].

# BUILD STATUS
[![build-status](https://travis-ci.org/hij1nx/cpp-spinaltap.svg)](https://travis-ci.org/hij1nx/cpp-spinaltap)

# EXAMPLE
```cpp
#include "tap.h"
using namespace TAP;

test("example tests", [&](auto t) {

  t.test("test some stuff", [&](auto t) {

    t.plan(2);
    t.equal("foobar", "foobar");
    t.equal(1, 100);
  });

  t.test("more stuff", [&](auto t) {

    t.ok(true, "true is true");
    t.ok();
  });

  t.end();
});
```

```
TAP version 13
# test one
ok 1 should be equal
not ok 2 should be equal
  ---
    operator: equal
    expected: 100
    actual:   1
  ...
# test two
ok 3 true is true
ok 4 (unnamed assert)

1..4
# tests 4
# pass  3
# fail  1
```

[0]:https://github.com/substack/tape

