#pragma once
#include <atomic>
#include <thread>
#include <vector>

constexpr unsigned MAX_HAZARD_POINTERS = 100;

struct HazardPointer {
    std::atomic<std::thread::id> id;
    std::atomic<void*> pointer;
};

inline HazardPointer hazard_pointers[MAX_HAZARD_POINTERS];

class HazardPointerOwner {
private:
    HazardPointer* hp;

public:
    HazardPointerOwner(const HazardPointerOwner&) = delete;
    HazardPointerOwner& operator=(const HazardPointerOwner&) = delete;

    HazardPointerOwner() : hp(nullptr) {
        for (unsigned i = 0; i < MAX_HAZARD_POINTERS; ++i) {
            std::thread::id old_id;
            if (hazard_pointers[i].id.compare_exchange_strong(
                    old_id,
                    std::this_thread::get_id())) {
                hp = &hazard_pointers[i];
                break;
            }
        }

        if (!hp)
            throw std::runtime_error("No hazard pointers available");
    }

    std::atomic<void*>& get_pointer() {
        return hp->pointer;
    }

    ~HazardPointerOwner() {
        hp->pointer.store(nullptr);
        hp->id.store(std::thread::id());
    }
};

inline bool outstanding_hazard_pointers_for(void* ptr) {
    for (unsigned i = 0; i < MAX_HAZARD_POINTERS; ++i) {
        if (hazard_pointers[i].pointer.load() == ptr)
            return true;
    }
    return false;
}
