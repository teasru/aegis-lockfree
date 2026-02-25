#pragma once
#include <stack>
#include <mutex>

template<typename T>
class MutexStack {
private:
    std::stack<T> stack;
    std::mutex mtx;

public:
    void push(T value) {
        std::lock_guard<std::mutex> lock(mtx);
        stack.push(value);
    }

    bool pop(T& result) {
        std::lock_guard<std::mutex> lock(mtx);

        if (stack.empty())
            return false;

        result = stack.top();
        stack.pop();
        return true;
    }
};
