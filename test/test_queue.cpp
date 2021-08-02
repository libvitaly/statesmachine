#include "../queue.hpp"

#include <gtest/gtest.h>

#include <vector>
#include <algorithm>
#include <thread>
#include <string>


TEST(queue, total) {

  struct Foobar{int _v; std::string _text;};

  Queue<Foobar> q;

  std::vector<Foobar> foobar(64);

  std::generate(foobar.begin(), foobar.end(), [](){
    static int curr = 0;
    return Foobar{++curr, std::to_string(curr)};
  });

  int timeout = 30;

  auto t1 = std::thread([&q, &foobar, timeout](){
    for(auto const& item: foobar){
      q.enqueue(item);
      std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
    }
  });
  
  auto t2 = std::thread([&q, &foobar, timeout](){
    for(auto const& item: foobar){
      auto curr = q.dequeue();
      if(curr)
      {
        ASSERT_EQ(curr.value()._v, item._v);
        EXPECT_STREQ(curr.value()._text.c_str(), item._text.c_str());
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout/3));
      }
    }
  });

  t1.join();
  t2.join();

  ASSERT_EQ(q.size(), 0);
}
