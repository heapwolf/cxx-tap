#ifndef TAP_H
#define TAP_H

#include <string>
#include <vector>
#include <map>
#include <exception>
#include <iostream>
#include <regex>
#include <sstream>
#include "deps/datcxx/cxx-timers/index.hxx"
#include "deps/datcxx/cxx-eventemitter/index.hxx"

namespace TAP {
  typedef std::string String;
  typedef std::exception Ex;
  typedef std::stringstream StringStream;

  template<typename T>
  String stringify(const T& t) {
    StringStream ss;
    String s;
    ss << t;
    ss >> s;
    return s;
  }

  class Test : public EventEmitter {
    bool fails = false;

    int testsExpected = 0;
    int testsActual = 0;

    int hasPlan = false;
    int assertionsExpected = 0;
    int assertionsMade = 0;
    int assertionsPassed = 0;

    int id = 0;
    int ended = 0;
    String name = "(unnamed test)";
    Test* parent;

    static bool finished;
    static bool started;

    bool asserts (bool value, const String& oper, const String& actual,
      const String& expected, const String& message);

    template<typename L, typename R, typename BinaryPredicate> 
    void assert_equal(const L&, const R&, const String&, 
      BinaryPredicate); 

    template<typename L, typename R, typename BinaryPredicate> 
    void assert_notEqual(const L&, const R&, const String&, 
      BinaryPredicate); 

    public:
      template<typename Lambda>
        void test(const String&, Lambda);

      void summarize();

      void end();
      void plan(int);
      void timeoutAfter(int);
 
      void comment(const String&);
      void pass(const String&); 
      void fail(const String&);
      void skip(int number, const String& reason);
      void ok();
      void ok(bool);
      void ok(bool, const String&);
      
      template<typename Left, typename Right>
      void equal(const Left& l, const Right& r);

      template<typename Left, typename Right>
      void equal(const Left& l, const Right& r, const String& msg);

      template<typename Left, typename Right>
      void notEqual(const Left& l, const Right& r);

      template<typename Left, typename Right>
      void notEqual(const Left& l, const Right& r, const String& msg);

      ~Test () {
        if (this->ended == 0) {
          this->end();
        }
      }
  };

  bool Test::finished = false;
  bool Test::started = false;

  void failedFinalEnd () {
    std::cout << "End of tests never reached." << std::endl;
    exit(1);
  }

  template<typename Lambda>
  void Test::test (const String& name, Lambda callback) {
    std::shared_ptr<Test> t = std::make_shared<Test>();

    if (!Test::started) {
      Test::started = true;

      std::cout << "TAP version 13" << std::endl;

      atexit([]{
        if (!Test::finished) failedFinalEnd();
      });
    }

    if (this->id == 0) {
      t->id = this->id + 1;
    } 

    this->testsExpected++;

    t->name = name;
    t->parent = this;

    callback(t);

    if (t->ended == 0) {
      this->fail("End never called.");
      return;
    }

    if (t->ended > 1) {
      this->fail("End called " + std::to_string(t->ended) + " times.");
      return;
    }

    bool assertionsMatch = t->assertionsPassed != t->assertionsExpected;
    bool assertionsFailed = t->assertionsMade - t->assertionsPassed;

    if (t->hasPlan == true && assertionsMatch) {
      this->fail("plan != count");
      return;
    }

    if (assertionsFailed > 0) {
      return;
    }

    if (t->id > 0) {
      t->parent->testsActual++;
    }
  }

  void Test::end () {
    this->ended++;

    if (this->id == 0 && this->ended == 1) {
      Test::finished = true;
      this->summarize();
    }
  }

  void Test::ok () {
    this->ok(true, "");
  }

  void Test::ok (bool value) {
    this->ok(value, "");
  }

  void Test::ok (bool value, const String& msg) {
    this->asserts(value, "ok", "true", "false", msg);
  }

  void Test::pass (const String& msg) {
    this->asserts(true, "pass", "", "", msg);
  }

  void Test::fail (const String& msg) {
    this->asserts(false, "fail", "", "", msg);
  }

  void Test::skip (int number, const String& msg) {
    this->assertionsExpected -= number;
    this->comment(msg);
  }

  void Test::comment (const String& msg) {
    String comment = msg;

    std::regex makeComment = std::regex("^#\\s*");
    std::regex makeTrim("^[\\s\\uFEFF\\xA0]+|[\\s\\uFEFF\\xA0]+$");

    comment = regex_replace(comment, makeComment, String(""));
    comment = regex_replace(comment, makeTrim, String(""));

    std::cout << comment << std::endl;
  }

  void Test::plan (int n) {
    this->hasPlan = true;
    this->assertionsExpected = n;
  }

  void Test::summarize () {
    using namespace std;

    cout
      << endl
      << "1.." << this->testsExpected << endl
      << "# tests " << this->testsExpected << endl
      << "# pass  " << this->testsActual << endl;

    if (this->testsExpected != this->testsActual) {
      auto failed = this->testsExpected - this->testsActual;
      cout << "\n# fail " << failed  << endl;
    }
    else {
      cout << "\n# ok\n" << endl;
    }

  }

  bool Test::asserts (
    bool value,
    const String& oper,
    const String& actual,
    const String& expected,
    const String& message) {
    this->assertionsMade++;
    String status = "not ok";

    if (value == true) {
      status = "ok";
      this->assertionsPassed++;
    }

    std::cout
      << status << " "
      << this->assertionsExpected << " "
      << message << std::endl;

    if (value == false) {
      StringStream ss;

      ss << "---\n";
      ss << "  operator: " << oper << "\n";
      ss << "  expected: " << expected << "\n";
      ss << "  actual:   " << actual << "\n";

      std::cout << ss.str();
    }

    return value;
  }

  template<typename L, typename R, typename BinaryPredicate> 
  void Test::assert_equal(const L& l, const R& r, const String& msg, BinaryPredicate p) {

    String message = !msg.empty() ? msg : "should be equal";
    String actual = stringify(l);
    String expected = stringify(r);

    try {
      this->asserts(!p(l, r), "equal", actual, expected, message);
    } catch(const std::exception& e) {
      this->fail(e.what());
    } catch(...) {
      this->fail("unknown exception");
    }
  }

  template<typename Left, typename Right>
  void Test::equal (const Left& l, const Right& r) {
    String msg;
    this->assert_equal(l, r, msg, std::equal_to<Left>());
  }

  template<typename Left, typename Right>
  void Test::equal(const Left& l, const Right& r, const String& msg) {
    this->assert_equal (l, r, msg, std::equal_to<Left>());
  }

  template<typename L, typename R, typename BinaryPredicate> 
  void Test::assert_notEqual (const L& l, const R& r, const String& msg, BinaryPredicate p) {

    String message = !msg.empty() ? msg : "should not be equal";
    String actual = stringify(l);
    String expected = stringify(r);

    try {
      this->asserts(!p(l, r), "notEqual", actual, expected, message);
    } catch(const Ex& e) {
      this->fail(e.what());
    } catch(...) {
      this->fail("unknown exception");
    }
  }

  template<typename Left, typename Right>
  void Test::notEqual (const Left& l, const Right& r) {
    String msg;
    this->assert_notEqual(l, r, msg, std::equal_to<Left>());
  }

  template<typename Left, typename Right>
  void Test::notEqual (const Left& l, const Right& r, const String& msg) {
    this->assert_notEqual(l, r, msg, std::equal_to<Left>());
  }
}

#endif
