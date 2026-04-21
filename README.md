*This project has been created as part of the 42 curriculum by adraji.*

# Codexion

## Description
Codexion is a concurrency simulation built in C using POSIX threads. It models coders sharing limited computing resources (dongles) to compile, debug, and refactor code. The simulation coordinates resource access via a priority heap scheduler while systematically preventing concurrency failures.

## Instructions
**Compilation:**
Run the `make` in the folder `coders` to create the executable file.

**Execution:**
`./codexion num_coders time_to_burnout time_to_compile time_to_debug time_to_refactor target_compiles cooldown scheduler`
*(Note: scheduler options are `fifo` or `edf`)*

## Thread synchronization mechanisms
- **Mutexes (`pthread_mutex_t`)**: 
  - `state_mtx`: Achieves thread-safe communication between `coders` and the `monitor` by protecting shared state reads/writes (`last_compile_start`, `compile_count`, `sim_stop`).
  - `print_mtx`: Locks STDOUT to prevent race conditions during logging.
  - `dongles[i].mtx`: Protects the availability state and priority heap of each individual hardware resource.
- **Condition Variables (`pthread_cond_t`)**: 
  - `pthread_cond_timedwait` and `pthread_cond_wait` are used to block coder threads efficiently while waiting for resources, instead of spinning the CPU. `pthread_cond_broadcast` is used in `dongle_release` to wake queued threads.

## Blocking cases handled
- **Deadlock Prevention (Coffman’s conditions)**: Circular wait is structurally broken in the initialization phase. Coders are asymmetrically assigned their `d1` and `d2` pointers to always lock the lower-ID dongle first.
- **Starvation Prevention**: Mitigated using a Min-Heap priority queue for resource requests, strictly enforced by either a First In First Out (FIFO) or Earliest Deadline First (EDF) scheduler.
- **Cooldown Handling**: Enforced using exact `available_at` timestamps. Threads use `pthread_cond_timedwait` to respect the required hardware delay before re-acquisition.
- **Precise Burnout Detection**: A discrete `monitor` thread constantly checks `last_compile_start` limits within a low-latency loop (`usleep(1000)`), ensuring burnout detection occurs safely and within the 10ms tolerance.
- **Log Serialization**: All state output is strictly routed through `print_state`, which utilizes `print_mtx` to ensure logs are never interleaved out of order.

## Resources
* **[GeeksforGeeks](https://www.geeksforgeeks.org/operating-systems/what-are-threads-in-computer-processor-or-cpu/)**: Used to review fundamental thread lifecycle management.
* **[Wikipedia](https://en.wikipedia.org/wiki/Hyper-threading)**: Referenced for conceptual understanding of hardware vs. software threads.
* **[Intel i9-14900K Specs](https://www.intel.fr/content/www/fr/fr/products/sku/236773/intel-core-i9-processor-14900k-36m-cache-up-to-6-00-ghz/specifications.html)**: Used to analyze real-world CPU thread/core ratios.

### AI Usage Disclosure
AI was used in this project for the following tasks:
* **Summarization**: Extracting key threading concepts and technical specifications from the documentation listed above.
* **Structural Mapping**: Assisting in the logical organization and Markdown formatting of this README file.
* **Optimization**: Suggesting efficient ways to handle the 10ms burnout detection tolerance in the monitor routine.
*Note: All source code logic was manually implemented and verified.*
