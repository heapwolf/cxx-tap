# SYNOPSIS
Test Anything Protocol for `C++` based on the [TAP 13][1] Spec.


# USAGE
This module is designed to work with the [`datcxx`][0] build tool. To add this
module to your project us the following command...

```bash
build add heapwolf/cxx-tap
```


# TEST

```bash
build test
```


# EXAMPLE

### CODE

```c++
#include "deps/heapwolf/cxx-tap/index.hxx"

int main () {
  TAP::Test t;

  t.test("Bazz", [&](auto a) {
    a->ok(true, "true is true");

    a->test("Quxx", [&] (auto b) {
      b->ok(true, "nested");
      b->end();
      a->end();
    });
  });

  t.test("Foo", [&](auto t) {

    float a = 2.23;
    int b = 2;

    t->equal(a, b, "a float is not an int");

    t->ok(false, "true is also true");
    t->end(); // t is not automatically called for children.
  });

  // t.end(); // is automatically called by t's destructor.
}
```

### OUTPUT

```tap
TAP version 13
ok 1 - true is true
ok 2 - nested
not ok 3 - a float is not an int
  ---
  operator: equal
  expected: 2
  actual:   2.23
  ...
not ok 4 - true is also true
  ---
  operator: ok
  expected: false
  actual:   true
  ...
1..3
# tests 3
# pass  2
# fail  1
```

[0]:https://github.com/datcxx/build
[1]:https://testanything.org/tap-version-13-specification.html
