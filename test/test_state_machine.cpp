#include <gtest/gtest.h>

#include "../states.hpp"

/*
|----------------------------------------------------------------------|
|            _____       _____        _____           _____            |
| (SATRT)--->| s0 |--e1->| s1 |--e2-->| s2 |---e3,--->| s4 |--->(END)  |
|            |____|      |____| ++cnt |____| cnt >= 3 |____|           |
|               +           +            |              |              |
|               |           |            e3, cnt < 3    |              |
|               |           |          __+__            |              |
|               |           <---e4-----| s3 |           |              |         
|               |                      |____|           |              | 
|               |                                       |              |
|               <---------------e5, cnt = 0 -------------              |       
|                                                                      |
|----------------------------------------------------------------------|
*/

TEST(state_machine, run) 
{
    struct Context {
        int count = 0;
        std::string _out;
    }context;

    using Event = std::string;

    fsm::StateMachine<Event, Context> state_machine(&context);
    
    auto s0 = state_machine.createState("s0");
    auto s1 = state_machine.createState("s1");
    auto s2 = state_machine.createState("s2");
    auto s3 = state_machine.createState("s3");
    auto s4 = state_machine.createState("s4");

    s0->setRule(s1, [](const auto& e) { return e == "e1"; });
    s1->setRule(s2, [&context](const auto& e) { return e == "e2" ? context.count++, true : false; });
    s2->setRule(s3, [&context](const auto& e) { return e == "e3" && context.count < 3; });
    s2->setRule(s4, [&context](const auto& e) { return e == "e3" && context.count >= 3; });
    s3->setRule(s1, [](const auto& e) { return e == "e4"; });
    s4->setRule(s0, [&context](const auto& e) { context.count = 0; return e == "e5"; });

    state_machine.run(s0);

    auto events = {"e1", "e2", "e3", "e4", "e2", "e3", "e4", "e2", "e3", "e4", "e3", "e5", ""};

    for(auto const& e: events)
    {
        state_machine.pushEvent(e);
    }

    state_machine.wait();

    EXPECT_STREQ(context._out.c_str(), "s0, s1, s2, s3, s1, s2, s3, s1, s2, s4, s4, s4, s0");
}
