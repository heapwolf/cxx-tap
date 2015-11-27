# SYNOPSIS
Test Anything Protocol for C++ inspired by [`tape`][0].

# BUILD STATUS
[![build-status](https://travis-ci.org/0xxff/cpp-spinaltap.svg)](https://travis-ci.org/0xxff/cpp-spinaltap)

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
    expected: 2 (int)
    actual:   2.23 (float)
  ...

1..6
# tests 6
# pass  2
# fail  1
```

# API

## CONSTRUCTOR

### Tap t;
Creates a lightweight test "harness".

## INSTANCE METHODS

### `void` test(string name, [Options opts,] lambda callback);
Create a new test with a `name` string and optional `Options` struct. 
cb(t) fires with the new test object once all preceeding tests have
finished. Tests execute serially.

Available opts options are:

opts.skip = true/false. See test.skip.
opts.timeout = 500. Set a timeout for the test, after which it will
fail. See the instance member `timeoutAfter`.

If you forget to t.plan() out how many assertions you are going to
run and you don't call t.end() explicitly, your test will hang.

### `void` skip(string name, [Options opts,] lambda callback);
Generate a new test that will be skipped over.

### `void` skip(string msg, [Options opts]);
Generate an assertion that will be skipped over.

### `void` plan(int n);
Declare that `n` assertions should be run. `t.end()` will be called
automatically after the nth assertion. If there are any more assertions
after the nth, or after `t.end()` is called, they will generate errors.

### `void` end([Error err]);
Declare the end of a test explicitly. If `err` is passed in, `t.end()` 
will assert that it is falsey.

### `void` fail(string msg);
Generate a failing assertion with a message `msg`.

### `void` pass(string msg);
Generate a passing assertion with a message `msg`.

### `void` timeoutAfter(int ms);
Automatically timeout the test after X ms.

### `void` ok(bool value[, string msg]);
Assert that value is truthy with an optional description message `msg`.

### `void` notOk(bool value[, string msg]);
Assert that value is falsy with an optional description message `msg`.

### `void` error(Error err[, string msg][, Options options]);
Assert that err is falsy. If err is non-falsy, use its `err.message`
as the description message.

### `void` equal(Type actual, Type expected, string msg);
Assert that `actual` is the same type and value as `expected`
with an optional description msg

### `void` notEqual(Type actual, Type expected, string msg);
Assert that `actual` is not the same type and value as `expected`
with an optional description msg

### `void` comment(string msg);
Print a message without breaking the tap output. (Useful when using
e.g. tap-colorize where output is buffered & console.log will print
in incorrect order vis-a-vis tap output.)

[0]:https://github.com/substack/tape

