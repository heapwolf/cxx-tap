# SYNOPSIS
Simple timeouts for C++

# USAGE

## CONSTRUCTOR
```cpp
Timeout t;
```

## METHODS

### set(lambda cb, int n)
Set a timeout for n milliseconds

```cpp
t.set([]() {
  cout << "ok, done";
}, 100 * 10);
```

### sleep(int n)
Go to sleep n for milliseconds

```cpp
t.sleep(100);
```

### clear()
Clear a timeout that has been set.

```cpp
t.clear();
```

