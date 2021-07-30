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
    std::string _name;
public:
    using EventType = E;
    using ContextType = C;
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

//template<typename S>
template<typename E, typename C>
class StateMachine
{
protected:    
    using S = State<E, C>;
public:
    StateMachine(C *context): _context(context), _current_state(nullptr){}
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
    void run(S* state) {
        if(!state || !_states.count(state)) {
            throw std::runtime_error{"invalid state"};
        }
        _current_state = state;
        _context->_out = _current_state->name();
        _thread = std::thread([this](){
            _continue_flag = true;
            while(_continue_flag)
            {
                auto event = _queue.dequeue();
                if(event != E{}){
                    onEvent(event);
                    _context->_out += ", " + _current_state->name();
                }else{
                    _continue_flag = false;
                } 
            }
        });
    }
    void stop() {
        _continue_flag = false;
        _queue.wakeup();
    }
    void wait() {
        if(_thread.joinable()) _thread.join();
    }
    void pushEvent(const E& event) {
        _queue.enqueue(event);
    }
    S const& currentState() {return *_current_state; }
protected:
    void onEvent(const E& event) {
        const auto old_state = _current_state;
        _current_state = _current_state->onEvent(event);
        if(!_states.count(_current_state)) {
            throw std::runtime_error("invalid state");
        }
        // if(old_state != _current_state)
        // {
        //     std::cout << old_state->name() << "(" << event << ") -> " << _current_state->name() << std::endl;
        // }
    }
private:
    C* _context;
    S* _current_state;
    std::unordered_set<S*> _states;
    Queue<E> _queue;
    std::thread _thread;
    bool _continue_flag;
};

}//namespace fsm