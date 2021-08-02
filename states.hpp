#pragma once

#include "queue.hpp"

#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
#include <unordered_set>
#include <string>
#include <thread>

namespace fsm
{

template<typename E, typename C>
class State
{
    const std::string _name;
public:
    State( C& c, std::string const& name_): _name(name_), _context(c){}
    std::string const& name() const { return _name; }
    using Predicate = std::function<bool(const E&)>;
    virtual ~State() = default;  
    void setRule(State* state, Predicate pred) {
        _edges.emplace_back(state, std::move(pred));
    }
    State* onEvent(const E& event) {
        for(auto&[next, pred] : _edges) {
            if(pred(event)) {
                return next;
            }
        }
        return this;
    }
private:
    std::vector<std::pair<State*, Predicate>>  _edges;
    C &_context;
};

template<typename E, typename C>
class StateMachine
{   
    using S = State<E, C>;
public:
    StateMachine(std::shared_ptr<C> context): _context(context), _current_state(nullptr){}
    ~StateMachine()
    {
        stop();
        wait();
        for(auto& s: _states) {
            delete s;
        }
    }
    template<typename... Args>
    S* createState(Args&&... args) {
        if(_context == nullptr) throw std::runtime_error("unknown context");
        return static_cast<S*>(*_states.insert(new S{*_context, std::forward<Args>(args)...}).first);
    }
    //async event processing
    void run(S* start_state, S* exit_state)
    {
        if(!start_state || !_states.count(start_state)) {
            throw std::runtime_error{"invalid state"};
        }
        _current_state = start_state;
#ifdef GTEST_ON                 
        _context->_out = _current_state->name();
#endif //#ifdef GTEST_ON        
        _thread = std::thread([this, exit_state](){
            while(auto event = _queue.dequeue())
            { 
                onEvent(event.value());
#ifdef GTEST_ON             
                _context->_out += ", " + _current_state->name();     
#endif //#ifdef GTEST_ON                
                if(_current_state == exit_state) break;
            }
        });
    }
    void stop() {
        _queue.wakeup();
    }
    void wait() {
        if(_thread.joinable()) _thread.join();
    }
    void pushEvent(const E& event) {
        _queue.enqueue(event);
    }
    //sync event processing
    void onEvent(const E& event) {
        const auto old_state = _current_state;
        _current_state = _current_state->onEvent(event);
        if(!_states.count(_current_state)) {
            throw std::runtime_error("invalid state");
        }
    }
    void setStartState(S* start_state)
    {
        if(!start_state || !_states.count(start_state)) {
            throw std::runtime_error{"invalid state"};
        }
        _current_state = start_state;
    }
    std::string const& currSateName() const {return _current_state->name();}
private:
    std::shared_ptr<C> _context;
    S* _current_state;
    std::unordered_set<S*> _states;
    Queue<E> _queue;
    std::thread _thread;
};

}//namespace fsm