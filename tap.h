#ifndef TAP_H
#define TAP_H

#include <cxxabi.h>
#include <string>
#include <vector>
#include <map>
#include <exception>
#include <iostream>
#include <regex>
#include <sstream>
#include "cc_modules/timeout/index.h"
#include "cc_modules/eventemitter/index.h"

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

  class Test;
  class ResultsRenderer;

  //
  // TODO
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
    int count = 0;
    int id = 0;
    int timeout = 4000;
    int todo = -1;
    int skip = -1;
  };

  struct Options : BaseOptions {
    Error error;
    BaseOptions extra;
  };

  struct Results {
    int count = 0;
    int pass = 0;
    int fail = 0;

    int plan = 0;
    int assertCount = 0;
    bool planError = false;
  };

  class Test : public EventEmitter {

    vector<Test> progeny;

    int _skip = false;
    int assertCount = 0;
    int pendingCount = 0;

    bool _ok = true;
    bool _planError = false;
    bool calledEnd = false;
    bool hasCallback = false;
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

      int _plan = 0;
      bool watched = false;
      void run();

      template<typename Lambda>
      void test(const string&, Lambda);

      template<typename Lambda>
      void test(const string&, Options, Lambda);

      void _test(Test);

      void comment(const string&);
      void plan(int);
      void timeoutAfter(int);
 
      void pass(const string&); 
      void pass(const string&, Options); 

      void skip(const string&);
      void skip(const string&, Options);

      template<typename Lambda>
      void skip(const string&, Lambda);

      template<typename Lambda>
      void skip(const string&, Options, Lambda);

      void fail(const string&);
      void fail(const string&, Options);
    
      void ok();
      void ok(bool);
      void ok(bool, const string&);
      void ok(bool, const string&, Options);

      void notOk();
      void notOk(bool);
      void notOk(bool, const string&);
      void notOk(bool, const string&, Options);

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

      string name;
      Options opts;
      function<void(Test)> _cb;

      Test() {}

      template<typename Lambda>
      Test(const string& s, Options o, Lambda lambda) :
        name(s),
        opts(o),
        _cb(lambda)
      {
        hasCallback = true;
        Options opts;
      }
  };

  void Test::run() {
    if (this->_skip > 0) {
      this->_end();
      return;
    }

    this->emit("prerun", this);
    if (this->hasCallback) {
      this->_cb(*this);
    }
    this->emit("run");
  }

  template<typename Callback>
  void Test::test(const string& name, Callback cb) {
    Options opts;
    this->test(name, opts, cb);
  }

  template<typename Callback>
  void Test::test(const string& name, Options opts, Callback cb) {

    Test t(name, opts, cb);
    this->_test(t);
  }

  void Test::_test(Test t) {

    this->pendingCount++;

    t.on("prerun", [&]() {
      this->assertCount++;
    });

    this->emit("test", &t);

    this->progeny.push_back(t);

    if (!this->_pendingAsserts()) {
      this->_end();
    }

    if (this->_plan == 0 && this->pendingCount == this->progeny.size()) {
      this->_end();
    }
  }

  void Test::comment(const string& msg) {

    Options opts;
    regex makeComment = regex("^#\\s*");
    regex makeTrim("^[\\s\\uFEFF\\xA0]+|[\\s\\uFEFF\\xA0]+$");

    opts.comment = regex_replace(msg, makeComment, string(""));
    opts.comment = regex_replace(opts.comment, makeTrim, string(""));
    this->emit("results", &opts);
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

  //
  // end()
  //
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

  //
  // _end();
  //
  void Test::_end() {
    Error err;
    this->_end(err);
  }

  void Test::_end(Error err) {

    if (this->progeny.size() > 0) {

      auto t = this->progeny.front();
      this->progeny.erase(this->progeny.begin());

      t.on("end", [&]() { 
        this->_end();
      });

      t.run();
      return;
    }

    int pendingAsserts = this->_pendingAsserts();

    if ((this->_plan != this->assertCount) && this->_plan != 0 && pendingAsserts > 0) {
      this->_planError = true;

      Options o;
      o.expected = to_string(this->_plan);
      o.actual = to_string(this->assertCount);

      this->fail("plan != count", o);
    }

    if (!this->ended) {
      // TODO allow emit with no events registered.
      this->on("end", [](){});
      this->emit("end");
    }

    this->ended = true;
  }

  int Test::_pendingAsserts() {
    if (this->_plan == 0) {
      return 0;
    } else {
      int len = this->progeny.size();
      int i = this->_plan - (len + this->assertCount);
      return i;
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
    // Report some more information about the       cout << "FAIL" << endl;failed assertion.
    // 
    // if (!ok) {
    //   res.exceptionDetails
    //   res.functionName
    //   res.file
    //   res.line
    //   res.column
    //   res.at
    // }

    this->emit("results", &res);

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

  template<typename Callback>
  void Test::skip(const string& msg, Callback cb) {
    Options opts;
    opts.skip = true;
    this->test(msg, opts, cb);
  }

  template<typename Callback>
  void Test::skip(const string& msg, Options opts, Callback cb) {
    opts.skip = true;
    this->test(msg, opts, cb);
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


  void Test::notOk() {
    Options extra;
    this->notOk(false, "", extra);
  }

  void Test::notOk(bool value) {
    Options extra;
    this->notOk(value, "", extra);
  }

  void Test::notOk(bool value, const string& msg) {
    Options extra;
    this->notOk(value, msg, extra);
  }

  void Test::notOk(bool value, const string& msg, Options extra) {

    Options o;
    o.message = msg;
    o.oper = "notOk";
    o.expected = "false";
    o.actual = !value ? "false" : "true";
    o.extra = extra;
    this->_assert(value, o);
  }

  template<typename Type>
  string getTypeName() {

    int status;
    string name = typeid(Type).name();
    char *realname = abi::__cxa_demangle(name.c_str(), NULL, NULL, &status);

    if (status == 0) {
      name = realname;
      free(realname);
    }
    return name;
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
      o.actual = stringify(l) + " (" + getTypeName<L>() + ")";
      o.expected = stringify(r) + " (" + getTypeName<R>() + ")";
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
      o.actual = stringify(l) + "(" + getTypeName<L>() + ")";
      o.notExpected = stringify(r) + "(" + getTypeName<R>() + ")";
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

  static Results r;

  //
  // RESULTS
  //
  class ResultsRenderer : public EventEmitter {

    public:
      bool first = true;

      string padding_outer = "  ";
      string padding_inner = padding_outer + "  ";

      string encodeResult(Options*, int);
      void close(Results);
      void watch(Test*);

      ResultsRenderer() {

        this->on("plan", [&](int n) {
          r.plan = n;
        });

        static function<void()> onProcessExit;

        onProcessExit = [&]() {
          this->close(r);
        };

        atexit([]{
          onProcessExit();
        });
      }
  };

  void ResultsRenderer::watch(Test* t) {

    t->watched = true;

    t->on("plan", [&](int n) {
      if (t->name.size())
        r.plan += n;
    });

    t->on("prerun", [&](Test* t) {
      if (first) {
        first = false;
        cout << "TAP version 13" << endl;
      }
      cout << "# " << t->name << endl;
      ++r.count;
    });

    t->on("results", [&](Options* res) {

      if (!res->comment.empty()) {
        cout << "# " << res->comment << endl;
        return;
      }

      res->count = ++r.count;

      cout << this->encodeResult(res, r.count);

      if (res->ok) {
        r.pass++;
      }
      else {
        r.fail++;
      }

    });

    t->on("end", [&](Test* t) {
      
    });

    t->on("test", [&](Test* st) {
      this->watch(st);
    });
  }

  string ResultsRenderer::encodeResult(Options* res, int count) {

    string output = "";

    output += (res->ok ? "ok " : "not ok ") + to_string(count);
    output += !res->name.empty() ? " " + res->name : "";

    if (res->skip > -1) output += " # SKIP";
    else if (res->todo > -1) output += " # TODO";
  
    output += "\n";
    if (res->ok) return output;

    output += padding_outer + "---\n";
    output += padding_inner + "operator: " + res->oper + "\n";

    if (!res->expected.empty() || !res->actual.empty()) {

      string ex = !res->expected.empty() ? res->expected : "";
      string ac = !res->actual.empty() ? res->actual : "";
  
      if (max(ex.size(), ac.size()) > 65) {
        output += padding_inner + "expected: |-\n" + 
          padding_inner + "  " + ex + "\n";
        output += padding_inner + "actual: |-\n" + 
          padding_inner + "  " + ac + "\n";
      } else {
        output += padding_inner + "expected: " + ex + "\n";
        output += padding_inner + "actual:   " + ac + "\n";
      }
    }

    //if (res.at) {
    //    output += inner + "at: " + res.at + "\n";
    //}

    if (res->oper == "error" && res->error) {

      output += padding_inner + "exception: |-\n" + 
        padding_inner + "  " + res->error.message;

      //for (int i = 0; i < lines.length(); i++) {
      //    output += inner + "  " + lines[i] + "\n";
      //}
    }

    output += padding_outer + "...\n";
    return output + "";
  }

  void ResultsRenderer::close(Results r) {

    if (r.plan != 0 && r.plan != r.count) {

      string output = "";
      
      output += "not ok " + to_string(r.count) + " plan != count\n";
      output += padding_outer + "---\n";
      output += padding_inner + "operator: fail\n";
      output += padding_inner + "expected: " + to_string(r.plan) + "\n";
      output += padding_inner + "actual:   " + to_string(r.count) + "\n";
      cout << output;
      return;
    }

    cout
      << endl
      << "1.." << r.count << endl
      << "# tests " << r.count << endl
      << "# pass  " << r.pass << endl;

    if (r.fail) {
      cout << "# fail  " << r.fail << endl;
    }
    else {
      cout << "\n# ok\n" << endl;
    }
  }

  ResultsRenderer results;

  //
  // HARNESS
  //
  class Tap : public Test {
    public:

      Tap();
      
      template<typename Callback>
      Tap(const string& name, Callback cb);

      template<typename Callback>
      Tap(const string& name, Options opts, Callback cb);
  };

  Tap::Tap() {
    results.watch(this);
  }

  template<typename Callback>
  Tap::Tap(const string& name, Callback cb) : 
    Test(name, cb) {
      results.watch(this);
  }

  template<typename Callback>
  Tap::Tap(const string& name, Options opts, Callback cb) : 
    Test(name, opts, cb) {
      results.watch(this);
  }

}

#endif

