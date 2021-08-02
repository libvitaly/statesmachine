#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <iostream>


// // A threadsafe-queue.
// template<class T>
// class Queue
// {
//   public:
//     Queue(void) : _queue(), _mutex(), _cv(), _wakeup_flag(false)
//     {}

//     ~Queue(void)
//     {}

//     // Add an element to the queue.
//     void enqueue(T t)
//     {
//         std::lock_guard<std::mutex> lock(_mutex);
//         _queue.push(t);
//         _cv.notify_one();
//     }

//     std::size_t size() const
//     {
//         std::lock_guard<std::mutex> lock(_mutex);
//         return _queue.size();
//     }

//     bool empty() const
//     {
//         std::lock_guard<std::mutex> lock(_mutex);
//         return _queue.empty();
//     }

//     void wakeup()
//     {
//         std::cout << "wakeup()" << std::endl;
//         _wakeup_flag = true;
//         _cv.notify_one();
//     }

//     // Get the "front"-element.
//     // If the queue is empty, wait till a element is avaiable.
//     std::optional<T> dequeue(void)
//     {
//         std::unique_lock<std::mutex> lock(_mutex);
//         while(_queue.empty()) {
//             // release lock as long as the wait and reaquire it afterwards.
//             _cv.wait(lock);
//         }
//             if(_wakeup_flag) {
//                 std::cout << "wakeup ... ";
//                 return {};
//             }
//         T val = _queue.front();
//         _queue.pop();
//         return val;
//     }

//   private:
//     std::queue<T> _queue;
//     mutable std::mutex _mutex;
//     std::condition_variable _cv;
//     bool _wakeup_flag;
// };


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
