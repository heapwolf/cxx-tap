#include "../index.hxx"

using namespace TAP;

int main() {

  Tap t;

  t.test("test1", [&](auto t) {
    t.ok(true, "true is true");
    t.end();
  });

  t.test("test2", [&](auto t) {

    t.test("test2a", [&](auto t) {

      t.test("test2b", [&](auto t) {
        t.ok();
      });

      t.ok();
    });

    t.ok(false, "false is true");
  });

} 

