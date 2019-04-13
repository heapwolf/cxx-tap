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
      t.ok();
    });

    float a = 2.23;
    int b = 2;

    t.equal(a, b, "a float is not an int");
  });
}

