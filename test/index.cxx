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

  t.test("Foo", [&](auto t) {

    float a = 2.23;
    int b = 2;

    t->equal(a, b, "a float is not an int");

    t->ok(false, "true is also true");
    t->end(); // t is not automatically called for children.
  });

  // t.end(); // is automatically called by t's destructor.
}
