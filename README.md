# Aegis-LockFree

A study of lock-free stack design in **C++20**, exploring correctness, memory safety, and scalability under multithreaded contention.

This project implements a Treiber lock-free stack using atomic operations and evaluates it against a traditional mutex-based implementation. It also demonstrates the real engineering challenges of safe memory reclamation in non-blocking data structures.

---

# Repository Structure

This repository contains two development stages:

### 🔹 `main` branch — Baseline Implementation

* Treiber lock-free stack using CAS
* Acquire/Release memory ordering
* Mutex-based comparison
* Benchmark harness
* Immediate deletion disabled to avoid use-after-free crashes

⚠ This version is structurally correct but does **not** implement safe memory reclamation.
Nodes are not safely reclaimed under concurrent access.

---

### 🔹 `v2` branch — Safe Memory Reclamation

* Hazard-pointer-based memory reclamation
* Deferred deletion using per-thread retire lists
* CAS failure instrumentation
* Cooperative backoff (`std::this_thread::yield`)
* Contention scaling benchmark

This version is memory-safe and demonstrates correct lock-free reclamation without global locking.

---

# Why Lock-Free?

Mutex-based synchronization:

* Blocks threads
* Relies on OS scheduling
* Causes context switches
* Can degrade heavily under contention

Lock-free data structures:

* Do not block threads
* Guarantee system-wide progress
* Avoid kernel involvement
* Often scale better under high contention

However, they require careful reasoning about:

* Memory visibility
* Instruction reordering
* ABA hazards
* Safe memory reclamation

---

# Implementation Details

## Treiber Lock-Free Stack

The core stack uses:

```cpp id="g4k1sl"
std::atomic<Node*> head;
```

Push and pop operations use CAS retry loops.

Memory ordering strategy:

* `memory_order_release` on push
* `memory_order_acquire` on pop

This ensures:

* Node initialization is visible before publication
* Threads observe fully constructed nodes
* No unnecessary full memory fences

---

## The ABA Problem

The ABA problem occurs when:

1. Thread A reads pointer X.
2. Thread B removes X.
3. Thread B re-inserts X.
4. Thread A performs CAS and succeeds incorrectly.

Even though the pointer value appears unchanged, the data structure has been modified.

---

## Tagged Pointer Mitigation

To mitigate ABA:

* The head pointer stores `{pointer, version}`
* The version increments on every modification
* CAS compares both pointer and version

This prevents incorrect CAS success when pointer values repeat.

---

# Memory Reclamation: The Core Challenge

## Baseline Behavior (`main`)

In the initial implementation:

* Immediate deletion caused use-after-free errors.
* Disabling deletion prevented crashes.
* But introduced memory leaks.

This illustrates a fundamental truth:

> Lock-free algorithms are structurally correct, but memory reclamation is a separate and complex problem.

---

## Hazard Pointer Reclamation (`v2`)

The improved version introduces hazard pointers:

* Threads publish nodes they are currently accessing.
* Nodes are not deleted while referenced.
* Nodes are added to per-thread retire lists.
* Periodic scanning safely reclaims memory.

This guarantees correctness without global locks.

---

# Benchmark Design

The benchmark:

* Spawns multiple threads
* Each thread performs repeated `push` + `pop`
* Measures execution time
* Tracks CAS failures
* Compares lock-free vs mutex implementations

Metrics observed:

* Throughput
* Scalability under contention
* CAS retry growth

---

# Benchmark Results

*(Lenovo S340, GCC 13, C++20)*

| Threads | Mutex (ms) | Lock-Free (ms) | CAS Failures |
| ------- | ---------- | -------------- | ------------ |
| 1       | 4          | 41             | 0            |
| 2       | 27         | 26             | 1,274        |
| 4       | 32         | 37             | 13,967       |
| 8       | 106        | 56             | 62,582       |

---

## Observations

* With 1 thread, mutex is faster due to lower overhead.
* As contention increases, mutex performance degrades.
* Lock-free scales better under high contention.
* CAS failures increase with thread count, reflecting contention.
* Hazard pointer reclamation introduces overhead but ensures safety.

These results align with theoretical expectations for non-blocking algorithms.

---

# Lock-Free vs Mutex

| Property           | Lock-Free   | Mutex               |
| ------------------ | ----------- | ------------------- |
| Blocking           | No          | Yes                 |
| Deadlock           | Impossible  | Possible            |
| Kernel Involvement | None        | Yes                 |
| Contention Cost    | CAS retries | Context switches    |
| Progress Guarantee | System-wide | Scheduler-dependent |

---

# Build Instructions

```bash
mkdir build
cd build
cmake ..
make
./benchmark
```

### Requirements

* C++20 compatible compiler
* CMake ≥ 3.16
