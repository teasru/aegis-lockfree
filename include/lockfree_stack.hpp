#pragma once
#include <atomic>

template<typename T>
class LockFreeStack {
private:
    struct Node {
        T data;
        Node* next;
        Node(T value) : data(value), next(nullptr) {}
    };

    std::atomic<Node*> head;

public:
    LockFreeStack() : head(nullptr) {}

    void push(T value) {
        Node* newNode = new Node(value);

        newNode->next = head.load(std::memory_order_relaxed);

        while (!head.compare_exchange_weak(
            newNode->next,
            newNode,
            std::memory_order_release,
            std::memory_order_relaxed
        ));
    }

    bool pop(T& result) {
        Node* oldHead = head.load(std::memory_order_acquire);

        while (oldHead &&
               !head.compare_exchange_weak(
                   oldHead,
                   oldHead->next,
                   std::memory_order_acquire,
                   std::memory_order_relaxed
               ));

        if (!oldHead)
            return false;

        result = oldHead->data;
   //     delete oldHead;    // disabled to avoid concurrent free
        return true;
    }
};
