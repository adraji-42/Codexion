*This project has been created as part of the 42 curriculum by adraji.*

---

# Codexion

> Master the race for resources before the deadline masters you.

---

## Description

Codexion is a concurrency simulation inspired by the classic Dining Philosophers problem. A group of coders sit in a circle around a shared Quantum Compiler. Each coder cycles through three phases — **compile → debug → refactor** — and must hold two adjacent USB dongles simultaneously to compile. Since there are exactly as many dongles as coders, access is always contested.

The goal is to keep every coder alive (i.e., prevent burnout) while fairly scheduling dongle access. The simulation ends either when all coders reach the required compile count, or when one burns out.

Key features:
- Each coder runs as a POSIX thread
- Dongles are protected by mutexes + condition variables
- Two scheduling policies: **FIFO** and **EDF** (Earliest Deadline First)
- A dongle cooldown enforces a mandatory rest period after each release
- A dedicated monitor thread detects burnout within 10 ms

---

## Instructions

### Compilation

```bash
make
```

Compiles with `-Wall -Wextra -Werror -pthread`. The binary is named `codexion`.

### Cleaning

```bash
make clean    # remove object files
make fclean   # remove objects + binary
make re       # full rebuild
```

### Execution

```
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
            time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

All arguments are mandatory. `scheduler` must be exactly `fifo` or `edf`.

**Example — 4 coders, 800 ms burnout limit, each compile 200 ms:**
```bash
./codexion 4 800 200 200 200 5 0 fifo
```

**Example — EDF scheduling with cooldown:**
```bash
./codexion 3 600 100 100 100 3 50 edf
```

### Expected log format

```
0 1 has taken a dongle
1 1 has taken a dongle
1 1 is compiling
201 1 is debugging
401 1 is refactoring
...
1505 4 burned out
```

---

## Blocking Cases Handled

### Deadlock Prevention
Deadlock in this problem arises when every coder holds one dongle and waits for the other — a circular wait. This is broken by reversing the acquisition order for even-numbered coders: odd coders take right-then-left, even coders take left-then-right. This asymmetry prevents a full circular dependency from forming.

### Coffman's Conditions Addressed
| Condition | How it's broken |
|---|---|
| Mutual exclusion | Necessary — kept, dongles are exclusive |
| Hold and wait | Avoided via ordered acquisition |
| No preemption | Kept — but starvation is prevented by the scheduler |
| Circular wait | Broken by asymmetric dongle order |

### Starvation Prevention
Under **FIFO**, each dongle maintains an arrival-ordered queue — no coder waits indefinitely as long as it keeps cycling. Under **EDF**, the coder with the nearest burnout deadline is served first, actively preventing burnout in feasible configurations.

### Cooldown Handling
After a dongle is released, it records `available_at = now + d_cooldown`. Waiters use `pthread_cond_timedwait` to sleep exactly until the dongle becomes available, avoiding busy-waiting while respecting the cooldown precisely.

### Burnout Detection
A dedicated monitor thread polls all coders every 1 ms, comparing `get_time_ms()` against each coder's `last_compile_start + t_burnout`. When a burnout is detected, it prints the log and sets `sim_stop`, guaranteeing the message appears within 10 ms of the actual deadline.

### Log Serialization
All `printf` calls are wrapped in a `print_mtx` lock. The check for `sim_stop` happens inside the lock, so no stale messages are printed after the simulation ends, and no two log lines interleave.

---

## Thread Synchronization Mechanisms

### Primitives Used

| Primitive | Purpose |
|---|---|
| `pthread_mutex_t print_mtx` | Serializes all stdout output |
| `pthread_mutex_t state_mtx` | Protects `sim_stop`, `compile_count`, `last_compile_start` |
| `pthread_mutex_t dongle.mtx` | Guards each dongle's state and its priority queue |
| `pthread_cond_t dongle.cv` | Blocks waiters until the dongle is free and available |

### How Access Is Coordinated

**Taking a dongle (`dongle_take`):**
1. Coder computes its deadline under `state_mtx` (snapshot of `last_compile_start`).
2. It locks `dongle.mtx`, pushes a `t_req` into the dongle's heap, then calls `wait_dongle`.
3. Inside `wait_dongle`, the coder loops: if it is at the front of the heap and the cooldown has expired, it breaks out. Otherwise it calls `pthread_cond_timedwait` (cooldown remaining) or `pthread_cond_wait` (waiting behind another coder).
4. On wake, it re-evaluates the condition — spurious wakeups are safe.

**Releasing a dongle (`dongle_release`):**
1. Locks `dongle.mtx`, sets `is_held = FALSE`, records `available_at`.
2. Calls `pthread_cond_broadcast` to wake all waiters so the next eligible coder can proceed.

**Monitor ↔ coder communication:**
- The monitor reads `last_compile_start` under `state_mtx` (written only by the coder thread when it finishes compiling).
- When `sim_stop` is set (by monitor or by `all_compiled`), coder threads check `check_stop()` at the top of their loop and after each `dongle_take` attempt, breaking out cleanly.
- `cleanup_sim` broadcasts on every dongle's condition variable so no coder thread remains blocked in `pthread_cond_wait` after shutdown.

### Race Condition Prevention Example

```
Coder thread                          Monitor thread
─────────────────────────────────     ──────────────────────────────
lock(state_mtx)
  compile_count++                     lock(state_mtx)
unlock(state_mtx)                       read compile_count  ← safe
                                      unlock(state_mtx)
```

Because `compile_count` and `sim_stop` are always read/written under `state_mtx`, there are no data races between coder threads and the monitor.

---

## Resources

### Concurrency & Scheduling
* **[GeeksforGeeks](https://www.geeksforgeeks.org/operating-systems/what-are-threads-in-computer-processor-or-cpu/)**: Used to review fundamental thread lifecycle management.
* **[Wikipedia](https://en.wikipedia.org/wiki/Hyper-threading)**: Referenced for conceptual understanding of hardware vs. software threads.
* **[Intel i9-14900K Specs](https://www.intel.fr/content/www/fr/fr/products/sku/236773/intel-core-i9-processor-14900k-36m-cache-up-to-6-00-ghz/specifications.html)**: Used to analyze real-world CPU thread/core ratios.

### AI Usage
Claude (Anthropic) was used to generate this README from the project subject and source files. All code was written independently; AI was not used for any implementation. The generated README was reviewed and verified for accuracy against the actual code before submission.