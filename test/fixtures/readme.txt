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
