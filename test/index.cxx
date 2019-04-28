#include "../index.hxx"
#include "../stacktrace.hxx"

#include <csignal>

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

    t->comment("this is a comment");
    t->equal(a, b, "a float is not an int");

    //
    // For stacktrace testing...
    //
    // std::raise(SIGSEGV);

    t->ok(false, "true is also true");
    t->end(); // t is not automatically called for children.
  });

  // t.end(); // is automatically called by t's destructor.
}
