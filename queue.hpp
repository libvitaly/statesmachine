#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

// A threadsafe-queue.
template <class T>
class Queue
{
public:
  Queue(void)
    : _queue()
    , _mutex()
    , _cv()
    , _wakeup_flag(false)
  {}

  ~Queue(void)
  {}

  // Add an element to the queue.
  void enqueue(T t)
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _queue.push(t);
    _cv.notify_one();
  }

  void wakeup()
  {
      _wakeup_flag = true;
      _cv.notify_one();
  }

  // Get the "front"-element.
  // If the queue is empty, wait till a element is avaiable.
  T dequeue(void)
  {
    std::unique_lock<std::mutex> lock(_mutex);
    while(_queue.empty())
    {
      // release lock as long as the wait and reaquire it afterwards.
      _cv.wait(lock);
      if(_wakeup_flag){return T{};}       
    }
    T val = _queue.front();
    _queue.pop();
    return val;
  }
private:
  std::queue<T> _queue;
  mutable std::mutex _mutex;
  std::condition_variable _cv;
  bool _wakeup_flag;
};