*This project has been created as part of the 42 curriculum by `adraji`.*

---

# Codexion — A Concurrent Scheduling Simulation

## Description

**Codexion** is a concurrency simulation inspired by the classic *Dining Philosophers* problem, recast in a software-engineering context. Instead of philosophers and forks, the simulation features **coders** and **USB dongles** — hardware license keys required to run a compiler.

Each coder is an independent POSIX thread. To compile, a coder must simultaneously hold **two dongles** (shared resources). Once acquired, the coder compiles for a fixed duration, then releases the dongles before entering sequential **debug** and **refactor** phases. The simulation ends either when every coder reaches the required number of compilations, or when any coder **burns out** — i.e., goes too long without starting a compilation.

### Goals

- Model and resolve real-world concurrency hazards (deadlock, starvation, race conditions) in a constrained multi-threaded environment.
- Implement and compare two scheduling disciplines: **FIFO** and **EDF (Earliest Deadline First)**.
- Produce a deterministic, race-free event log with millisecond-precision timestamps.

### Architecture at a Glance

| Component | Role |
|-----------|------|
| `coder_routine` | Thread entry-point; drives the compile → debug → refactor loop |
| `monitor_routine` | Dedicated watchdog thread; detects burnout and checks completion |
| `dongles_take` / `dongles_release` | Dongle acquisition/release with priority-queue arbitration |
| `heap` (per dongle) | Wait-list sorted by FIFO arrival or EDF deadline |
| `parse` / `init` | Argument validation and full simulation setup/teardown |

---

## Instructions

### Requirements

- A POSIX-compliant OS (Linux or macOS)
- `clang`
- `make` (optional but recommended)

### Compilation

```bash
# With make (if a Makefile is present)
make

# Or manually
cc -Wall -Wextra -Werror -pthread codexion.c init.c parse.c coder.c dongle.c dongle_utils.c heap.c monitor.c utils.c -o codexion
```

### Execution

```
./codexion <n_coders> <t_burnout> <t_compile> <t_debug> <t_refactor> <target_compiles> <d_cooldown> <scheduler>
```

| Argument | Type | Description |
|----------|------|-------------|
| `n_coders` | int > 0 | Number of concurrent coders (threads) |
| `t_burnout` | int ≥ 0 | Max milliseconds a coder can go without compiling |
| `t_compile` | int ≥ 0 | Duration of one compilation cycle (ms) |
| `t_debug` | int ≥ 0 | Duration of the debug phase (ms) |
| `t_refactor` | int ≥ 0 | Duration of the refactor phase (ms) |
| `target_compiles` | int > 0 | Compilations each coder must complete |
| `d_cooldown` | int ≥ 0 | Dongle cooldown after release before reuse (ms) |
| `scheduler` | string | `fifo` or `edf` |

### Example runs

```bash
# 5 coders, 800 ms burnout limit, 200 ms compile, 100 ms debug,
# 100 ms refactor, 5 compiles each, 50 ms cooldown, FIFO scheduling
./codexion 5 800 200 100 100 5 50 fifo

# Same setup but with Earliest-Deadline-First scheduling
./codexion 5 800 200 100 100 5 50 edf
```

### Output format

Each event is printed as:

```
<elapsed_ms> <coder_id> <event>
```

Events: `has taken a dongle`, `is compiling`, `is debugging`, `is refactoring`, `burned out`.

---

## Blocking Cases Handled

### 1. Deadlock Prevention — Coffman's Conditions

The classic deadlock scenario arises when every coder simultaneously holds one dongle and waits for a second one, forming a circular wait. Codexion breaks this with a **consistent global lock ordering**:

- Even-numbered coders acquire `dongles[i]` then `dongles[(i+1) % n]`.
- Odd-numbered coders acquire them in the **reverse** order.

This asymmetry eliminates the circular-wait condition (Coffman condition #4), making deadlock structurally impossible regardless of the number of coders.

### 2. Starvation Prevention — Priority Queues

Each dongle maintains a **bounded heap** (capacity 2, since each dongle is shared by at most 2 coders). When a coder wants a dongle, it pushes a request onto the heap before entering its wait loop. The dongle is only granted when the requesting coder is **at the front of both heaps** simultaneously.

- **FIFO mode**: requests are served in arrival order — no coder can be indefinitely skipped.
- **EDF mode**: the request with the earliest burnout deadline is served first — the coder most at risk of burning out gets priority.

### 3. Cooldown Handling

After a coder releases its dongles, each dongle records an `available_at` timestamp set to `now + d_cooldown`. Any coder checking `both_available()` also verifies `now >= dongle.available_at`, ensuring the cooldown window is fully respected before reuse. The wait loop in `dongles_take` uses `wait_dongels()` to sleep until the later of the two dongles' availability times, avoiding a tight busy-wait.

### 4. Precise Burnout Detection

The monitor thread runs a dedicated loop (polling every 1 ms) that compares `get_time_ms()` against each coder's `last_compile_start + t_burnout`. The `last_compile_start` field is updated under the coder's own mutex at the very beginning of `coder_compile`, ensuring the timestamp reflects the true start of compilation. This tight coupling minimises false positives and false negatives in burnout detection.

### 5. Log Serialization

All console output goes through `print_state`, which acquires `print_mtx` before reading the clock and `state_mtx` before checking the stop flag. This double-lock pattern ensures that the printed timestamp is coherent with the simulation state and that lines from concurrent threads are never interleaved.

---

## Thread Synchronization Mechanisms

### `pthread_mutex_t` — Mutual Exclusion

| Mutex | Protected resource |
|-------|-------------------|
| `dongle.mtx` (one per dongle) | `dongle.is_held`, `dongle.available_at`, `dongle.heap` |
| `coder.mtx` (one per coder) | `coder.last_compile_start`, `coder.compile_count` |
| `sim.print_mtx` | `stdout` — serialises all log output |
| `sim.state_mtx` | `sim.stop`, `sim.start` — shared control flags |

**Dongle locking order** is always enforced through `lock_dongles` / `unlock_dongles`, which acquire `first->mtx` before `second->mtx`. Because `first` and `second` are assigned opposite orderings for odd vs even coders, no cycle can form.

**Race condition example — compile count:**
```c
// coder_compile — protected write
pthread_mutex_lock(&c->mtx);
c->compile_count++;
pthread_mutex_unlock(&c->mtx);

// monitor — protected read
pthread_mutex_lock(&c->mtx);
finished = (c->compile_count >= sim->target_compiles);
pthread_mutex_unlock(&c->mtx);
```
Without this guard, the monitor could read a partial increment and incorrectly conclude the simulation is not finished.

### `pthread_cond_t` — Condition Variables

| Condition variable | Purpose |
|--------------------|---------|
| `dongle.cv` | Wakes waiters after a dongle is released (broadcast in `dongles_release`) |
| `sim.start_cv` | Holds all coder threads at the barrier until the monitor sets `sim.start = TRUE` and broadcasts |

The **start barrier** pattern ensures every coder and the monitor are fully initialised before any work begins:
```c
// Coder — waits at barrier
pthread_mutex_lock(&sim->state_mtx);
while (!sim->start)
    pthread_cond_wait(&sim->start_cv, &sim->state_mtx);
pthread_mutex_unlock(&sim->state_mtx);

// Monitor — releases all coders atomically
pthread_mutex_lock(&sim->state_mtx);
sim->start_time = get_time_ms();   // single reference clock
sim->start = TRUE;
pthread_cond_broadcast(&sim->start_cv);
pthread_mutex_unlock(&sim->state_mtx);
```
This guarantees that `sim->start_time` is set before any coder reads it, and that all coders start from the same logical origin.

### Custom Wait Logic in `dongles_take`

Rather than blocking unconditionally on a condition variable, `dongles_take` uses an **active wait loop** that calls `check_stop` on every iteration. This allows threads to exit promptly when the simulation is terminated mid-wait:

```c
while (!check_stop(c->sim))
{
    if (both_available(c))
    {
        // acquire both dongles atomically under their held locks
        c->first->is_held = TRUE;
        c->second->is_held = TRUE;
        ...
        return (TRUE);
    }
    wait_dongels(c);   // release locks, sleep, re-acquire
}
return (cleanup_and_unlock(c), FALSE);  // graceful exit on stop
```

`wait_dongels` temporarily releases both dongle mutexes, sleeps for the calculated cooldown remainder, then re-acquires them — preventing the thread from holding locks while sleeping and allowing other coders to make progress.

---

## Resources

### Classic References

- **[GeeksforGeeks](https://www.geeksforgeeks.org/operating-systems/what-are-threads-in-computer-processor-or-cpu/)**: Used to review fundamental thread lifecycle management.
- **[Wikipedia](https://en.wikipedia.org/wiki/Hyper-threading)**: Referenced for conceptual understanding of hardware vs. software threads.
- **[Intel i9-14900K Specs](https://www.intel.fr/content/www/fr/fr/products/sku/236773/intel-core-i9-processor-14900k-36m-cache-up-to-6-00-ghz/specifications.html)**: Used to analyze real-world CPU thread/core ratios.

### AI Usage

Large-language-model assistance (ChatGPT / Claude) was used during this project for the following specific tasks:

- **Design review**: discussing trade-offs between condition-variable blocking and active-wait loops for the dongle acquisition logic, and validating the lock-ordering strategy against Coffman's four conditions.
- **Debugging**: analysing potential TOCTOU (time-of-check/time-of-use) races in the `both_available` check and the interaction between `available_at` updates and the wait loop.
- **Documentation**: drafting and structuring this README, including formatting of the synchronisation mechanism tables.

The core algorithmic choices — the priority-queue per dongle, the asymmetric lock ordering, the monitor-based burnout detection, and the start barrier — were designed and implemented by the project author.
