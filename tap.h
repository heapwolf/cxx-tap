#ifndef TAP_H
#define TAP_H

#include <string>
#include <vector>
#include <map>
#include <exception>
#include <iostream>
#include <regex>
#include <sstream>
#include "cc_modules/timeout@1.0.0/index.h"
#include "cc_modules/eventemitter@2.0.0/index.h"

namespace TAP {

  using namespace std;

  template<typename T>
  string stringify(const T& t) {
    stringstream ss;
    string s;
    ss << t;
    ss >> s;
    return s;
  }

  class Error : public runtime_error {
    public:
      string message = "";
      bool state = false;

      inline operator bool() const {
        return state;
      }

      void operator =(bool s) {
        state = s;
      }

      Error() : runtime_error("Unknown Error") {}

      Error(const string& s) : 
        message(s),
        runtime_error(s) {}
  };

  //
  //
  // TODO
  // Break-out test functions into test.h
  // 
  struct BaseOptions {
    string name;
    string message;
    string comment;
    string actual;
    string expected;
    string notExpected;
    string oper;
    bool ok = false;
    bool bail = false;
    bool exiting = false;
    int id = 0;
    int timeout = 4000;
    int todo = -1;
    int skip = -1;
  };

  struct Options : BaseOptions {
    Error error;
    BaseOptions extra;
  };

  class Test : public EventEmitter {

    vector<Test*> progeny;

    int _skip = false;
    int _plan = -1;
    int assertCount = 0;
    int pendingCount = 0;

    bool _ok = true;
    bool _planError = false;
    bool calledEnd = false;
    bool ended = false;

    void _end(Error);
    void _end();
    
    
    int _pendingAsserts();
    
    void _assert(bool, Options);

    template<typename L, typename R, typename BinaryPredicate> 
    void assert_equal(const L&, const R&, const string&, 
      BinaryPredicate, Options); 

    template<typename L, typename R, typename BinaryPredicate> 
    void assert_notEqual(const L&, const R&, const string&, 
      BinaryPredicate, Options); 

    public:

      bool watched = false;
      void run();
      void quit();

      template<typename Callback>
      void test(const string&, Callback);

      template<typename Callback>
      void test(const string&, Options, Callback);

      void test(Test);

      void comment(const string&);
      void plan(int);
      void timeoutAfter(int);
 
      void pass(const string&); 
      void pass(const string&, Options); 
      
      void skip(const string& msg);
      void skip(const string& msg, Options);

      void fail(const string&);
      void fail(const string&, Options);
    
      void ok();
      void ok(bool);
      void ok(bool, const string&);
      void ok(bool, const string&, Options);
      
      template<typename Left, typename Right>
      void equal(const Left& l, const Right& r);

      template<typename Left, typename Right>
      void equal(const Left& l, const Right& r, const string& msg);

      template<typename Left, typename Right>
      void equal(const Left& l, const Right& r, const string& msg, Options extra);

      template<typename Left, typename Right>
      void notEqual(const Left& l, const Right& r);

      template<typename Left, typename Right>
      void notEqual(const Left& l, const Right& r, const string& msg);

      template<typename Left, typename Right>
      void notEqual(const Left& l, const Right& r, const string& msg, Options extra);

      void error(Error);
      void error(Error, const string&);
      void error(Error, const string&, Options);

      void end(Error);
      void end();

      string name = "(anonymous)";
      Options opts;
      function<void(Test)> _cb;
      function<void(Test*)> resultsWatcher;
  };

  void Test::run() {
    if (this->_skip > 0) {
      this->_end();
      return;
    }

    this->emit("prerun");
    this->_cb(*this);
    this->emit("run");
  }

  template<typename Callback>
  void Test::test(const string& name, Callback cb) {
    Options opts;
    return this->test(name, opts, cb);
  }

  template<typename Callback>
  void Test::test(const string& name, Options opts, Callback cb) {

    Test t;
    t.name = name;
    t.opts = opts;
    t._cb = cb;

    this->test(t);
  }

  void Test::test(Test t) {

    this->resultsWatcher(&t);
    t.resultsWatcher = this->resultsWatcher;

    this->pendingCount++;
    this->progeny.push_back(&t);
    this->emit("test", t);

    t.on("prerun", [&]() {
      this->assertCount++;
    });

    if (this->_pendingAsserts() <= 0) {
      this->_end();
    }

    if (this->_plan < 1 && this->pendingCount == this->progeny.size()) {
      this->_end();
    }
  }

  void Test::comment(const string& msg) {

    Options opts;
    regex makeComment = regex("^#\\s*");
    regex makeTrim("^[\\s\\uFEFF\\xA0]+|[\\s\\uFEFF\\xA0]+$");

    opts.comment = regex_replace(msg, makeComment, string(""));
    opts.comment = regex_replace(opts.comment, makeTrim, string(""));
    this->emit("results", opts);
  }

  void Test::plan(int n) {
    this->_plan = n;
    this->emit("plan", n);
  }

  void Test::timeoutAfter(int ms) {

    Timeout t;

    t.set([&]() {
      this->fail("test timed out after " + to_string(ms) + "ms");
      this->end();
    }, ms);

    this->once("end", [&]() {
      t.clear();
    });
  }

  void Test::_end() {
    Error err;
    this->_end(err);
  }

  void Test::end() {
    Error err;
    this->end(err);
  }

  void Test::end(Error err) { 

    if (err) {
      this->error(err);
    }
    
    if (this->calledEnd) {
      this->fail(".end() called twice");
    }

    this->calledEnd = true;
    this->_end();
  }

  void Test::_end(Error err) {

    if (this->progeny.size() > 0) {

      // shift off the first element as `t`
      auto t = this->progeny[0];
      this->progeny.erase(this->progeny.begin());

      t->on("end", [&]() { 
        this->_end();
        this->pendingCount--;
      });

      t->run();
      return;
    }

    if (!this->ended && this->pendingCount == 0) { 
      this->emit("end");
    }

    int pendingAsserts = this->_pendingAsserts();

    if (!this->_planError && this->_plan > -1 && pendingAsserts > 0) {
      this->_planError = true;

      Options o;
      o.expected = this->_plan;
      o.actual = to_string(this->assertCount);

      this->fail("plan != count", o);
    }
    this->ended = true;
  }

  void Test::quit() {

    Options o;

    if (this->_plan > -1 &&
      !this->_planError && this->assertCount != this->_plan) {
      this->_planError = true;

      o.expected = this->_plan;
      o.actual = to_string(this->assertCount);
      o.exiting = true;

      this->fail("plan != count", o);
    }
    else if (!this->ended) {

      o.exiting = true;
      this->fail("test exited without ending", o);
    }
  }

  int Test::_pendingAsserts() {
    if (this->_plan == -1) {
      return 1;
    }
    else {
      return this->_plan - (this->progeny.size() + this->assertCount);
    }
  }

  void Test::_assert(bool ok, Options opts) {

    Options extra;
    Options res;
    opts.extra = extra;

    res.id = this->assertCount++;
    res.ok = ok;

    if (extra.skip > -1) {
      res.skip = extra.skip;
    } else if (opts.skip > -1) {
      res.skip = opts.skip;
    }

    if (!extra.message.empty()) res.name = extra.message;
    else if (!opts.message.empty()) res.name = opts.message;
    else res.name = "(unnamed assert)";

    if (!extra.oper.empty()) res.oper = extra.oper;
    else if (!opts.oper.empty()) res.oper = opts.oper;

    if (!extra.actual.empty()) res.actual = extra.actual;
    else if (!opts.actual.empty()) res.actual = opts.actual;

    if (!extra.expected.empty()) res.expected = extra.expected;
    else if (!opts.expected.empty()) res.expected = opts.expected;

    this->_ok = this->_ok && ok;

    if (!ok) {
      Error err(res.name);

      if (extra.error) res.error = extra.error;
      else if (opts.error) res.error = opts.error;
      else res.error = err;
    }

    //
    // TODO
    // Report some more information about the failed assertion.
    // 
    // if (!ok) {
    //   res.exceptionDetails
    //   res.functionName
    //   res.file
    //   res.line
    //   res.column
    //   res.at
    // }

    this->emit("results", res);

    int pendingAsserts = this->_pendingAsserts();

    if (!pendingAsserts) {
      this->_end();
    }

    if (!this->_planError && pendingAsserts < 0) {
      this->_planError = true;

      Options o;
      o.expected = this->_plan;
      o.actual = to_string(this->_plan - pendingAsserts);

      this->fail("plan != count", o);
    }
  }

  void Test::fail(const string& msg) {
    Options extra;
    this->fail(msg, extra);
  }

  void Test::fail(const string& msg, Options extra) {

    Options o;
    o.message = msg;
    o.oper = "fail";
    o.extra = extra;

    this->_assert(false, o);
  }

  void Test::pass(const string& msg) {
    Options extra;
    this->pass(msg, extra);   
  }

  void Test::pass(const string& msg, Options extra) {

    Options o;
    o.message = msg;
    o.oper = "pass";
    o.extra = extra;

    this->_assert(true, o);
  }

  void Test::skip(const string& msg) {
    Options extra;
    this->skip(msg, extra);
  }

  void Test::skip(const string& msg, Options extra) {
 
    Options o;
    o.message = msg;
    o.oper = "skip";
    o.skip = true;
    o.extra = extra;   

    this->_assert(true, o);
  }

  void Test::ok() {
    Options extra;
    this->ok(true, "", extra);
  }

  void Test::ok(bool value) {
    Options extra;
    this->ok(value, "", extra);
  }

  void Test::ok(bool value, const string& msg) {
    Options extra;
    this->ok(value, msg, extra);
  }

  void Test::ok(bool value, const string& msg, Options extra) {

    Options o;
    o.message = msg;
    o.oper = "ok";
    o.expected = "true";
    o.actual = !value ? "false" : "true";
    o.extra = extra;
    this->_assert(value, o);
  }

  template<typename L, typename R, typename BinaryPredicate> 
  void Test::assert_equal(
    const L& l, 
    const R& r, 
    const string& msg, 
    BinaryPredicate p, 
    Options extra) {
   
      Options o;
      o.message = !msg.empty() ? msg : "should be equal";
      o.oper = "equal";
      o.actual = stringify(l);
      o.expected = stringify(r);
      o.extra = extra;

      try {
        this->_assert(p(l, r), o);
      }
      catch(const exception& e) {
        this->fail(e.what(), o);
      }
      catch(...) {
        this->fail("unknown exception", o);
      }
  }

  template<typename Left, typename Right>
  void Test::equal(const Left& l, const Right& r) {
    string msg;
    Options extra;
    this->assert_equal(l, r, msg, equal_to<Left>(), extra);
  }

  template<typename Left, typename Right>
  void Test::equal(const Left& l, const Right& r, const string& msg) {
    Options extra;
    this->assert_equal(l, r, msg, equal_to<Left>(), extra);
  }

  template<typename Left, typename Right>
  void Test::equal(
    const Left& l, 
    const Right& r, 
    const string& msg, 
    Options extra) {
      this->assert_equal(l, r, msg, equal_to<Left>(), extra);
  }

  template<typename L, typename R, typename BinaryPredicate> 
  void Test::assert_notEqual(
    const L& l, 
    const R& r, 
    const string& msg, 
    BinaryPredicate p, 
    Options extra) {
 
      Options o;
      o.message = !msg.empty() ? msg : "should not be equal";
      o.oper = "notEqual";
      o.actual = stringify(l);
      o.notExpected = stringify(r);
      o.extra = extra;

      try {
        this->_assert(!p(l, r), o);
      }
      catch(const exception& e) {
        this->fail(e.what(), o);
      }
      catch(...) {
        this->fail("unknown exception", o);
      }
  }

  template<typename Left, typename Right>
  void Test::notEqual(const Left& l, const Right& r) {
    string msg;
    Options extra;
    this->assert_notEqual(l, r, msg, equal_to<Left>(), extra);
  }

  template<typename Left, typename Right>
  void Test::notEqual(const Left& l, const Right& r, const string& msg) {
    Options extra;
    this->assert_notEqual(l, r, msg, equal_to<Left>(), extra);
  }

  template<typename Left, typename Right>
  void Test::notEqual(const Left& l, const Right& r, const string& msg, Options extra) {
    this->assert_notEqual(l, r, msg, equal_to<Left>(), extra);
  }

  void Test::error(Error err) {
    Options extra;
    this->error(err, "", extra);
  }

  void Test::error(Error err, const string& msg) {
    Options extra;
    this->error(err, msg, extra);
  }

  void Test::error(Error err, const string& msg, Options extra) {

    Options o;
    o.message = msg;
    o.oper = "error";
    o.actual = err.message;
    o.extra = extra;

    this->_assert(!err, o);
  }

  //
  // RESULTS
  // TODO
  // Break-out results into results.h
  //
  class Results : public EventEmitter {

    string _only;
    
    public:
      int count = 0;
      int fail = 0;
      int pass = 0;
      bool first = true;
      bool closed = false;
      vector<Test*> tests;

      void push(Test&);
      void only(const string&);
      void _watch(Test&);
      string encodeResult(Options, int);
      void close(int count, int pass, int fail);
  };

  void Results::push(Test& t) {
    t.watched = true;
    this->tests.push_back(&t);
    this->_watch(t);
    this->emit("_push", t);
  }

  /* void Results::only(const string& name) {
    if (!this->_only.empty()) {
      this->count++;
      this->fail++;
      cout
        << "not ok " 
        << this->count 
        << " already called .only()" 
        << endl;
    }
    this->_only = name;
  }*/

  void Results::_watch(Test& t) {

    t.on("prerun", [&]() {
      if (first) {
        first = false;
        cout << "TAP version 13" << endl;
      }
      cout << "# " << t.name << endl;
    });

    t.on("results", [&](Options res) {

      if (!res.comment.empty()) {
        cout << "# " << res.comment << endl;
        return;
      }
      cout << this->encodeResult(res, this->count + 1);
      this->count++;

      if (res.ok) {
        this->pass++;
      }
      else {
        this->fail++;
      }
    });

    t.on("results", [&](Options res) {  
      this->emit("update", *this);
    });

    t.on("test", [&](Test st) { 
      this->_watch(st); 
    });
  }

  string Results::encodeResult(Options res, int count) {

    string output = "";

    output += (res.ok ? "ok " : "not ok ") + to_string(count);
    output += !res.name.empty() ? " " + res.name : "";

    if (res.skip > -1) output += " # SKIP";
    else if (res.todo > -1) output += " # TODO";
  
    output += "\n";
    if (res.ok) return output;

    string outer = "  ";
    string inner = outer + "  ";
    output += outer + "---\n";
    output += inner + "operator: " + res.oper + "\n";

    if (!res.expected.empty() || !res.actual.empty()) {

      string ex = !res.expected.empty() ? res.expected : "";
      string ac = !res.actual.empty() ? res.actual : "";
  
      if (max(ex.size(), ac.size()) > 65) {
        output += inner + "expected: |-\n" + inner + "  " + ex + "\n";
        output += inner + "actual: |-\n" + inner + "  " + ac + "\n";
      } else {
        output += inner + "expected: " + ex + "\n";
        output += inner + "actual:   " + ac + "\n";
      }
    }

    //if (res.at) {
    //    output += inner + "at: " + res.at + "\n";
    //}

    if (res.oper == "error" && res.error) {

      output += inner + "exception: |-\n" + 
        inner + "  " + res.error.message;

      //for (int i = 0; i < lines.length(); i++) {
      //    output += inner + "  " + lines[i] + "\n";
      //}
    }

    output += outer + "...\n";
    return output + "";
  }

  void Results::close(int count, int pass, int fail) {

    cout
      << endl
      << "1.." << count << endl
      << "# tests " << count << endl
      << "# pass  " << pass << endl;

    if (fail) {
      cerr << "# fail  " << fail << endl;
    }
    else {
      cout << "\n# ok\n" << endl;
    }
  }

  //
  // HARNESS
  //
  class test {

    public:
      Test harness;
      Results results;

      template<typename Callback>
      test(const string&, Callback);
  };

  template<typename Callback>
  test::test(const string& name, Callback cb) {

    Options opts;

    harness.name = name;
    harness.opts = opts;
    harness._cb = cb;

    //
    // declre the final results static because by the
    // time `atexit` fires, everything else will have
    // gone off-stack.
    //

    static int count = 10;
    static int pass = 10;
    static int fail = 0;

    this->results.on("update", [&](Results res) {
      count = res.count;
      pass = res.pass;
      fail = res.fail;
    });

    harness.resultsWatcher = [&](Test* child) {
      this->results.push(*child);
    };

    static function<void()> onProcessExit;
    
    onProcessExit = [&]() {
      this->results.close(count, pass, fail);
    };

    atexit([]{
      onProcessExit();
    });

    harness.run();
  }
}

#endif

