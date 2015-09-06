#include "tap.h"

using namespace TAP;

int main() {

  test("timing test", [&](auto t) {

    t.test("test one", [&](auto t) {

      t.plan(2);
      t.equal("foobar", "foobar");
      t.equal(1, 100);
    });

    t.test("test two", [&](auto t) {

      t.ok(true, "true is true");
      t.ok();
    });

    t.end();
  });

} 

