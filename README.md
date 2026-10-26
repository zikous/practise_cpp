# C++ Practice Problems

A set of C++ practice problems from **[getcracked.io](https://getcracked.io)** (Coding Jesus), covering concurrency, templates, metaprogramming, and lock-free data structures.

## Progress

| # | Problem | Difficulty | Solved |
|---|---------|------------|--------|
| 1 | [Avoiding False Sharing](#1-avoiding-false-sharing) | Easy | [X] |
| 2 | [Composable Candlestick Attributes](#2-composable-candlestick-attributes) | Easy | [X] |
| 3 | [Call Stack Sampling to Trace Events](#3-call-stack-sampling-to-trace-events) | Easy | [X] |
| 4 | [Redundant Feed Arbitration](#4-redundant-feed-arbitration) | Medium | [X] |
| 5 | [Selecting Instruments for a 30-Day Horizon](#5-selecting-instruments-for-a-30-day-horizon) | Medium | [ ] |
| 6 | [Parallel Accumulate](#6-parallel-accumulate) | Medium | [X] |
| 7 | [Optional with Small-Buffer Optimization](#7-optional-with-small-buffer-optimization) | Hard | [X] |
| 8 | [Hybrid Spin/Block Mutex](#8-hybrid-spinblock-mutex) | Hard | [ ] |
| 9 | [Lock-Free Single-Producer Single-Consumer Queue](#9-lock-free-single-producer-single-consumer-queue) | Hard | [ ] |
| 10 | [Writer-Preferring Reader/Writer Lock](#10-writer-preferring-readerwriter-lock) | Hard | [ ] |

---

## 1. Avoiding False Sharing
`Difficulty: Easy`

### Context
Two threads update two different counters that happen to sit next to each other in memory. If both counters share a cache line, each write by one thread invalidates the line in the other core's cache, and performance collapses even though the threads never touch the same variable.

### Task
Define a pair structure `Data` holding two atomic counters `x` and `y`, so that concurrent updates of `x` by one thread and of `y` by another never cause false sharing.

### Requirements
- `Data` is templated on two types, one for each counter.
- Both types must be constrained at compile time to be integral types; any other type must be rejected with a clear error.
- Each counter is an atomic of its type.
- The layout must guarantee that `x` and `y` are on different cache lines, using the portable constant provided by the standard library rather than a hard-coded size.

---

## 2. Composable Candlestick Attributes
`Difficulty: Easy`

### Context
A charting library draws candlesticks. A candle is described by four prices: open, high, low and close. Different applications want to attach different extra data to each candle (a text label, a color opacity, a volume, ...), and the library authors do not want a new candle type for every combination.

### Task
Make the candle type composable: it must accept any number of independent attribute types and expose all of their members directly on the candle (a candle with a label attribute gives access to `candle.Text`, with no intermediate wrapper object). Then define the concrete candle type used by the application, combining a label attribute (a string `Text`) and a color attribute (an integer `Alpha`).

A chart stores a sequence of such candles and must be able to return the label and the alpha of its first candle, or the empty string and −1 respectively if the chart is empty.

### Constraints
- Zero, one or many attributes must be supported.
- Attributes know nothing about each other or about the candle.
- The composition must be resolved at compile time and add no runtime indirection.

---

## 3. Call Stack Sampling to Trace Events
`Difficulty: Easy`

### Context
A sampling profiler periodically records the call stack of a running program. Each sample consists of a timestamp and the list of active functions, from the outermost (e.g. `main`) to the innermost. To display a flame graph or a trace timeline, these snapshots must be converted into Begin and End events.

### Task
Given the samples in chronological order, produce the list of events. Comparing each sample with the previous one:
- the functions that were on the previous stack but are no longer there produce End events, innermost first;
- the functions newly present produce Begin events, outermost first;
- all these events carry the timestamp of the current sample;
- the first sample emits a Begin for each of its functions.

Only the common prefix of two consecutive stacks is considered unchanged: if a function appears at the same depth but under a different caller, it is a different call. Functions still active after the last sample are not closed. An empty input gives an empty output.

### Example
| Time | Stack |
|------|-------|
| 1.0  | [main] |
| 2.0  | [main, foo] |
| 3.0  | [main, foo, bar] |
| 4.0  | [main] |

Expected events: `Begin(1.0, main)`, `Begin(2.0, foo)`, `Begin(3.0, bar)`, `End(4.0, bar)`, `End(4.0, foo)`.

---

## 4. Redundant Feed Arbitration
`Difficulty: Medium`

### Context
Trading systems often subscribe to the same market data through two redundant feeds, A and B, so that a packet lost or delayed on one feed can still be obtained from the other. Each packet carries its source feed, a receive timestamp and a sequence number. The same sequence number may be delivered on both feeds, at different times.

### Task
Given the two feeds, each exposed as a lazily produced sequence of packets (a `std::generator`) that can only be traversed once, build the arbitrated stream: for every distinct sequence number, keep exactly one packet.

### Rules
- The kept packet is the copy with the smallest receive time.
- If both copies have the same receive time, the packet from feed A wins.
- The result is sorted by increasing sequence number.
- A sequence number seen on only one feed is kept as is.
- The two feeds are not assumed to be sorted, nor to have the same length.

### Example
| Seq. | Feed A (time) | Feed B (time) | Kept |
|------|----------------|----------------|------|
| 1    | 10             | 8              | B    |
| 2    | 20             | 20             | A (tie) |
| 3    | 30             | —              | A    |
| 4    | —              | 5              | B    |

---

## 5. Selecting Instruments for a 30-Day Horizon
`Difficulty: Medium`

### Context
A pricing model needs a value at a constant maturity of 30 days, which is obtained by interpolating between two listed instruments: one expiring before 30 days (near) and one expiring after (far). Each instrument has a payment frequency (monthly or annual), a number of days to expiration (DTE, a floating-point number) and a list of settlement prices.

### Task
From an unordered list of instruments, return the one or two instruments to use, according to the following rules.

1. Discard annual instruments and instruments that are expired (DTE ≤ 0). Among the remaining ones, only monthly instruments with at least two settlement prices are usable.
2. **Exact match.** If a usable instrument has DTE = 30 (compared with a floating-point tolerance), return only that instrument.
3. **Near instrument.** Take the usable instrument with the largest DTE in [10, 30). If there is none, fall back to the usable instrument with the smallest DTE strictly above 30. If there is none either, return an empty list.
4. **Far instrument.** Take the usable instrument with the smallest DTE that is strictly above 30 and strictly greater than the DTE of the near instrument. If there is none, return an empty list.
5. Otherwise return the pair (near, far), in this order.

### Examples
| Instruments (frequency, DTE) | Result |
|-------------------------------|--------|
| M 12, M 25, M 45, M 60, A 40 | M 25 and M 45 |
| M 30 (two prices), M 12, M 45 | M 30 only |
| M 1.5, M 5, M 9.9, A 20 | empty (nothing usable near) |

*(All monthly instruments above have at least two settlement prices.)*

---

## 6. Parallel Accumulate
`Difficulty: Medium`

### Context
Summing a very large range of numbers is trivially parallelizable, as long as the work is split sensibly: too many threads for a small range costs more than it saves.

### Task
Implement a function that returns the sum of all elements of a range given by two iterators, added to an initial value (default-constructed by default), using several threads when the range is large.

### Rules
- A fixed maximum chunk size (a constant, of the order of ten million elements) is given. If the range does not exceed it, compute the sum directly on the calling thread.
- Otherwise, split the range into `⌈n/chunk⌉` consecutive chunks, process each in its own thread (the last one may be shorter, and must never read past the end) and combine the partial sums once all threads have finished.
- The initial value must be counted exactly once.
- An empty range returns the initial value.
- If the end iterator comes before the start iterator, throw a `std::logic_error`.
- The function must work for any random-access iterator type and any value type supporting addition.

---

## 7. Optional with Small-Buffer Optimization
`Difficulty: Hard`

### Context
`std::optional` may or may not contain a value. This exercise asks for a simplified version whose storage strategy depends on the size of the contained type.

### Task
Implement an optional-like class template that chooses its storage at compile time.
- A type is **small** if both its size and its alignment are at most those of a pointer. A small value is stored *inline*, inside the optional object itself, with no heap allocation: the object is constructed in place in an aligned buffer, and a flag records whether it exists.
- Any other type is **large**. A large value is stored *on the heap*, the optional holding a pointer that is null when empty, and the memory is released when the optional is destroyed.

### Interface
Both variants provide: default construction (empty), construction from a value, a test for the presence of a value (explicit method and conversion to bool), and read access to the value (dereference operator and a `value` method) returning a constant reference. The optional can be neither copied nor moved.

### Notes
The classification of types as small or large must be a reusable compile-time trait, and the two variants must be selected through partial specialization (or constraints) rather than runtime tests.

---

## 8. Hybrid Spin/Block Mutex
`Difficulty: Hard`

### Context
A pure spinlock wastes CPU when the lock is held for long, while a pure blocking mutex pays a system call even when the lock is released a few nanoseconds later. A hybrid mutex spins briefly first, and only then goes to sleep.

### Task
Implement a mutex class with `lock` and `unlock` operations, using a single atomic variable as its only synchronization primitive.

### Rules
- The lock operation first tries to acquire the lock, then spins for a bounded number of attempts (a constant number of cycles defined by the user). If the lock is still not obtained, the thread must sleep on the atomic until it is notified, instead of continuing to spin.
- The unlock operation is only called by the thread holding the lock; it releases the lock and wakes up at most one waiting thread.
- Mutual exclusion must be guaranteed: lock acquisition has acquire semantics and release has release semantics (use no stronger ordering than needed).
- A thread woken up must re-check the lock rather than assume it owns it.

---

## 9. Lock-Free Single-Producer Single-Consumer Queue
`Difficulty: Hard`

### Context
A producer thread hands items to a consumer thread through a bounded queue. Because exactly one thread pushes and exactly one thread pops, the queue can be implemented without any lock, using only atomic indices.

### Task
Implement a generic, fixed-capacity, lock-free SPSC queue as a ring buffer.

### Rules
- The capacity is given at construction. It must be a power of two and at least 2; otherwise a `std::logic_error` is thrown. Index wrap-around must be done with a bit mask rather than a modulo.
- The push operation returns `false` without modifying the queue when it is full; the pop operation returns `false` when it is empty.
- As in the classical ring buffer, one slot is kept free to distinguish a full queue from an empty one: a queue of capacity N holds at most N − 1 items.
- Query operations are provided to know whether the queue is full, empty, and how many items it holds.
- The queue can be neither copied nor moved.
- Memory orderings must be as weak as correctness allows (acquire/release between the two threads, relaxed for an index read by its own owner).
- The producer index and the consumer index must live on different cache lines to avoid false sharing.

---

## 10. Writer-Preferring Reader/Writer Lock
`Difficulty: Hard`

### Context
A data structure is read very often and modified rarely. A reader/writer lock lets any number of readers work simultaneously, while a writer needs the data for itself.

### Task
Implement a lock class shared by all reader and writer threads, with four operations: lock and unlock for readers, lock and unlock for writers.

### Rules
- Any number of readers may hold the lock at the same time.
- A writer holds the lock exclusively: no reader and no other writer at the same time.
- No writer starvation: as soon as a writer is waiting, readers arriving afterwards must wait until that writer has been served, even if other readers are currently inside.
- Waiting must be passive (no busy loop).
- The reader-lock operation returns the number of readers that already held the lock at the moment of the call, before the caller enters.
- The reader-unlock operation returns the number of readers holding the lock at the moment of the call, including the caller, before it leaves.
