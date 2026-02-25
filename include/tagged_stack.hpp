#pragma once
#include <atomic>
#include <cstdint>

template<typename T>
class TaggedStack {
private:
    struct Node {
        T data;
        Node* next;
        Node(T value) : data(value), next(nullptr) {}
    };

    struct TaggedPtr {
        Node* ptr;
        std::uint64_t tag;
    };

    std::atomic<TaggedPtr> head;

public:
    TaggedStack() {
        head.store({nullptr, 0});
    }

    void push(T value) {
        Node* newNode = new Node(value);
        TaggedPtr oldHead = head.load(std::memory_order_acquire);

        while (true) {
            newNode->next = oldHead.ptr;

            TaggedPtr newHead;
            newHead.ptr = newNode;
            newHead.tag = oldHead.tag + 1;

            if (head.compare_exchange_weak(
                    oldHead,
                    newHead,
                    std::memory_order_release,
                    std::memory_order_relaxed))
                break;
        }
    }

    bool pop(T& result) {
        TaggedPtr oldHead = head.load(std::memory_order_acquire);

        while (oldHead.ptr) {
            TaggedPtr newHead;
            newHead.ptr = oldHead.ptr->next;
            newHead.tag = oldHead.tag + 1;

            if (head.compare_exchange_weak(
                    oldHead,
                    newHead,
                    std::memory_order_acquire,
                    std::memory_order_relaxed)) {

                result = oldHead.ptr->data;
                delete oldHead.ptr;
                return true;
            }
        }

        return false;
    }
};
