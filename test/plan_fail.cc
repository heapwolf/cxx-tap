#include "../tap.h"

using namespace TAP;

int main() {

  Tap t;

  t.plan(2);

  t.test("test1", [&](auto t) {
    t.end();
  });

  t.test("test2", [&](auto t) {
    t.plan(2);
    t.ok();
    t.ok();
    t.ok();
    t.end();
  });
} 

