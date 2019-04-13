#include "../index.hxx"

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

  t.test("Hello", [&](auto t) {
    t->ok(false, "true is also true");
    t->end(); // t is not automatically called for children.
  });

  // t.end(); // is automatically called by t's destructor.
}
