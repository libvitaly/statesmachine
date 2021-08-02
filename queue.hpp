#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <iostream>


// // A threadsafe-queue.
template<typename T>
class Queue
{
  private:
    std::mutex _mutex;
    std::condition_variable _cv;
    std::deque<T> _queue;
    bool _wakeup_flag = false;
  public:
    void enqueue(T const& value)
    {
        {
            std::unique_lock<std::mutex> lock(this->_mutex);
            _queue.push_front(value);
        }
        this->_cv.notify_one();
    }
    std::optional<T> dequeue()
    {
        std::unique_lock<std::mutex> lock(this->_mutex);
        this->_cv.wait(lock, [=]{ return !this->_queue.empty() || this->_wakeup_flag; });
        T rc(std::move(this->_queue.back()));
        this->_queue.pop_back();
        if(this->_wakeup_flag){
         return {};
        }else{
         return rc;
        }
    }
    void wakeup()
    {
        _wakeup_flag = true;
        _cv.notify_one();
    }
    int size()
    {
        return _queue.size();
    }
};
