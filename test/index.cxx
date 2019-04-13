#include "../index.hxx"

int main () {
  TAP::Test t;

  t.test("Hello", [&](auto t) {
    t->ok(true, "true is true");
    t->end(); // t is not automatically called for children.
  });

  // t.end(); // is automatically called by t's destructor.
}
