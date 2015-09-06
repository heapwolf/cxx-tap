#ifndef TIMEOUT_H
#define TIMEOUT_H

#include <mutex>
#include <thread>
#include <condition_variable>


class Timeout {

  typedef std::function<void()> Callback;

  Callback callback;

  class {

    bool clear = false;
    struct escape {};

    std::mutex mu;
    std::condition_variable con;

    public:

      void cancel() {
        std::unique_lock<std::mutex> lock(mu);
        clear = true;
        con.notify_all();
      }

      void wait(int i) {

        auto period = std::chrono::milliseconds(i);
        auto timedout = std::cv_status::no_timeout;

        std::unique_lock<std::mutex> lock(mu);

        if (clear || con.wait_for(lock, period) == timedout) {
          clear = false;
          throw escape();
        }
      }
  } clearable;

  int ms = 0;
  int counter = 0;

  std::unique_ptr<std::thread> th;

  void loop() {
    try {
      while (true) {
        if (++counter == ms) {
          clearable.cancel();
          callback();
        }
        clearable.wait(1);
      }
    } catch (...) {
      return;
    }
  }

  public:
    void set(Callback cb, int i) {
      callback = cb;
      ms = i;

      auto fn = std::bind(&Timeout::loop, this);
      auto t = new std::thread(fn);

      th = std::unique_ptr<std::thread>(t);
    }

    void clear() {
      clearable.cancel();
      th->join();
    }

    void sleep(int n) {
      auto d = std::chrono::duration<long, std::ratio<1, 1000000> >((long) n);
      // auto ms = std::chrono::nanoseconds(d);
      std::this_thread::sleep_for(d);
    }
};

#endif

