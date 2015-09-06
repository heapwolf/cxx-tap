#ifndef __EVENTS_H_
#define __EVENTS_H_

#include <iostream>
#include <stdexcept>
#include <functional>
#include <typeinfo>
#include <string>
#include <unordered_map>

class EventEmitter {

  typedef std::tuple<bool, void*> Event;
  typedef std::unordered_multimap<std::string, Event> Events;

  template <typename Callback> 
  struct traits : public traits<decltype(&Callback::operator())> {
  };

  template <typename ClassType, typename R, typename... Args>
  struct traits<R(ClassType::*)(Args...) const> {

    typedef std::function<R(Args...)> fn;
  };

  template <typename Callback>
  typename traits<Callback>::fn
  to_function (Callback& cb) {

    return static_cast<typename traits<Callback>::fn>(cb);
  }

  int _listeners = 0;

  public:

    Events events;
    int maxListeners = 10;

    int listeners() {
      return this->_listeners;
    }

    int listeners(std::string name) {
      return events.count(name);
    }

    template <typename Callback>
    void on(std::string name, Callback cb) {
      this->on(name, false, cb);
    }

    template <typename Callback>
    void on(std::string name, bool once, Callback cb) {

      if (++this->_listeners >= this->maxListeners) {
        std::cout 
          << "warning: possible EventEmitter memory leak detected. " 
          << this->_listeners 
          << " listeners added. "
          << std::endl;
      };

      auto f = to_function(cb);
      auto fn = new decltype(f)(to_function(cb));
      auto event = std::make_tuple(once, static_cast<void*>(fn));
      events.insert(std::make_pair(name, event));
    }

    template <typename Callback>
    void once(std::string name, Callback cb) {
      this->on(name, true, cb);
    }

    void off() {
      events.clear();
      this->_listeners = 0;
    }

    void off(std::string name) {

      int count = events.count(name);

      if (count) {
        auto range = events.equal_range(name);
        events.erase(name);
        this->_listeners -= count;
      }
    }

    template <typename ...Args> 
    void emit(std::string name, Args... args) {

      int count = events.count(name);

      if (count) {
        auto range = events.equal_range(name);
        int index = 0;

        for (Events::iterator it=range.first; it!=range.second; ++it) {
          auto cb = std::get<1>(it->second);
          auto fp = static_cast<std::function<void(Args...)>*>(cb);
          (*fp)(args...);

          if (std::get<0>(it->second) == true) {
            events.erase(it);
            this->_listeners--;
          }
        }
      }
    }

    EventEmitter() {}

    ~EventEmitter () {
      events.clear();
    }
};

#endif

